import PetitionsRepository from "./peititonsRepository";
import SignaturesRepository from "./signaturesRepository";
import UsersRepository from "./usersRepository";

const repositories = {
  Petitions: PetitionsRepository,
  Signatures: SignaturesRepository,
  Users: UsersRepository
};

export default repositories;
