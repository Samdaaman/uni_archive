import VueRouter from "vue-router";
import Vue from "vue";
import Paths from "./Paths";

const routes = [];
Object.values(Paths).forEach((route) =>
  routes.push({
    name: route.name,
    path: route.path,
    component: route.component
  }));

Vue.use(VueRouter);

export default  new VueRouter({
  routes: routes,
  mode: 'history'
});
