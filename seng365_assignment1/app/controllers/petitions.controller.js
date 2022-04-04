const petitions = require('../models/petitions.model');
const category = require('../models/category.model');
const users = require('../models/users.model');
const signatures = require('../models/signatures.model');
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

function getPetitionOverview(petition) {
    return {
        petitionId: petition.petitionId,
        title: petition.title,
        category: petition.category,
        authorName: petition.authorName,
        signatureCount: petition.signatureCount
    };
}

exports.getAllPetitions = async function(req, res) {
    try {
        let allPetitions = await petitions.getAllPetitions();
        let startIndex = parseInt(req.query.startIndex || "0");
        let count = req.query.count;
        let q = req.query.q;
        let categoryId = req.query.categoryId;
        let authorId = req.query.authorId;
        let sortBy = req.query.sortBy || "SIGNATURES_DESC";

        let badReq = false;
        if (sortBy == "SIGNATURES_DESC")
        {
            allPetitions.sort((a, b) => (a.signatureCount < b.signatureCount ? 1 : -1));
        }
        else if (sortBy == "SIGNATURES_ASC")
        {
            allPetitions.sort((a, b) => (a.signatureCount > b.signatureCount ? 1 : -1));
        }
        else if (sortBy == "ALPHABETICAL_ASC")
        {
            allPetitions.sort((a, b) => (a.title.localeCompare(b.title) > 0 ? 1 : -1));
        }
        else if (sortBy == "ALPHABETICAL_DESC")
        {
            allPetitions.sort((a, b) => (a.title.localeCompare(b.title) <= 0 ? 1 : -1));
        }
        else
        {
            badReq = true;
        }

        if (q && q != "")
        {
            allPetitions = allPetitions.filter(petition => 
            {
                return (petition.title.toLowerCase().includes(q.toLowerCase()));
            });
        }

        const allCategories = await category.getAllCategorys();
        if (isFinite(Number(categoryId)))
        {
            const categoryName = category.getCategoryName(Number(categoryId), allCategories);
            if (!categoryName) {
                badReq = true;
            }
            else {
                allPetitions = allPetitions.filter((petition, _, __) => {
                    return (petition.category == categoryName);
                });
            }
        }

        if (isFinite(Number(authorId)))
        {
            allPetitions = allPetitions.filter((petition, _, __) => {
                return (petition.authorId === Number(authorId));
            });
        }

        if (isFinite(Number(startIndex)))
        {
            allPetitions = allPetitions.slice(startIndex);
        }

        if (isFinite(Number(count)))
        {
            allPetitions = allPetitions.slice(0, count > allPetitions.length ? allPetitions.length : count);
        }

        allPetitionsOverview = []
        allPetitions.forEach(petition => {
            allPetitionsOverview.push(getPetitionOverview(petition));
        });

        if (badReq)
        {
            res.statusMessage = "Bad Request";
            res.status(400).send();
        }
        else 
        {
            res.statusMessage = "OK";
            res.status(200).send(allPetitionsOverview);
        }
    }
    catch (err) {
        console.log(`ERROR getting petitions ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.createPetition = async function(req, res) {
    function addHours(date, hours) {
        return new Date(date.getTime() + hours*60*60*1000);
    }
    
    try {
        const title = req.body.title;
        const description = req.body.description;
        const categoryId = req.body.categoryId;
        const closingDate = new Date(req.body.closingDate);
        const createdDate = new Date();
        const categories = await category.getAllCategorys()
    
        if (title === undefined || description === undefined || categoryId === undefined || !categories.find(item => item.categoryId === categoryId) || closingDate === undefined || closingDate < new Date()) {
            res.statusMessage = 'Bad Request'
            res.status(400).send();
        }
        else {
            const userId = await checkAuthedId(-1, req, res)
            if (userId) {
                const petitionId = await petitions.createPetition(title, description, categoryId, closingDate, createdDate, userId);
                res.statusMessage = 'Created'
                res.status(201).send({petitionId: petitionId});
            }
        }

    }
    catch (err) {
        console.log(`ERROR creating petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.getPetition = async function(req, res) {
    try {
        foundPetition = await petitions.getPetitionById(Number(req.params.petitionId) || 0);
        if (foundPetition) {
            res.statusMessage = 'OK';
            res.status(200).send(foundPetition);
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
    }
    catch (err) {
        console.log(`ERROR getting petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.modifyPetition = async function(req, res) {
    try {
        const petitionId = req.params.petitionId;
        const title = req.body.title;
        const description = req.body.description;
        const categoryId = Number(req.body.categoryId);
        const closingDate = new Date(req.body.closingDate);

        const foundPetition = await petitions.getPetitionById(petitionId);
        
        if (foundPetition) {
            foundPetition.categoryId = category.getCategoryId(foundPetition.category, await category.getAllCategorys());
            const newTitle = title || foundPetition.title;
            const newDescription = description || foundPetition.description;
            const newCategoryId = categoryId || foundPetition.categoryId;
            const newClosingDate = isNaN(closingDate.valueOf()) ? foundPetition.closingDate : closingDate;

            if ((isNaN(closingDate.valueOf()) || closingDate > new Date()) && category.getCategoryName(newCategoryId, await category.getAllCategorys())) {
                if (await checkAuthedId(foundPetition.authorId, req, res)) {
    
                    await petitions.updatePetition(newTitle, newDescription, newCategoryId, newClosingDate, petitionId);
                    res.statusMessage = 'OK';
                    res.status(200).send();
                }
            } else {
                res.statusMessage = 'Bad Request';
                res.status(400).send();
            }
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
    }
    catch (err) {
        console.log(`ERROR modifying petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.deletePetition = async function(req, res) {
    try {
        const petitionId = Number(req.params.petitionId) || 0;
        const foundPetition = await petitions.getPetitionById(petitionId);

        if (foundPetition) {
            if (await checkAuthedId(foundPetition.authorId, req, res)) {
                await petitions.deletePetition(petitionId);
                res.statusMessage = 'OK';
                res.status(200).send();
            }
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }

    }
    catch (err) {
        console.log(`ERROR deleting petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.getAllCategories = async function(req, res) {
    try {
        const categories = await category.getAllCategorys()
        res.statusMessage = 'OK';
        res.status(200).send(categories);
    }
    catch (err) {
        console.log(`ERROR deleting petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.getPetitionPhoto = async function(req, res) {
    try {
        const petitionId = req.params.petitionId;
        const [imageData, extension] = await petitions.getPhotoData(petitionId);
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
        console.log(`ERROR geting photo for petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.setPetitionPhoto = async function(req, res) {
    try {
        const mimeType = req.get('Content-Type');
        const extension = mime.getExtension(mimeType);
        const petitionId = Number(req.params.petitionId);
        const photoData = req.body;
        if (photoData === null || !petitionId || !['jpg', 'jpeg', 'png', 'gif'].includes(extension))
        {
            console.warn('Bad request' + photoData.length + petitionId + extension);
            res.statusMessage = 'Bad Request';
            res.status(400).send();
        }
        else {
            const foundPetition = await petitions.getPetitionById(petitionId);
            if (foundPetition) {
                if (await checkAuthedId(foundPetition.authorId, req, res)) {
                    let photoDeleted = await petitions.removePhoto(petitionId)
                    await petitions.savePhotoData(photoData, petitionId, extension);
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
    catch (err) {
        console.log(`ERROR setting photo for petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.getSignature = async function(req, res) {
    try {
        const petitionId = Number(req.params.petitionId);
        if (await petitions.getPetitionById(petitionId)) {
            const allSignatures = await signatures.getSignaturesForPetition(petitionId);
            res.statusMessage = 'OK';
            res.status(200).send(allSignatures)
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send()
        }
    }
    catch (err) {
        console.log(`ERROR getting signature for petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};

exports.signPetition = async function(req, res) {
    try {
        const petitionId = Number(req.params.petitionId);

        const foundPetition = await petitions.getPetitionById(petitionId);
        if (foundPetition) {
            const authedId = await checkAuthedId(-1, req, res)
            if (authedId) {
                const existingSignatures = await signatures.getSignaturesForPetition(petitionId);
                if (existingSignatures.find(signature => signature.signatoryId === authedId) || new Date(foundPetition.closingDate) < new Date()) {
                    res.statusMessage = 'Forbidden';
                    res.status(403).send();
                } else {
                    await signatures.signPetition(authedId, petitionId);
                    res.statusMessage = 'Created';
                    res.status(201).send();
                }
            }
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
    }
    catch (err) {
        console.log(`ERROR siging petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
        throw err;
    }
};

exports.unsignPetition = async function(req, res) {
    try {
        const petitionId = Number(req.params.petitionId);

        const foundPetition = await petitions.getPetitionById(petitionId);
        if (foundPetition) {
            const authedId = await checkAuthedId(-1, req, res)
            if (authedId) {
                const existingSignatures = await signatures.getSignaturesForPetition(petitionId);
                if (authedId === foundPetition.authorId || !existingSignatures.find(signature => signature.signatoryId === authedId) || new Date(foundPetition.closingDate) < new Date()) {
                    res.statusMessage = 'Forbidden';
                    res.status(403).send();
                } else {
                    await signatures.unsignPetition(authedId, petitionId);
                    res.statusMessage = 'OK';
                    res.status(200).send();
                }
            }
        } else {
            res.statusMessage = 'Not Found';
            res.status(404).send();
        }
    }
    catch (err) {
        console.log(`ERROR unsiging petition ${err}`);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};