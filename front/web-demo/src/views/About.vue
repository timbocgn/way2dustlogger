<template>
  
  <v-container fluid>
    <v-row align="center" justify="center"  > 
      <v-col cols="10">
        
        <v-card class="overflow-hidden" > 

          <v-toolbar flat color="grey darken-1" dense>
            <v-toolbar-title class="font-weight-light" color="white">About</v-toolbar-title>
          </v-toolbar>

          <v-card-text>
            <p>Welcome to the ESP32 based temperature logging device from way2.net services.</p>

            <p>It gives you a SHT15 based sensor configuration for any 
            number of sensors, a nice web interface for bootstrapping and configuring the device, as well as an MQTT client for pushing the sensor
            data to any MQTT broker.</p>

            <p>You also can leverage the API calls to configure the device and get the sensor data to your favorite application.</p>

            <v-divider></v-divider>
            <br>

            <p> This appliction is provided as public domain - feel free to use in any kind of project. </p>

            <v-divider></v-divider>

            <br>
            <p> Project homepage: <a href="http://www.way2.net/esplogger">way2.net Homepage</a> </p>

          </v-card-text>

        </v-card>

      </v-col>
    </v-row>
  
  </v-container>
  
</template>



