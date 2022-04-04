const users = require('../controllers/users.controller');

module.exports = function(app) {
    app.route(app.rootUrl + '/users/login')
        .post(users.login);
    
    app.route(app.rootUrl + '/users/logout')
        .post(users.logout);

    app.route(app.rootUrl + '/users/register')
        .post(users.register);
    
    app.route(app.rootUrl + '/users/:userId')
        .get(users.getOneUser)
        .patch(users.updateUser);

    app.route(app.rootUrl + '/users/:userId/photo')
        .get(users.getUserPhoto)
        .put(users.uploadUserPhoto)
        .delete(users.deleteUserPhoto);
}