
import Vue from 'vue'
import vuetify from '@/plugins/vuetify' // path to vuetify export

import App from './App.vue'
import router from './router'
import axios from 'axios'
import store from './store'

Vue.config.productionTip = false

/*
axios.interceptors.request.use(request => {
  console.log('Starting Request', request)
  return request
})

axios.interceptors.response.use(response => {
  console.log('Response:', response)
  return response
})*/

Vue.prototype.$ajax = axios

new Vue({
  vuetify,
  router,
  store,
  render: h => h(App)
}).$mount('#app')

