const petitions = require('../controllers/petitions.controller');

module.exports = function(app) {
    app.route(app.rootUrl + '/petitions')
        .get(petitions.getAllPetitions)
        .post(petitions.createPetition);
    app.route(app.rootUrl + '/petitions/categories')
        .get(petitions.getAllCategories);
    app.route(app.rootUrl + '/petitions/:petitionId')
        .get(petitions.getPetition)
        .patch(petitions.modifyPetition)
        .delete(petitions.deletePetition);
    app.route(app.rootUrl + '/petitions/:petitionId/photo')
        .get(petitions.getPetitionPhoto)
        .put(petitions.setPetitionPhoto);
    app.route(app.rootUrl + '/petitions/:petitionId/signatures')
        .get(petitions.getSignature)
        .post(petitions.signPetition)
        .delete(petitions.unsignPetition);
};