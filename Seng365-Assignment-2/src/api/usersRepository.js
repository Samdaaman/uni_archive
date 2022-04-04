import Repository from "./Repository";
import {authedUserId, authedUsername, setAuthedUserId, setAuthToken, updateAuthed} from "../utils/auth";

const resource = '/users'

export default {
  createUser(name, password, email, country, city) {
    let data = {
      name: name,
      email: email,
      password: password
    }
    if (country) { data = {...data, country: country}}
    if (city) { data = {...data, city: city}}
    return Repository.post(`${resource}/register`, data);
  },

  getAuthedUserPhotoUrl() { return this.getUserPhotoUrl(authedUserId) },
  getUserPhotoUrl(userId) { return `${resource}/${userId}/photo` },

  setUserPhoto(data, mime) {
    return Repository.put(this.getAuthedUserPhotoUrl(), data, {
      headers: {
        'Content-Type': mime
      }
    });
  },

  deleteUserPhoto() {
    return Repository.delete(this.getAuthedUserPhotoUrl());
  },

  async tryLogin(email, password) {
    const response = await Repository.post(`${resource}/login`, {
      email: email,
      password: password
    });

    if (response.data.token && response.data.userId) {
      setAuthToken(response.data.token);
      setAuthedUserId(response.data.userId);
      updateAuthed();
    }
    return response;
  },

  getSingle(userId) {
    return Repository.get(`${resource}/${userId}`);
  },

  async logout() {
    try {
      await Repository.post(`${resource}/logout`);
    }
    catch(error) {}
    await updateAuthed();
  },

  editUser(name, email, oldPassword, newPassword, country, city) {
    let data = {};
    if (name) { data = {...data, name: name}}
    if (email) { data = {...data, email: email}}
    if (oldPassword && newPassword) { data = {...data, currentPassword: oldPassword, password: newPassword}}
    if (country) { data = {...data, country: country}}
    if (city) { data = {...data, city: city}}
    return Repository.patch(`${resource}/${authedUserId}`, data)
  }
}
