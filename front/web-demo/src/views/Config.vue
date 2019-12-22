<template>
  
  <v-container fluid>
    <v-row align="center" justify="center"  > 
      <v-col cols="10">
        
        <v-card class="overflow-hidden" > 

          <v-toolbar flat color="grey darken-1" dense>
            <v-toolbar-title class="font-weight-light" color="white">Device Profile</v-toolbar-title>
          </v-toolbar>

          <v-card-text>
            <v-text-field v-model="device_name" :rules="[rules.required]" :counter="40" label="Device Name" required dense></v-text-field>
            <br>
           

            <v-combobox v-model="wifi_name" :items="aps" :loading="loading_aps" label="Wifi access point" no-data="Press Scan to populate..." dense ></v-combobox>
            <br>
            
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

            <br>
            <v-divider></v-divider>

            <v-switch v-model="mqtt_enable" label="Enable MQTT"></v-switch>
            <br>
            <v-text-field v-model="mqtt_server" :disabled="!mqtt_enable" :counter="200" label="MQTT Server" required dense></v-text-field>
            <br>
            <v-text-field v-model="mqtt_topic" :disabled="!mqtt_enable" :counter="200" label="MQTT Base Topic" required dense></v-text-field>
            <br>
            <v-text-field v-model="mqtt_time" :disabled="!mqtt_enable" v-mask="'#####'" :rules="[rules.time]" suffix="seconds" :counter="5" label="Send MQTT post every ... seconds" required dense></v-text-field>
            <br>

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
        mqtt_enable: false,
        mqtt_server: '',
        mqtt_topic: '',
        mqtt_time: '',
        errtext: '',
        showerr: false,
        loading_aps: false,
        showpwd: false,
        iconEyeOn: mdiEye,
        iconEyeOff: mdiEyeOff,

        rules: 
        {
          required: value => !!value || 'Required.',
          port: value => (value>0 && value <= 65535) || 'Not a valid port.',
          time: value => (value>=5) || 'At least 5 seconds.',
          email: value => {
            const pattern = /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
            return pattern.test(value) || 'Invalid e-mail.'
          }
        },
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

            //console.log("--- scanips api call received...");

            this.aps          = data.data.WiFI_Scan;
            this.loading_aps  = false;

            this.errtext      = "Scan successfull...click into Wifi access point field to see results";
            this.showerr      = true;

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
            Device_Name: this.device_name,
            mqtt_enable: this.mqtt_enable ? 1 : 0,
            mqtt_server: this.mqtt_server,
            mqtt_topic: this.mqtt_topic,
            mqtt_time: parseInt(this.mqtt_time, 10),
        },{timeout: 10000}
        )
        .then(data => {
          this.loading_aps  = false;

          this.errtext      = "Configuration saved successfully";
          this.showerr      = true;
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

            this.device_name  = data.data.Device_Name;
            this.wifi_name    = data.data.Wifi_SSID;
            this.mqtt_server  = data.data.mqtt_server;
            this.mqtt_topic   = data.data.mqtt_topic;
            this.mqtt_time    = data.data.mqtt_time;
            this.mqtt_enable  = data.data.mqtt_enable == 1 ? true : false;

          })
            
   },

};
</script>