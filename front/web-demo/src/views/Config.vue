<template>
  
  <v-container fluid>
    <v-row align="center" justify="center"  > 
      <v-col cols="10">
        
        <v-card class="overflow-hidden" > 

          <v-toolbar flat color="grey darken-1" dense>
            <v-toolbar-title class="font-weight-light" color="white">Device Profile</v-toolbar-title>
          </v-toolbar>

          <v-card-text>
            <v-text-field v-model="device_name" :counter="40" label="Device Name" required dense></v-text-field>
            <v-spacer></v-spacer>
            <v-spacer></v-spacer>
            <v-spacer></v-spacer>
            <v-spacer></v-spacer>

            <v-combobox v-model="wifi_name" :items="aps" :loading="loading_aps" label="Wifi access point" no-data="Press Scan to populate..." dense ></v-combobox>
            
            <v-text-field
                  v-model="wifi_pass"
                  :append-icon="showpwd ? iconEyeOn : iconEyeOff"
                  :type="showpwd ? 'text' : 'password'"
                  name="input-10-1"
                  label="Wifi Password"
                  hint="At least 8 characters"
                  counter
                  @click:append="showpwd = !showpwd">
            </v-text-field>

            <v-divider></v-divider>

          </v-card-text>

          <v-card-actions>
            
            <v-spacer></v-spacer>
            <v-btn class="ma-2" color="primary"  :disabled="loading_aps"   @click="scan_aps">Scan</v-btn>
            <v-btn class="ma-2" color="primary"  :disabled="loading_aps" @click="set_config">Save</v-btn>
          </v-card-actions>

        </v-card>

      </v-col>
    </v-row>

    <v-snackbar v-model="showerr">{{ errtext }}</v-snackbar>
  
  </v-container>
  
</template>

<script>

import { mdiEye } from '@mdi/js'
import { mdiEyeOff } from '@mdi/js'

export default {
  data () {
      return { 
        aps: [], 
        device_name: '',
        wifi_name: '',
        wifi_pass: '',
        errtext: '',
        showerr: false,
        loading_aps: false,
        showpwd: false,
        iconEyeOn: mdiEye,
        iconEyeOff: mdiEyeOff,
      };
  },

  methods: 
  {

    scan_aps: function() 
    {

      this.loading_aps = true;
      this.$ajax
          .get("/api/v1/apscan", {timeout: 60000})
          .then(data => {

            console.log("--- scanips api call received...");

            this.aps          = data.data.WiFI_Scan;
            this.loading_aps  = false;
          })
          .catch(error => {
            this.loading_aps  = false;
            this.errtext      = "Error invoking scan";
            this.showerr      = true;

            console.log(error);
          });
            
    },


    set_config: function() 
    {
      this.loading_aps  = true;

      this.$ajax
        .post("/api/v1/config", {
	          Wifi_SSID: this.wifi_name,
	          Wifi_Password: this.wifi_pass,
	          Device_Name: this.device_name
        },{timeout: 10000}
        )
        .then(data => {
          console.log(data);
          this.loading_aps  = false;
        })
        .catch(error => {
          this.loading_aps  = false;
          this.errtext      = "Error saving configuration";
          this.showerr      = true;
          console.log(error);
        });
    }
  },

  mounted() 
    {
      this.$ajax
          .get("/api/v1/config")
          .then(data => {

            //console.log("--- config api call received...");

            this.device_name = data.data.Device_Name;
            this.wifi_name   = data.data.Wifi_SSID;
          })
            
   },

};
</script>

