const db = require('../../config/db');
const crypto = require('crypto');
const fs = require('fs');
const passwords = require('../utils/passwords');

exports.getAllUsersData = async function() {
    const q = 'SELECT user_id, name, city, country FROM User';
    const [rows, _] = await db.getPool().query(q);

    userData = []
    rows.forEach(row => userData.push({
        userId: row.user_id,
        name: row.name,
        city: row.city,
        country: row.country
    }));
    return userData;
}

exports.getUsername = function(userId, usernames) {
    const user = usernames.find(user => user.userId === userId);
    if (user) { 
        return user.name;
    } else {
        return 'username not found';
    }
};

exports.getCity = function(userId, userData) {
    const user = userData.find(user => user.userId === userId);
    if (user) {
        return user.city;
    } else {
        return 'city not found';
    }
};

exports.getCountry = function(userId, userData) {
    const user = userData.find(user => user.userId === userId);
    if (user) {
        return user.country;
    } else {
        return 'country not found';
    }
};

exports.tryLogin = async function(email, password) {
    const q = 'SELECT * FROM User WHERE email = ? AND password = ?';
    const [rows, _] = await db.getPool().query(q, [email, passwords.hash(password)]);
    if (rows.length === 0)
    {
        return [null, null];
    }
    
    const userId = rows[0].user_id;
    const token = generateToken();
    await exports.setToken(userId, token);
    return [userId, token];
};

function generateToken() {
    return crypto.randomBytes(24).toString('base64');
};

exports.getUserIdFromToken = async function(token) {
    if (token == '' || token == null)
    {
        return null;
    }
    const q = 'SELECT * FROM User WHERE auth_token = ?';
    const [rows, _] = await db.getPool().query(q, [token]);
    if (rows.length === 0)
    {
        return null;
    }
    return rows[0].user_id;
};

exports.setToken = async function(userId, tokenValue) {
    const q = 'UPDATE User SET auth_token = ? WHERE user_id = ?';
    await db.getPool().query(q, [tokenValue, userId]);
};

exports.create = async function(name, email, password, city, country) {
    // TODO store the hash of the password not the actual password??
    const q = 'INSERT INTO User (name, email, password, city, country) VALUES (?, ?, ?, ?, ?)';
    const [result, _] = await db.getPool().query(q, [name, email, passwords.hash(password), city, country]);
    return result.insertId;
};

exports.getUserIdFromEmail = async function(email) {
    const q = 'SELECT * FROM User WHERE email = ?';
    const [rows, _] = await db.getPool().query(q, [email]);
    if (rows.length === 0)
    {
        return null;
    }
    return rows[0].user_id;
};


exports.getUserData = async function(userId) {
    const q = 'SELECT * FROM User WHERE user_id = ?';
    const [rows, _] = await db.getPool().query(q, [userId]);
    if (rows.length === 0)
    {
        return null;
    }
    return rows[0];
};

exports.tryUpdate = async function(userId, name, email, password, currentPassword, city, country) {
    if (password == '') {
        // Password if supplied with no currentPassword
        return false;
    }
    else if (email && !email.includes('@'))
    {
        // Invalid email
        return false
    }
    else {
        user = await exports.getUserData(userId)
        if ((name == null) &&
            (email == null) &&
            (city == null) &&
            (country == null) &&
            (password == null)) {
                // There is no data to update 
                return false;
        }
        else if (passwords.hash(currentPassword) != user.password && password != null) {
            // Password incorrect
            return false;
        }
        else if (await exports.getUserIdFromEmail(email) && await exports.getUserIdFromEmail(email) != userId) {
            // User already exists with email
            return false;
        }
        else {
            new_name = (name != null) ? name : user.name;
            new_email =  (email != null) ? email : user.email;
            new_password_hash = (password != null) ? passwords.hash(password) : user.password;
            new_city = (city != null) ? city : user.city;
            new_country = (country != null) ? country : user.country;

            q = 'UPDATE User SET name = ?, email = ?, password = ?, city = ?, country = ? WHERE user_id = ?';
            await db.getPool().query(q, [new_name, new_email, new_password_hash, new_city, new_country, userId]);
            return true;
        }
    }
};

exports.getPhotoData = async function(userId) {
    let contents = null
    let foundExtension = null;
    const extensions = ['jpg', 'jpeg', 'png', 'gif'];
    extensions.forEach(extension => {
        try {
            contents = fs.readFileSync('./storage/photos/user_' + userId + '.' + extension);
            foundExtension = extension;
        } catch {}
    });
    return [contents, foundExtension];
};

exports.removePhoto = async function(userId) {
    let [contents, extension] = await exports.getPhotoData(userId);
    if (extension != null) {
        try {
            fs.unlinkSync('./storage/photos/user_' + userId + '.' + extension)
            return true;
        }
        catch {}
    }
    return false;
};

exports.savePhotoData = async function(photoData, userId, extension) {
    fs.writeFileSync('./storage/photos/user_' + userId + '.' + extension, photoData);
};