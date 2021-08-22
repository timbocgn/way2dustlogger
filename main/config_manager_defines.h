/*
    --------------------------------------------------------------------------------

    ESPDustLogger       
    
    ESP32 based IoT Device for air quality logging featuring an MQTT client and 
    REST API acess. Works in conjunction with a VINDRIKTNING air sensor from IKEA.
    
    --------------------------------------------------------------------------------

    Copyright (c) 2021 Tim Hagemann / way2.net Services

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    --------------------------------------------------------------------------------
*/

///////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_MANAGER_DEFINES_H_
#define	CONFIG_MANAGER_DEFINES_H_

////////////////////////////////////////////////////////////////////////////////////////

#define CFMGR_BOOTSTRAP_DONE    "Bootstrap_Done"
#define CFMGR_WIFI_SSID         "Wifi_SSID"
#define CFMGR_WIFI_PASSWORD     "Wifi_Password"
#define CFMGR_DEVICE_NAME       "Device_Name"
#define CFMGR_MQTT_SERVER       "mqtt_server"
#define CFMGR_MQTT_TOPIC        "mqtt_topic"
#define CFMGR_MQTT_TIME         "mqtt_time"
#define CFMGR_MQTT_ENABLE       "mqtt_enable"

////////////////////////////////////////////////////////////////////////////////////////

#endif

