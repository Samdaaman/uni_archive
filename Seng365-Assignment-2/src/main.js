import Vue from 'vue';
import App from './App.vue';
import router from './router/router';
import axios from 'axios';
import VueAxios from 'vue-axios';
import EventBus from "./utils/EventBus";

import TheNavigationBar from './components/TheNavigationBar';
import PetitionDetails from "./components/PetitionDetails";
import UserData from "./components/UserData";


Vue.use(VueAxios, axios);

Vue.component('the-navigation-bar', TheNavigationBar);
Vue.component('petition-details', PetitionDetails);
Vue.component('user-data', UserData)

Vue.prototype.$bus = EventBus;

new Vue({
  el: '#app',
  router: router,
  render: h => h(App),
});
