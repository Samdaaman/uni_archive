import Repositories from "../api/Repositories";
import EventBus from "./EventBus";

export let authedUsername = '';
let authUpdating = false

export function isAuthed() {
  return authedUserId !== null;
}

export const updateAuthed = async function() {
  let response = null;
  if (!authUpdating) {
    authUpdating = true;
    let newAuthedUsername = '';
    if (authedUserId) {
      try{
        response = await Repositories.Users.getSingle(authedUserId);
        if (response.data.email) {
          newAuthedUsername = response.data.name;
        }
      }
      catch(error) {}
    }
    if (authedUsername !== newAuthedUsername) {
      authedUsername = newAuthedUsername;
      authedUserId = authedUsername ? authedUserId : null;
      console.log(`Auth state changed, now logged in as: ${authedUserId}"${authedUsername}"`);
      EventBus.$emit("auth-changed");
    }
    authUpdating = false;
  }
  return response;
}

export let authedUserId = null;
export const setAuthedUserId = function(userId) { authedUserId = userId };
export let authToken = '';
export const setAuthToken = function(token) { authToken = token; console.log('Set new auth token') };
