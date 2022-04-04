require('dotenv').config();
const db = require('./config/db');
const express = require('./config/express');
const passwords = require('./app/utils/passwords');

const app = express();
const port = process.env.SENG365_PORT || 4941;

// Test connection to MySQL on start-up
async function testDbConnection() {
    try {
        await db.createPool();
        await db.getPool().getConnection();
    } catch (err) {
        console.error(`Unable to connect to MySQL: ${err.message}`);
        process.exit(1);
    }
}

testDbConnection()
    .then(function () {
        app.listen(port, function () {
            console.log(`Listening on port: ${port}`);
            console.log(`${process.env.SENG365_MYSQL_DATABASE} - ${process.env.SENG365_MYSQL_USER} - ${process.env.SENG365_MYSQL_HOST}`);
            setTimeout(() => console.log('Still going'), 10000);
            console.log(`hash of password is ${passwords.hash('password')}`);
        });
    });
