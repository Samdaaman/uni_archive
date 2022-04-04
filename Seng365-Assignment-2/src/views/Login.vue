<template>
  <div class="form-container">
    <form class="gen-form" @submit.prevent v-on:submit="formSubmit()">
      <h1 class="form-h1">Login</h1>
      <div v-if="errorFlag" id="error-div">{{error}}</div>
      <div v-if="successFlag" id="success-div">{{success}}</div>
      <hr class="form-hr">

      <div class="form-col-50" style="width: 100%;">
        <label class="form-label"><b>Email address</b></label>
        <input name="email" class="gen-form-input" type="email" placeholder="someone@example.com" v-model="email" required>
        <label class="form-label"><b>Password</b></label>
        <input name="password" class="gen-form-input" type="password" placeholder="Please enter a password" v-model="password" required>
      </div>
      <div class="form-button-container">
        <button class="btn btn-success form-button" type="submit">Login</button>
      </div>
    </form>
  </div>
</template>

<script>
  import Repositories from "../api/Repositories";
  import delay from "../utils/delay";

  export default {
    data () {
      return {
        errorFlag: false,
        error: '',
        successFlag: false,
        success: '',
        email: '',
        password:''
      }
    },
    created() {
      if (this.$route.query.error) {
        this.errorFlag = true;
        this.error = this.$route.query.error;
      } else if (this.$route.query.success) {
        this.successFlag = true;
        this.success = this.$route.query.success;
      }
    },
    methods: {
      formSubmit: async function() {
        try {
          await Promise.all([
            (async function() {
              await Repositories.Users.tryLogin(this.email, this.password);
              this.successFlag = true;
              this.success = 'User successfully logged in';
              this.errorFlag = false;
              this.error = '';
            }).bind(this)(),
            delay()
          ]);
          await this.$router.push('/');
        }
        catch (err) {
          this.successFlag = false;
          this.success = '';
          this.errorFlag = true;
          this.error = `Error logging in. Check email and password`;
        }
      }
    }
  }
</script>

<style scoped>

</style>
