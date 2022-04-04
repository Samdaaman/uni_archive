<template>
  <div v-if="loaded" class="form-container" style="max-width: 800px; min-width: 800px">
    <form class="gen-form" @submit.prevent v-on:submit="formSubmit()">
      <h1 class="form-h1">User Details</h1>
      <div v-if="errorFlag" id="error-div">{{error}}</div>
      <div v-if="successFlag" id="success-div">{{success}}</div>
      <hr class="form-hr">
      <div class="form-col-33" style="padding-right: 0; padding-top: 15px">
        <img v-bind:src="getUserPhotoUrl()" style="max-width: 100%; max-height: 100%"/>
        <label class="form-label"><b>Change Profile Picture:</b></label>
        <input :disabled="readonly" ref="image-upload" name="pictureUpload" class="gen-form-input" type="file" accept=".gif,.jpg,.jpeg,.png" v-on:change="pictureChanged">
        <button :disabled="readonly" v-on:click="deletePicture()" class="btn btn-danger form-button" style="margin: 5px auto 10px; width: 200px; height: 35px" type="button">Remove Profile Picture</button>
      </div>
      <div class="form-col-33">
        <label class="form-label"><b>Full Name:</b></label><label class="form-sub-label">Required</label>
        <input v-bind:disabled="readonly" name="name" class="gen-form-input" type="text" placeholder="Please enter your full name" v-model="name" required>

        <label class="form-label"><b>Old Password:</b></label><label class="form-sub-label">Required</label>
        <input v-bind:disabled="readonly" name="password" class="gen-form-input" type="password" placeholder="Please enter your old password" v-model="oldPassword">

        <label class="form-label"><b>New Password:</b></label><label class="form-sub-label">Required</label>
        <input :required="true" v-bind:disabled="readonly || !oldPassword" name="password" class="gen-form-input" type="password" placeholder="Please enter a new password" v-model="newPassword">

        <button v-on:click="editOrReset()" v-bind:class="readonly ? 'btn-info' : 'btn-danger'" class="btn form-button" style="margin-top: 25px" type="button">{{readonly ? "Edit Details" : "Discard Changes"}}</button>
      </div>
      <div class="form-col-33">
        <label class="form-label"><b>Email:</b></label><label class="form-sub-label">Required</label>
        <input v-bind:disabled="readonly" name="email" class="gen-form-input" type="email" placeholder="someone@example.com" v-model="email" required>

        <label class="form-label"><b>Country:</b></label><label class="form-sub-label">Optional</label>
        <input v-bind:disabled="readonly" name="country" class="gen-form-input" type="text" placeholder="What is your home country?" v-model="country">

        <label class="form-label"><b>City:</b></label><label class="form-sub-label">Optional</label>
        <input v-bind:disabled="readonly" name="city" class="gen-form-input" type="text" placeholder="What is your home city?" v-model="city">

        <button :disabled="readonly" class="btn btn-success form-button" style="margin-top: 25px" type="submit">Save Changes</button>
      </div>
      <div style="clear: both"></div>
    </form>
  </div>
</template>

<script>
  import Repositories from "../api/Repositories";
  import {isAuthed, updateAuthed} from "../utils/auth";
  import {BASE_URL} from "../api/Repository";

  export default {
    data () {
      return {
        loaded: false,
        readonly: true,
        errorFlag: false,
        error: '',
        successFlag: false,
        success: '',
        currentUserDetails: {
          name: '',
          email: '',
          country: '',
          city: '',
        },
        name: '',
        email: '',
        newPassword: '',
        oldPassword: '',
        country: '',
        city: '',
        pictureData: null,
        pictureMime: null
      }
    },
    async mounted() {
      await this.loadData();
      this.loaded = true;
    },
    methods: {
      pictureChanged(event) {
        this.pictureData = event.target.files[0];
        this.pictureMime = event.target.files[0].type;
      },
      getUserPhotoUrl() { return BASE_URL + Repositories.Users.getAuthedUserPhotoUrl() },
      async loadData() {
        const userDetailsResponse = await updateAuthed();
        if (!isAuthed()) {
          await this.$router.push('/login?error=Session Expired: Please log in again');
        } else {
          const userDetails = userDetailsResponse.data;
          this.name = userDetails.name;
          this.email = userDetails.email;
          this.country = userDetails.country;
          this.city = userDetails.city;
          this.oldPassword = '';
          this.newPassword = '';
          if (this.$refs['image-upload']) {
            this.$refs['image-upload'].value = null;
          }
          this.readonly = true;
        }
      },
      editOrReset() {
        this.successFlag = false;
        if (this.readonly) {
          this.readonly = false;
        } else {
          this.loadData();
        }
      },
      async deletePicture() {
        await Repositories.Users.deleteUserPhoto();
      },
      async formSubmit() {
        try {
          await Repositories.Users.editUser(this.name, this.email, this.oldPassword, this.newPassword, this.country, this.city);
          if (this.pictureData) {
            await Repositories.Users.setUserPhoto(this.pictureData, this.pictureMime);
          }
          this.successFlag = true;
          this.success = 'Your user details have been successfully updated';
          this.errorFlag = false;
          this.readonly = true;
        } catch (err) {
          this.successFlag = false;
          this.errorFlag = true;
          this.error = `Error editing user details: please check password is correct`
          console.log(JSON.stringify(err));
        }
      }
    }
  }
</script>

<style scoped>

</style>
