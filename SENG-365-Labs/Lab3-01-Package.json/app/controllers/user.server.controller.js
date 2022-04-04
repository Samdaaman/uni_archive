const User = require('../models/user.server.model');

exports.list = async function(req, res) {
    try {
        const result = await User.getAll();
        res.status(200).send(result);
    }
    catch (err) {
        res.status(500).send(`ERROR getting users ${err}`);
    }
};

exports.create = async function(req, res) {
    try {
        let user_data = {
            "username" : req.body.username
        };

        let user = user_data['username'].toString();

        let values = [
            [user]
        ];

        const result = await User.insert(values);
        res.status(201).send(`Inserted ${req.body.username} at id ${result}`);
    }
    catch (err) {
        res.status(500).send(`ERROR posting user ${err}`);
    }
};

exports.read = async function(req, res) {
    try {
        const id = req.params.userId;
        const result = await User.getOne(id);
        res.status(200).send(result);
    }
    catch (err) {
        res.status(500).send(`ERROR fetching user ${err}`);
    }
};

exports.update = async function(req, res) {
    try {
        const id = req.params.userId;
        
        let user_data = {
            "username" : req.body.username
        };

        let user = user_data['username'].toString();

        const result = await User.alter(id, user);
        res.status(200).send(result);
    }
    catch (err) {
        res.status(500).send(`ERROR updating user ${err}`);
    }
};

exports.delete = async function(req, res) {
    try {
        const id = req.params.userId;
        const result = await User.remove(id);
        res.status(200).send(result);
    }
    catch (err) {
        console.log(`ERROR deleting user ${err}`);
    }
};

exports.userById = async function(req, res) {
    return null;
};