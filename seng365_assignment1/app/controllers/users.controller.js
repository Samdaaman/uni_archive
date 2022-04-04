const users = require('../models/users.model');
const utils = require('../utils/utils');
const mime = require('mime');

function getToken(req) {
    return utils.parseText(req.get('X-Authorization'));
}

async function checkAuthedId(userId, req, res) {
    // Returns userId if auth is successful
    // UserId can be set to -1 for any user (only needs correct token)
    const token = getToken(req);
    const authedId = await users.getUserIdFromToken(token);
    if (authedId == null || userId == null) {
        res.statusMessage = 'Unauthorized'
        res.status(401).send();
    }
    else if (authedId != userId && userId != -1) {
        res.statusMessage = 'Forbidden';
        res.status(403).send();
    }
    else {
        return authedId;
    }
    return false;
}

exports.login = async function(req, res) {
    try {
        [email, password] = utils.parseTextArray([req.body.email, req.body.password]);

        if (email != '' && password != '') {
            [userId, token] = await users.tryLogin(email, password);
            if (userId != null && token != null)
            {
                res.statusMessage = 'OK';
                res.status(200)
                    .send({
                        'userId': userId, 
                        'token': token
                    });
                return;
            }
        }
        // Below is only executed if there is an error in the request (ie the above return is never reached)
        res.statusMessage = 'Bad Request';
        res.status(400).send();
    }
    catch (err) {
        console.log(`ERROR logging in as a user ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.logout = async function(req, res) {
    try {
        const userId = await checkAuthedId(-1, req, res)
        if (userId)
        {
            await users.setToken(userId, null)
            res.statusMessage = 'OK';
            res.status(200).send();
        }        
    }
    catch (err) {
        console.log(`ERROR logging out as a user ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.register = async function(req, res) {
    try {
        [name, email, password, city, country] = utils.parseTextArray([req.body.name, req.body.email, req.body.password, req.body.city, req.body.country]);

        if (name != '' && email.includes('@') && password != '' && await users.getUserIdFromEmail(email) == null) {
            result = await users.create(name, email, password, city, country);
            res.statusMessage = 'Created';
            res.status(201).send({"userId" : result});
        }
        else {
            res.statusMessage = 'Bad Request';
            res.status(400).send();
        }
    }
    catch (err) {
        console.log(`ERROR registering user ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.getOneUser = async function(req, res) {
    try {
        const userId = req.params.userId;
        const userData = await users.getUserData(userId);
        if (userData == null)
        {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
        else {
            const token = getToken(req)
            if (await users.getUserIdFromToken(token) != req.params.userId)
            {
                delete userData.email;
            }
            const userDataTrimmed = {
                name: userData.name,
                city: userData.city,
                country: userData.country,
                email: userData.email
            }
            res.statusMessage = 'OK';
            res.status(200).send(userDataTrimmed);
        }
    }
    catch (err) {
        console.log(`ERROR getting user details ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.updateUser = async function(req, res) {
    try {
        const userId = req.params.userId;
        if (await checkAuthedId(userId, req, res)) {
            const name = req.body.name;
            const email = req.body.email;
            const currentPassword = req.body.currentPassword;
            const password = req.body.password;
            const city = req.body.city;
            const country = req.body.country;

            if (await users.tryUpdate(userId, name, email, password, currentPassword, city, country)) {
                res.statusMessage = 'OK';
                res.status(200).send();
            }
            else {
                res.statusMessage = 'Bad Request';
                res.status(400).send();
            }
        }
    }
    catch (err) {
        console.log(`ERROR updating user details ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
}

exports.getUserPhoto = async function(req, res) {
    try {
        const userId = req.params.userId;
        const [imageData, extension] = await users.getPhotoData(userId);
        if (imageData !== null) {
            const mimeType = mime.getType(extension);
            res.contentType(mimeType);
            res.statusMessage = 'OK';
            res.status(200).send(imageData);
        }
        else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
    }
    catch (err) {
        console.log(`ERROR getting user photo: ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.uploadUserPhoto = async function(req, res) {
    try {
        const mimeType = req.get('Content-Type');
        const extension = mime.getExtension(mimeType);
        const userId = req.params.userId;
        const photoData = req.body;
        if (photoData === null || userId === null || !['jpg', 'jpeg', 'png', 'gif'].includes(extension))
        {
            console.warn('Bad request' + photoData.length + userId + extension);
            res.statusMessage = 'Bad Request';
            res.status(400).send();
        }
        else {
            if (await users.getUserData(userId)) {
                if (await checkAuthedId(userId, req, res)) {
                    let photoDeleted = await users.removePhoto(userId)
                    await users.savePhotoData(photoData, userId, extension);
                    if (photoDeleted)
                    {
                        res.statusMessage = 'OK';
                        res.status(200).send();
                    }
                    else
                    {
                        res.statusMessage = 'Created';
                        res.status(201).send();
                    }
                }
            } else {
                res.statusMessage = 'Not Found';
                res.status(404).send();
            }
        } 
    }
    catch (err)
    {
        console.log(`ERROR uploading user photo: ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.deleteUserPhoto = async function(req, res) {
    try {
        const userId = req.params.userId;
        const [_, extension] = await users.getPhotoData(userId);
        if (userId === null)
        {
            res.statusMessage = 'Bad Request';
            res.status(400).send();
        }
        else if (extension == null)
        {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
        else {
            if (await checkAuthedId(userId, req, res)) {
                await users.removePhoto(userId)
                    res.statusMessage = 'OK';
                    res.status(200).send();
            }
        }
    }
    catch (err)
    {
        console.log(`ERROR deleting user photo: ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};