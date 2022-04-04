<template>
  <div v-if="!loading" style="padding: 10px">
    <img :src="getPhotoUrl()" style="max-width: 150px; max-height: 100px; float: left" onerror="this.onerror=null;this.src='/src/assets/default-hero.png';"/>
    <div style="float: left; margin: 10px 0">
      <ul style="list-style: none; font-size: large">
        <li v-if="showName"><span style="font-size: larger"><b>{{name}}</b></span></li>
        <li v-if="showLocation && country"><span><b>Country: </b>{{country}}</span></li>
        <li v-if="showLocation && city"><span><b>City: </b>{{city}}</span></li>
      </ul>
    </div>
    <div style="clear: both"></div>
  </div>
</template>

<script>
  import Repositories from "../api/Repositories";
  import {BASE_URL} from "../api/Repository";
  import {authedUserId} from "../utils/auth";

  export default {
    props: ['showName', 'showLocation', 'userId'],
    data() {
      return {
        loading: true,
        name: '',
        country: '',
        city: '',
      }
    },
    async created() {
      const userData = (await Repositories.Users.getSingle(this.userId ? this.userId : authedUserId)).data;
      this.name = userData.name;
      this.country = userData.country;
      this.city = userData.city;
      this.loading = false;
    },
    methods: {
      getPhotoUrl() {
        return BASE_URL + Repositories.Users.getUserPhotoUrl(this.userId ? this.userId : authedUserId);
      }
    }
  }
</script>

<style scoped>

</style>
