const express = require('express'),
bodyParser = require('body-parser');

module.exports = function(){
    const app = express();
    app.use(bodyParser.json());
    require('../app/routes/user.server.routes')(app);
    return app;
};