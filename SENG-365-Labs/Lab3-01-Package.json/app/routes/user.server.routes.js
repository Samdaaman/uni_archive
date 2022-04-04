const users = require('../controllers/user.server.controller');

module.exports = function(app) {
    app.route('/api/users')
        .get(users.list)
        .post(users.create);

    app.route('/api/users/:userId')
        .get(users.read)
        .put(users.update)
        .delete(users.delete)
}