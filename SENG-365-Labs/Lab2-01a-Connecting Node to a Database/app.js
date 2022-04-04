const mysql = require('mysql2');

const connection = mysql.createConnection({
    host: 'db2.csse.canterbury.ac.nz',
    user: '',
    password: '',
    database: '_d365_lab2'
})

connection.connect(err => {
    if (err) throw err;
    console.log('Connected!');

    connection.query('SELECT * FROM lab2_users', (err1, result1) => {
        if (err1) throw err1;
        
        connection.query('SELECT * FROM lab2_users WHERE username = ' + result1[0], (err2, result2) => {
            if (err2) throw err2;
            console.log('Users:' + result2);
        });
    });
});