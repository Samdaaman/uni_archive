require('dotenv').config();
const mysql = require('mysql2/promise');



async function main() {
    const pool = mysql.createPool({
        host: process.env.SENG365_MYSQL_HOST,
        user: process.env.SENG365_MYSQL_USER,
        password: process.env.SENG365_MYSQL_PASSWORD,
        database: process.env.SENG365_MYSQL_DATABASE
    });

    const connection = await pool.getConnection();
    const sql = 'insert into lab2_users (username) values ?';
    const values = [
        [ 'James' ],
        [ 'Lotte' ],
        [ 'Adrien' ]
    ];
    const [ result ] = await connection.query(sql, [values]);
    console.log(result.affectedRows);
    const [ rows ] = await connection.query('select * from lab2_users');
    connection.release();
    console.log('Users:', rows);
}

main()
    .catch(err => console.error(err));