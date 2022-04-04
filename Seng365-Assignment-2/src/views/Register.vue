<template>
  <div class="form-container">
    <form class="gen-form" @submit.prevent v-on:submit="formSubmit()">
      <h1 class="form-h1">Registration</h1>
      <div v-if="errorFlag" id="error-div">{{error}}</div>
      <div v-if="successFlag" id="success-div">{{success}}</div>
      <hr class="form-hr">
      <div class="form-col-50">
        <label class="form-label"><b>Full Name:</b></label><label class="form-sub-label">Required</label>
        <input name="name" class="gen-form-input" type="text" placeholder="Please enter your full name" v-model="name" required>

        <label class="form-label"><b>Email:</b></label><label class="form-sub-label">Required</label>
        <input name="email" class="gen-form-input" type="email" placeholder="someone@example.com" v-model="email" required>

        <label class="form-label"><b>Password:</b></label><label class="form-sub-label">Required</label>
        <input name="password" class="gen-form-input" type="password" placeholder="Please enter a password" v-model="password" required>
      </div>
      <div class="form-col-50">
        <label class="form-label"><b>Country:</b></label><label class="form-sub-label">Optional</label>
        <input name="country" class="gen-form-input" type="text" placeholder="What is your home country?" v-model="country">

        <label class="form-label"><b>City:</b></label><label class="form-sub-label">Optional</label>
        <input name="city" class="gen-form-input" type="text" placeholder="What is your home city?" v-model="city">

        <input name="pictureUpload" class="gen-form-input" type="file" accept=".gif,.jpg,.jpeg,.png" v-on:change="pictureChanged">
      </div>
      <div style="clear: both; padding: 0 15px 0">
        <button class="btn btn-success form-button" style="margin-top: 25px" type="submit">Create User</button>
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
          name: '',
          email: '',
          password:'',
          country: '',
          city: '',
          pictureData: null,
          pictureMime: null
        }
      },
      methods: {
        pictureChanged(event) {
          this.pictureData = event.target.files[0];
          this.pictureMime = event.target.files[0].type;
        },
        async formSubmit() {
          try {
            await Promise.all([
              (async function () {
                try{
                  await Repositories.Users.createUser(this.name, this.password, this.email, this.country, this.city);
                  await Repositories.Users.tryLogin(this.email, this.password);
                  if (this.pictureData) {
                    console.log('uploading photo')
                    await Repositories.Users.setUserPhoto(this.pictureData, this.pictureMime);
                  } else {console.log('didn"t upload photo')}
                } catch(error) { throw error }
              }).bind(this)(),
              delay()
            ]);
            this.successFlag = true;
            this.success = 'User successfully created';
            this.errorFlag = false;
            this.error = '';
            await this.$router.push('/');
          }
          catch (err) {
            this.errorFlag = true;
            this.error = `Error creating user - email already in use`
            throw err;
          }
        }
      }
    }
</script>

<style scoped>

</style>
