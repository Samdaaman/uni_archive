require('dotenv').config({ path : '../.env' });
const mysql = require('mysql2/promise');
const express = require('express');
const bodyParser = require('body-parser');

const app = express();
app.use(bodyParser.json());

const pool = mysql.createPool({
    host: process.env.SENG365_MYSQL_HOST,
    user: process.env.SENG365_MYSQL_USER,
    password: process.env.SENG365_MYSQL_PASSWORD,
    database: process.env.SENG365_MYSQL_DATABASE
});

async function getUsers(req, res) {
    console.log('Request to get all users from the database');
    try {
        const connection = await pool.getConnection();
        console.log('Successfully connected to database');
        const [ rows, fields ] = await connection.query('select * from lab2_users');
        res.status(200).send(rows);
    }
    catch (err) {
        res.status(500).send(`ERROR getting users: ${errr}`);
    }
}

async function postUser(req, res) {
    console.log('Request to add a new user to the database');
    try {
        const connection = await pool.getConnection();
        const sql = 'insert into lab2_users (username) values ( ? )';
        const values = [req.body.username];

        await connection.query(sql, [values]);
        res.status(201).send('User successfully added to database');
    }
    catch (err) {
        res.status(500).send(`ERROR posting user ${err}`);
    }
}

app.get('/users', getUsers);
app.post('/users', postUser);

const port = process.env.port || 3000;
app.listen(port, () => {
    console.log(`Listening on port ${port}`);
});