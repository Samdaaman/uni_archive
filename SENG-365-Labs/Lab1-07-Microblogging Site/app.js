const express = require('express');
app = express();

const data = require('./users.json');
const users = data.users;
console.log(users[0]);

app.get('/users', function(req, res) {
    res.send(users);
});

app.get('/users/:id', function(req, res) {
    let id = req.params.id;
    let res_data = "No user";

    for (let user of users) {
        if (id === user.id) {
            res_data = user;
            break;
        }
    }

    res.send(res_data);
});

const bodyParser = require('body-parser');
app.use(bodyParser.json());

app.post('/users', function(req, res) {
    let user_data = req.body;

    users.push(user_data);
    res.send(users);
});

app.put('/users/:id', function(req, res) {
    let id = req.params.id;
    let user_data = req.body;

    for (let user of users) {
        if (id === user.id) {
            let uid = users.indexOf(user);
            users[uid] = user_data;
            break;
        }
    }

    res.send(users);
});

app.delete('/users/:id', function(req, res) {
    let id = req.params.id;
    
    for (let user of users) {
        if (id === user.id) {
            let uid = users.indexOf(user);
            users.splice(uid, 1);
        }
    }

    res.send(users);
});

app.listen(8081, function() {
    console.log('Started on port 8081');
});