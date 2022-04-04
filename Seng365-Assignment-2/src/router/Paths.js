import Home from "../views/Home";
import Petitions from "../views/Petitions";
import Login from "../views/Login";
import Logout from "../views/Logout";
import Register from "../views/Register";
import Profile from "../views/Profile";
import PetitionEdit from "../views/PetitionEdit";
import PetitionCreate from "../views/PetitionCreate";

export default {
  Home: {
    path: '/',
    name: 'home',
    component: Home,
    text: "Home"
  },
  Petitions: {
    path: "/petitions",
    name: "petitions",
    component: Petitions,
    text: "Petitions"
  },
  PetitionEdit: {
    path: "/petitions/:petitionId",
    name: "petitionEdit",
    component: PetitionEdit,
  },
  PetitionCreate: {
    path: "/new-petition",
    name: "petitionCreate",
    component: PetitionCreate,
    text: 'Create Petition'
  },
  Profile: {
    path: "/profile",
    name: "profile",
    component: Profile,
    text: "Profile",
    auth: true
  },
  Login: {
    path: "/login",
    name: "login",
    component: Login,
    text: "Login",
    auth: false
  },
  Logout: {
    path: "/logout",
    name: "logout",
    component: Logout,
    text: "Logout",
    auth: true
  },
  Register: {
    path: "/register",
    name: "register",
    component: Register,
    text: "Register",
    auth: false
  }
}
