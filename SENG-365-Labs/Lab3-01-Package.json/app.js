const db = require('./config/db');
const express = require('./config/express');

const app = express();

// Connect to MYSQL on start
async function main() {
    try {
        await db.connect();
        app.listen(process.env.SENG365_PORT, function() {
            console.log('Listening on port: ' + process.env.SENG365_PORT);
        });
    }
    catch (err) {
        console.log('Connection to MYSQL or listening failed');
        process.exit(1);
    }
}

main().catch(err => console.log(err));