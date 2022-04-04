<template>
  <div>
    <router-link :to="{ name: 'petitions' }">Back to Petitions</router-link>
    <div v-if="errorFlag" id="error-div">{{error}}</div>
    <div v-if="successFlag" id="success-div">{{success}}</div>
    <div v-if="loaded" id="petition-container" >
      <h2 v-if="!editable">{{petition.title}}</h2>
      <h2 v-if="create">Create New Petition</h2>
      <form @submit.prevent v-on:submit="formSubmit()">
        <div v-if="editable && !create" style="padding: 12px">
          <input type="text" style="font-size: x-large; width: 100%" v-model="petition.title" required>
        </div>
        <div class="d-l" style="margin-top: 20px">
          <div v-if="canEdit && !create">
            <div class="d-l">
              <button @click="editOrReset" class="btn form-button-small" v-bind:class="editable ? 'btn-danger' : 'btn-info'" type="button">{{ editable ? 'Discard Changes' : 'Edit Petition'}}</button>
            </div>
            <div class="d-r">
              <button :disabled="!editable" class="btn btn-success form-button-small" type="submit">Save Changes</button>
            </div>
          </div>
          <div v-if="!create" style="padding: 10px 0; clear: both">
            <img v-bind:src="photoUrl" style="width: 100%"/>
          </div>

          <label v-if="create">Petition Title</label>
          <input v-if="create" v-model="petition.title" class="gen-form-input" type="text" placeholder="Enter Petition Title" style="font-size: larger; margin-bottom: 20px" required>

          <label v-if="create">Petition Photo</label>
          <input v-if="editable" ref="image-upload" name="pictureUpload" class="gen-form-input" type="file" accept=".gif,.jpg,.jpeg,.png" :change="changedPicture">

          <div v-if="create">
            <label class="form-label">Category</label>
            <select :disabled="!editable" v-model="petition.categoryId">
              <option v-for="category in categories" v-bind:value="category.categoryId">{{ category.name }}</option>
            </select>
          </div>
        </div>
        <div class="d-r">
          <label class="form-label">Description</label>
          <textarea :disabled="!editable" class="gen-form-input" v-model="petition.description" required></textarea>

          <div v-if="!create">
            <label class="form-label">Created Date</label>
            <input disabled="true" type="text" class="gen-form-input" v-model="petition.createdDate">
          </div>

          <label class="form-label">Closing Date</label>
          <input :disabled="!editable" type="text" class="gen-form-input" v-model="petition.closingDate">

          <div v-if="!create">
            <label class="form-label">Category</label>
            <select :disabled="!editable" v-model="petition.categoryId">
              <option v-for="category in categories" v-bind:value="category.categoryId">{{ category.name }}</option>
            </select>
          </div>
        </div>
        <div v-if="editable && loaded && !create" style="clear: both">
          <button class="btn btn-danger form-button-small" style="margin-top: 10px">Delete Petition</button>
        </div>
        <div v-if="editable && loaded && create" style="clear: both">
          <button class="btn btn-success form-button-small" style="margin-top: 10px" type="submit">Create Petition</button>
        </div>
      </form>
      <div style="clear: both; padding-top: 1px">
        <hr>
        <h3>About the Author - {{create ? authedUsername : petition.authorName}}</h3>
        <user-data :show-name="false" :user-id="create ? null : petition.authorId" :show-location="true"/>
      </div>
      <div style="clear: both"></div>
      <div v-if="!create" style="padding: 10px 0 20px">
        <hr>
        <div>
          <h3 style="display: inline-block">Signatories (Total: {{signatories.length}})</h3>
          <button style="margin-left: 10px; margin-top: -5px; padding: 2px 10px" class="btn btn-info" data-toggle="modal" data-target="#signatories-modal">View Details</button>
        </div>
        <p style="display: inline-block; margin: 0 5px 20px; background-color: #ffe8ab; padding: 5px 10px" v-for="signatory in signatories">{{signatory.name}}</p>
        <button type="button" class="btn btn-success form-button" v-if="canSign && !hasSigned" @click="signPetition">Sign This Petition</button>
        <button type="button" class="btn btn-danger form-button" v-if="canSign && hasSigned" @click="unsignPetition">Unsign This Petition</button>
      </div>
    </div>
    <div class="modal" id="signatories-modal">
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <h3 style="display: inline-block">Details of Signatories</h3>
            <button type="button" class="close" data-dismiss="modal">&times;</button>
          </div>
          <ul class="n-ul" style="margin: 20px; overflow: hidden">
            <li style="overflow: hidden" v-for="signatory in signatories">
              <user-data :user-id="signatory.signatoryId" :show-location="true" :show-name="true"/>
            </li>
          </ul>
          <div style="clear: both;"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Repositories from "../api/Repositories";
  import {BASE_URL} from "../api/Repository";
  import {authedUserId, authedUsername, isAuthed} from "../utils/auth";

  export default {
    props: {
      create: Boolean,
      petitionId: String
    },
    data() {
      return {
        error: "",
        errorFlag: false,
        success: "",
        successFlag: false,
        loaded: false,
        petition: null,
        signatories: [],
        editable: false,
        canEdit: true,
        photoUrl: '',
        categories: [],
        canSign: true,
        hasSigned: false,
        authedUserId: null,
        authedUsername: ''
      }
    },
    async created() {
      this.authedUsername = authedUsername;
      this.authedUserId = authedUserId;
      if (!this.create)
        await this.load();
      else {
        this.editable = true;
        this.petition = {
          title: '',
          description: '',
          categoryId: null,
          closingDate: '2020-01-18 11:00:00.000',
          authorId: this.authedUserId
        }
      }
      await this.updateCategories();
      this.loaded = true;
    },
    methods: {
      async updateCategories() {
        this.categories = (await Repositories.Petitions.getAllCategories()).data;
      },
      async updateSignatories() {
        this.signatories =  (await Repositories.Signatures.getSignatories(this.$route.params.petitionId)).data;
        this.canSign = this.petition.authorId !== this.authedUserId && isAuthed();
        this.hasSigned = this.signatories.some(function(signatory) { return signatory.signatoryId === this.authedUserId }.bind(this))
      },
      async load() {
        try {
          this.petition = (await Repositories.Petitions.getSingle(this.$route.params.petitionId)).data;
          this.canEdit = this.petition.petitionId === authedUserId || true;
          await this.updateSignatories();
          this.photoUrl = BASE_URL + Repositories.Petitions.getPhotoUrl(this.petition.petitionId);
          await this.updateCategories();
          this.loaded = true;
        }
        catch(error) {
          this.successFlag = false;
          this.errorFlag = true;
          this.error = `Couldn't load petitions error: ${error}`
        }
      },
      async editOrReset() {
        if (this.editable) {
          await this.load();
          this.editable = false;
        } else {
          this.editable = true;
        }
      },
      async formSubmit() {
        if (this.create) {
          await Repositories.Petitions.createPetition({
            title: this.petition.title,
            description: this.petition.description,
            categoryId: this.petition.categoryId,
            closingDate: this.petition.closingDate
          })
        }
        else {
          await Repositories.Petitions.editPetition(this.petition);
          this.editable = false;
        }
      },
      async changedPicture(event) {

      },
      async signPetition() {
        await Repositories.Signatures.signPetition(this.petition.petitionId);
        await this.updateSignatories();
      },
      async unsignPetition() {
        await Repositories.Signatures.unsignPetition(this.petition.petitionId);
        await this.updateSignatories();
      }
    }
  }
</script>

<style scoped>
  #petition-container {
    max-width: 800px;
    min-width: 700px;
    padding: 0 20px;
    border: 1px solid black;
    margin: auto auto 20px;
  }
</style>
