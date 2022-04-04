import Repository from "./Repository";
import { resource as petitionsResource } from "./peititonsRepository";

const resource = function(petitionId) { return `${petitionsResource}/${petitionId}/signatures`; };

export default {
  getSignatories(petitionId) {
    return Repository.get(resource(petitionId));
  },
  signPetition(petitionId) {
    return Repository.post(resource(petitionId));
  },
  unsignPetition(petitionId) {
    return Repository.delete(resource(petitionId));
  }
}
