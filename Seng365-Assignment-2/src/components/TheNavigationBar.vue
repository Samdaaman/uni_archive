<template>
  <div>
    <ul class="navbar-ul">
      <li class="navbar-li" v-for="link in links" :key="link.id">
        <router-link v-if="link.auth === undefined || link.auth === authed" class="navbar-link" :to="link.path">{{link.text}}</router-link>
      </li>
      <li v-if="username">{{ `Welcome: ${username} ${userId}` }}</li>
      <li class="navbar-li navbar-li-right"><a class="navbar-link" v-on:click="reload()" href="#">Reload</a></li>
    </ul>
  </div>
</template>

<script>
  import {authedUserId, authedUsername} from "../utils/auth";
  import Paths from "../router/Paths";

  export default {
    name: "TheNavigationBar",
    data: () => {
      return {
        authed: false,
        username: '',
        userId: null,
        links: []
      };
    },
    created() {
      Object.values(Paths).forEach((link) => { if (link.text) {
        this.links.push({
          name: link.name,
          path: link.path,
          auth: link.auth,
          text: link.text
        })}});
      this.$bus.$on('auth-changed', function() {
        this.authed = authedUserId !== null;
        this.username = this.authed ? authedUsername : '';
        this.userId = authedUserId;
      }.bind(this));
    },
    methods: {
      reload: function() {
        this.$http.post('http://localhost:4941/api/v1/reload');
      }
    }
  }
</script>

<style scoped>
  .navbar-link {
    display: block;
    padding: 10px;
    border: 1px solid black;
    color: black;
  }
  .navbar-li:hover {
    background-color: #6ea8ff;
  }
  .navbar-li{
    float: left;
  }
  .navbar-li-right{
    float: right;
  }
  .navbar-ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    background-color: #e2e2e2;
    overflow: auto;
  }
  a {
    text-decoration: none;
  }
  .router-link-active {
    font-weight: bolder;
    color: black;
  }
</style>
