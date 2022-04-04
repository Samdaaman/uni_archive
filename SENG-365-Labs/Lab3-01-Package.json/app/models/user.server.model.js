const db = require('../../config/db');

exports.getAll = async function() {
    const connection = await db.get_pool().getConnection();
    const q = 'SELECT * From Users';
    const [rows, fields] = await connection.query(q);
    return rows;
};

exports.getOne = async function(userId) {
    const connection = await db.get_pool().getConnection();
    const q = 'SELECT * FROM Users WHERE user_id = ?';
    const [rows, _] = await connection.query(q, [userId]);
    return rows;
};

exports.insert = async function(username) {
    let values = [username];
    const connection = await db.get_pool().getConnection();
    const q = 'INSERT INTO Users (username) VALUES ?';
    const [result, _] = await connection.query(q, values);
    console.log(`Inserted user with id ${result.insertId}`);
    return result.insertId;
};

exports.alter = async function(user_id, new_username) {
    console.log(`altering user ${new_username}`);
    const connection = await db.get_pool().getConnection();
    const q = 'UPDATE Users SET username = ? WHERE user_id = ?'
    const [rows, _] = await connection.query(q, [new_username, user_id]);
    console.log(`Amender user with id: ${user_id}`);
    return rows;
};

exports.remove = async function(user_id) {
    const connection = await db.get_pool().getConnection();
    const q = 'DELETE FROM Users WHERE user_id = ?'
    const [rows, _] = await connection.query(q, [user_id]);
    console.log(`Deleted user with id: ${user_id}`);
    return rows;
};