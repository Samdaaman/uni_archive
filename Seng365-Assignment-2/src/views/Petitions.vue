<template>
  <div id="global-container" class="container">
    <div id="create-container" class="container">
      <button type="button" class="btn btn-success">Create New Petition</button>
    </div>

    <div id="sort-container" class="container">
      <table>
        <tr>
          <td>
            <p>Search Petitions: </p>
          </td>
          <td>
            <input type="text" placeholder="Search Petitions" v-model="searchText">
          </td>
        </tr>
        <tr>
          <td>
            <p>Sort By: </p>
          </td>
          <td>
            <select id="sorting-mode" v-model="sortByKey">
              <option v-for="option in sortOptions" v-bind:value="option.key">{{ option.name }}</option>
            </select>
          </td>
        </tr>
        <tr>
          <td>
            <p>Category Filter: </p>
          </td>
          <td>
            <select id="category-filter" v-model="categoryFilterId">
              <option v-for="category in categories" v-bind:value="category.categoryId">{{ category.name }}</option>
            </select>
          </td>
        </tr>
      </table>


      <button type="button" class="btn btn-primary" v-on:click="getPetitions()">Apply Filter</button>
    </div>

    <div v-if="errorFlag" style="color: red;">
      {{ error }}
    </div>

    <div v-else id="petitions-container" class="container">
      <table id="petitions-table">
        <tr id="header-row">
          <th width="200px"></th>
          <th width="200px">Title</th>
          <th width="100px">Category</th>
          <th width="100px">Author Name</th>
          <th width="100px" style="text-align: center">Signatures</th>
          <th width="100px" style="text-align: center">Details</th>
        </tr>
        <tr class="petition-row" v-for="petition in petitions">
          <td><img style="max-height: 100px; max-width: 200px;" :src="'http://localhost:4941/api/v1/petitions/' + petition.petitionId + '/photo'"></td>
          <td>{{ petition.title }}</td>
          <td>{{ petition.category }}</td>
          <td>{{ petition.authorName }}</td>
          <td style="text-align: center">{{ petition.signatureCount }}</td>
          <td style="text-align: center"><router-link :to="{ name: 'petitionEdit', params: { petitionId: `${petition.petitionId}` }}"><button type="button" class="btn btn-primary">Details</button></router-link></td>
        </tr>
      </table>
    </div>
  </div>
</template>

<script>
  import Repositories from "../api/Repositories";

  export default {
    data () {
      return {
        error: "",
        errorFlag: false,
        petitions: [],
        sortOptions: [
          {
            key: 'ALPHABETICAL_ASC',
            name: 'Titles A-Z'
          },
          {
            key: 'ALPHABETICAL_DESC',
            name: 'Titles Z-A'
          },
          {
            key: 'SIGNATURES_ASC',
            name: 'Signatures Lowest-Highest'
          },
          {
            key: 'SIGNATURES_DESC',
            name: 'Signatures Highest-Lowest'
          }
        ],
        sortByKey: 'ALPHABETICAL_ASC',
        searchText: '',
        categories: [{
          categoryId: -1,
          name: "All"
        }],
        categoryFilterId: -1
      }
    },
    created: function() {
      this.getPetitions();
      this.getCategories();
    },
    methods: {
      async getPetitions() {
        this.petitions = (await Repositories.Petitions.getAllWithFilter(this.sortByKey, this.searchText, this.categoryFilterId)).data;
      },
      async getCategories() {
        const categoriesTemp = [{
          categoryId: -1,
          name: "All"
        }];
        const categoriesFromApi = (await Repositories.Petitions.getAllCategories()).data;
        for (let i = 0; i < categoriesFromApi.length; i++) {
          categoriesTemp.push(categoriesFromApi[i]);
        }
        this.categories = categoriesTemp;
      }
    }
  }
</script>

<style scoped>
  #global-container {
    height: auto;
    width: 100%;
  }

  #petitions-container {
    width: fit-content;
    height: fit-content;
    padding: 10px;
  }

  .container {
    display: flex;
    flex-direction: column;
    border: solid 2px black;
  }

  #petitions-table {

  }

  #header-row {
    font-weight: bold;
    font-size: larger;
    background-color: darkblue;
    color: white;
  }

  td, th {
    padding: 5px;
  }

  .petition-row {
    background-color: lightblue;
  }
</style>
