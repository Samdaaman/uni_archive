const express = require('express');
app = express();

const data = require('./data.json');
const users = data.users;
const posts = data.posts;
console.log(posts[0]);

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
            break;
        }
    }

    res.send(users);
});

app.post('/follow/:id', function(req, res) {
    let id = req.params.id;
    let id_to_follow = req.body.id;

    for (let user of users) {
        if (id === user.id) {
            if (!user.following.includes(id_to_follow))
            {
                user.following.push(id_to_follow);
            }
            break;
        }
    }

    res.send(users);
});

app.post('/unfollow/:id', function(req, res) {
    let id = req.params.id;
    let id_to_unfollow = req.body.id;

    for (let user of users) {
        if (id === user.id) {
            let uid = user.following.indexOf(id_to_unfollow);
            if (uid != -1)
            {
                user.following.splice(uid, 1);
            }
            break;
        }
    }

    res.send(users);
});

app.get('/followers/:id', function(req, res) {
    let id = req.params.id;
    let res_data = 'User not found';
    for (let user of users) {
        if (id === user.id) {
            res_data = user.following;
        }
    }

    res.send(res_data);
});

app.post('/posts/:id', function(req, res) {
    let id = req.params.id;
    let content = req.body.content;

    posts.put({"id": id, "content": content});

    res.send(posts)
});

app.get('/posts/:id/first', function(req, res) {
    let id = req.params.id;
    let res_data = "User has not made any posts";

    for (let post of posts) {
        if (post.id === id) {
            res_data = post;
        }
    }

    res.send(res_data);
});


app.get('/posts/:id/all', function(req, res) {
    let id = req.params.id;
    let res_data = []

    for (let post of posts) {
        if (post.id === id) {
            res_data.push(post);
        }
    }
    if (res_data.length == 0){
        res_data = "User has not made any posts yet"
    }
    res.send(res_data);
});

app.listen(8081, function() {
    console.log('Started on port 8081');
});