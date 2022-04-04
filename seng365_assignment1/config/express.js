const express = require('express');
const bodyParser = require('body-parser');
const { allowCrossOriginRequestsMiddleware } = require('../app/middleware/cors.middleware');


module.exports = function () {
    // INITIALISE EXPRESS //
    const app = express();
    app.rootUrl = '/api/v1';

    // DEBUG (you can remove these)
    app.use((req, res, next) => {
        /*if (req.path.includes('users') || 1==1)
        {
            console.log(`##### ${req.method} ${req.path} ${req.params} #####`);
        }*/
        next();
    });

    // MIDDLEWARE
    app.use(allowCrossOriginRequestsMiddleware);
    app.use(bodyParser.json());
    app.use(bodyParser.raw({ type: '*/*', limit: '50mb' }));
    app.use(bodyParser.raw({ type: 'image/jpeg', limit: '50mb' }));
    app.use(bodyParser.raw({ type: 'image/gif', limit: '50mb' }));
    app.use(bodyParser.raw({ type: 'image/png', limit: '50mb' }));
    app.use(bodyParser.raw({ type: 'text/plain' }));  // for the /executeSql endpoint


    app.get('/', function (req, res) {
        res.send({ 'message': 'Hello World!' })
    });

    // ROUTES
    require('../app/routes/backdoor.routes')(app);
    require('../app/routes/users.routes')(app);
    require('../app/routes/petitions.routes')(app);

    return app;
};
