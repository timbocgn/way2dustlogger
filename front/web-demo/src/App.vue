<template>
  <v-app id="inspire">
    
    <v-app-bar
      color="grey accent-4"
      app
      dark
      short
      clipped-left
    >
      <v-app-bar-nav-icon @click.stop="drawer = !drawer"><v-icon>{{ iconMenu }}</v-icon></v-app-bar-nav-icon>
      <v-toolbar-title>way2.net Thermometer</v-toolbar-title>
    </v-app-bar>


    <v-navigation-drawer v-model="drawer" app  clipped>
      <v-list
        dense
        nav
      >
        <v-list-item
          v-for="item in items"
          :key="item.title"
          link
          :to="item.to"
        >
          <v-list-item-icon>
            <v-icon>{{ item.icon }}</v-icon>
          </v-list-item-icon>

          <v-list-item-content>
            <v-list-item-title>{{ item.title }}</v-list-item-title>
          </v-list-item-content>

        </v-list-item>
      </v-list>
    </v-navigation-drawer>
   
 
    <v-content>
      <v-container fluid fill-height>
        <router-view></router-view>
      </v-container>
    </v-content>
   
    <v-footer color="grey accent-4" app fixed>
      <span class="white--text"> &copy; 2019 way2.net Services</span>
    </v-footer>
  </v-app>
</template>

<script>

import { mdiMenu } from '@mdi/js'
import { mdiHomeCircle } from '@mdi/js'
import { mdiSettings } from '@mdi/js'
import { mdiHelpCircle } from '@mdi/js'

export default {
  name: 'App',
  data () {
    return {
      item: null,
      items: [
          { title: 'Home', icon: mdiHomeCircle, to: '/' },
          { title: 'Config', icon: mdiSettings, to:'/config' },
          { title: 'About', icon: mdiHelpCircle, to:'about' },
        ],
      drawer: null,
      iconMenu: mdiMenu,
    }
  }
}
</script>
