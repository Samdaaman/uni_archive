const express = require('express');

const app = express();

app.get('/', function(req, res) {
    res.send("Hello World!");
});

app.listen(8081, function () {
    console.log("Example app listening on port 8081!");
});

app.use(function (req, res, next) {
    res.status(404).send("404 Not Found");
});