const mysql = require('mysql2/promise');
require('dotenv').config();
let state = {
    pool: null
};

exports.connect = async function () {
    state.pool = await mysql.createPool({
    host: process.env.SENG365_MYSQL_PORT,
    user: process.env.SENG365_MYSQL_USER,
    password: process.env.SENG365_MYSQL_PASSWORD,
    database: process.env.SENG365_MYSQL_DATABASE,
    });

await state.pool.getConnection(); //Check connection
console.log('Successfully connected to the database');
};

exports.get_pool = function() {
    return state.pool;
};