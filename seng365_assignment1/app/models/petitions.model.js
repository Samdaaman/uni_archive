const db = require('../../config/db');
const category = require('./category.model');
const signature = require('../models/signatures.model');
const user = require('./users.model');
const crypto = require('crypto');
const fs = require('fs');

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

function getToken(req) {
    return utils.parseText(req.get('X-Authorization'));
}

async function checkAuthedId(userId, req, res) {
    // Returns true if auth is successful
    // UserId can be set to -1 for any user (only needs correct token)
    const token = getToken(req);
    const authedId = await users.getUserIdFromToken(token);
    if (authedId == null || userId == null) {
        res.statusMessage = 'Unauthorised'
        res.status(401).send();
    }
    else if (authedId != userId && userId != -1) {
        res.statusMessage = 'Forbidden';
        res.status(403).send();
    }
    else {
        return true;
    }
    return false;
};

function formatPetitionFromDB(petition, categories, signatures, userData)
{
    return {
        petitionId: petition.petition_id,
        title: petition.title,
        category: category.getCategoryName(petition.category_id, categories),
        authorName: user.getUsername(petition.author_id, userData),
        signatureCount: signatures[petition.petition_id] || 0,
        description: petition.description,
        authorId: petition.author_id,
        authorCity: user.getCity(petition.author_id, userData),
        authorCountry: user.getCountry(petition.author_id, userData),
        createdDate: new Date(petition.created_date),
        closingDate: new Date(petition.closing_date)
    };
}

exports.setToken = async function(userId, tokenValue) {
    const q = 'UPDATE User SET auth_token = ? WHERE user_id = ?';
    await db.getPool().query(q, [tokenValue, userId]);
};

exports.getAllPetitions = async function() {
    const q = 'SELECT * FROM Petition';
    const [rows, _] = await db.getPool().query(q);
    
    let petitions = [];
    let categories = await category.getAllCategorys();
    let signatures = await signature.getAllSignatures();
    let userData = await user.getAllUsersData();
    rows.forEach(row => petitions.push(formatPetitionFromDB(row, categories, signatures, userData)));
    return petitions;
};

exports.getPetitionById = async function(petitionId) {
    if (isNaN(petitionId)) {
        return null;
    }
    const q = 'SELECT * FROM Petition WHERE petition_id = ?'
    const [rows, _] = await db.getPool().query(q, [petitionId]);

    return rows.length > 0 ? formatPetitionFromDB(rows[0], await category.getAllCategorys(), await signature.getAllSignatures(), await user.getAllUsersData()) : null;
}

exports.createPetition = async function(title, description, categoryId, closingDate, createdDate, authorId) {
    const q = 'INSERT INTO Petition (title, description, category_id, closing_date, created_date, author_id) VALUES (?, ?, ?, ?, ?, ?)';
    const [result, _] = await db.getPool().query(q, [title, description, categoryId, closingDate, createdDate, authorId]);
    return result.insertId;
};

exports.updatePetition = async function(title, description, categoryId, closingDate, petitionId) {
    const q = 'UPDATE Petition SET title = ?, description = ?, category_id = ?, closing_date = ? WHERE petition_id = ?';
    const [_, __] = await db.getPool().query(q, [title, description, categoryId, closingDate, petitionId]);
};

exports.deletePetition = async function(petitionId) {
    const q1 = 'DELETE FROM Petition WHERE petition_id = ?';
    const [_, __] = await db.getPool().query(q1, [petitionId]);
};

exports.getPhotoData = async function(petitionId) {
    let contents = null
    let foundExtension = null;
    const extensions = ['jpg', 'jpeg', 'png', 'gif'];
    extensions.forEach(extension => {
        try {
            contents = fs.readFileSync('./storage/photos/petition_' + petitionId + '.' + extension);
            foundExtension = extension;
        } catch {}
    });
    return [contents, foundExtension];
};

exports.removePhoto = async function(petitionId) {
    let [contents, extension] = await exports.getPhotoData(petitionId);
    if (extension != null) {
        try {
            fs.unlinkSync('./storage/photos/petition_' + petitionId + '.' + extension)
            return true;
        }
        catch {}
    }
    return false;
};

exports.savePhotoData = async function(photoData, petitionId, extension) {
    fs.writeFileSync('./storage/photos/petition_' + petitionId + '.' + extension, photoData);
};