import Repository from "./Repository";

export const resource = '/petitions';

export default {
  getAll() {
    return Repository.get(resource);
  },
  getSingle(petitionId) {
    return Repository.get(`${resource}/${petitionId}`)
  },
  getAllWithFilter(sortByKey, searchText, categoryId) {
    let additionalParams = ''
    if (searchText !== '') {
      additionalParams += `&q=${searchText}`;
    }
    if (categoryId !== -1) {
      additionalParams += `&categoryId=${categoryId}`;
    }
    return Repository.get(`${resource}/?sortBy=${sortByKey}${additionalParams}`);
  },
  getAllCategories() {
    return Repository.get(`${resource}/categories`);
  },
  getPhotoUrl(petitionId) {
    return `${resource}/${petitionId}/photo`
  },
  editPetition(petition) {
    return Repository.patch(`${resource}/${petition.petitionId}`, petition)
  },
  createPetition(petition) {
    console.log(JSON.stringify(petition))
    return Repository.post(`${resource}`, petition)
  }
}
