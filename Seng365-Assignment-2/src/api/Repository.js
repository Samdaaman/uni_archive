import axios from "axios";
import {authToken, updateAuthed} from "../utils/auth";

export const BASE_URL = 'http://localhost:4941/api/v1';

const axiosInstance = axios.create({
  baseURL: BASE_URL
})

axiosInstance.interceptors.request.use(
  request => {
    if (authToken) {
      request.headers["X-Authorization"] = authToken;
    }
    return request
  },
  error => {
    return Promise.reject(error);
  }
);

export default axiosInstance;

axiosInstance.interceptors.response.use(
  response => {
    return response;
  },
  async error => {
    updateAuthed();
    return Promise.reject(error);
  }
)
