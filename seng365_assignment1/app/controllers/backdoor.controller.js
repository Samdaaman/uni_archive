const Backdoor = require('../models/backdoor.model');

exports.resetDb = async function (req, res) {
    try {
        await Backdoor.resetDb();
        res.statusMessage = "OK";
        res.status(200).send();
    } catch (err) {
        console.log(err);
        res.statusMessage = "Internal Server Error";
        res.status(500).send();
    }
};

exports.resample = async function (req, res) {
    try {
        await Backdoor.loadData();
        res.statusMessage = "Created";
        res.status(201).send();
    } catch (err) {
        console.log(err);
        res.statusMessage = "Internal Server Error";
        res.status(500).send();
    }
};

exports.reload = async function (req, res) {
    try {
        await Backdoor.resetDb();
        await Backdoor.loadData();
        res.statusMessage = "Created";
        res.status(201).send();
    } catch (err) {
        console.log(err);
        res.statusMessage = "Internal Server Error";
        res.status(500).send();
    }
};

exports.executeSql = async function (req, res) {
    const sqlCommand = String(req.body);
    try {
        const results = await Backdoor.executeSql(sqlCommand);
        res.statusMessage = 'OK';
        res.status(200).json(results);
    } catch (err) {
        if (!err.hasBeenLogged) console.error(err);
        res.statusMessage = 'Internal Server Error';
        res.status(500).send();
    }
};
