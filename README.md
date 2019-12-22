# ESPTempLogger

The ESPTempLogger is a ESP32 IDF based IoT device. It has an interface for multiple SHT1x sensors (configurable on compile time), 
some REST-APIs for getting the current data and a web interface for configuration.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. 

### Prerequisites

You will need a running copy of the ESP IDF SDK. Please follow these instructions:

https://docs.espressif.com/projects/esp-idf/en/latest/get-started/

Be sure that you have the `idf.py` application in your path and that all environment variables are setup, e.g. by providing a `.zshrc` file (example for macOS Catalina):

```
# append
path+=('/Library/Frameworks/Python.framework/Versions/3.6/bin')
# export to sub-processes (make it inherited by child processes)
export PATH

#ESP32 stuff

export IDF_PATH=<your IDF path>/esp-idf

source $IDF_PATH/export.sh
```

### Installing and Compiling

Download or clone the repository to your system

```
https://github.com/timbocgn/way2templogger.git
```

Firstly you should install all the dependencies for the vue based web application

```
cd front/web-demo
npm install
```

When this is done, you can build the vue app

```
npm run build
```

This will "compile" the app into the build directory where the ESP toolchain will pick it up to store it onto the ESP32 fat flash filesystem, where it is then served by a http server.

When the build is done, you can configure your IDF app

```
cd ../..
idf.py menuconfig
```

Under "ESP Temp Logger Configuration" you will be able to define the number of sensors connected and the respective GPIO pins. Please see the wiring instructions for the bootstrap switch and the info LED.

When configured, compile and flash to your device:

```
idf.py build
...
idf.py -p <your serial device> flash
```

If you are using a ESP DevKitC (https://docs.espressif.com/projects/esp-idf/en/latest/hw-reference/get-started-devkitc.html) using the
CP2102 USB/Serial chip and its standard driver (https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers), your serial device will be `/dev/cu.SLAB_USBtoUART`.

### Monitor your device

A lot of debugging output will be generated which you can see if you monitor the ESP:

```
idf.py -p /dev/cu.SLAB_USBtoUART monitor
```

## How to use

### Bootstrap

* Close the bootstrap switch for more than 10 seconds. The system reboots...
* The LED will blink in a 500ms on - 2500ms off sequence, which indicated that the build in access point is up and running
* Connect your system to this AP's IP-Address plus 1 - the password is "let-me-in-1234"
* Go to the configuration page, provide your WLAN access point SSID (press 'Scan' to get a list) and provide the password
* Reboot the system
* When the LED blinks in a 100ms on - 100ms off - 100ms on - 2700ms off fashion, the system is connecting to your AP
* When the LED blinks in a 100ms on - 2900ms off fashion, the system is connected to your AP

### Access the web interface

Please take a look at your router, DHCP server or the monitor output to get the IP address of the ESP32. Point your browser to the IP address
to check if the sensors are working.

### Access the sensor data 

The sensor provides a REST-API for the device. See the postman examples in ESP.postman_collection.json.

* `temp` is the temperature in degree celsius
* `rh` is the relative humidity
* `dp` is the dew point in degree celsius

### Push the sensor data to MQTT

Just provide the necessary data in the MQTT section and enable the MQTT client. The sensor will provide the data as JSON struct:

```
{
  "temp" : 24.039997100830078,
  "rh" : 55.46722412109375,
  "dp" : 14.568182945251465
}
```

## Development

### Changing the UI

To change the UI it is very handy to you the build in web server of webpack:

```
npm run serve
```

It will start a local web server on your machine which forwards the API calls to your device. Plese check the `vue.config.js` file in `front/web-demo` and enter the IP adress the requests should be forwarded to (to the address of your ESP).

Please follow the vue.js guides and how to's on how to change the front end code.

## Wiring

The default configuration is:

```
SENSOR1_SCLK_GPIO   = 26
SENSOR1_DATA_GPIO   = 25
SENSOR2_SCLK_GPIO   = 17
SENSOR2_DATA_GPIO   = 16
BOOTSTRAP_GPIO      = 35
INFOLED_GPIO        = 2
```

The wiring is quite simple:

```

                                                 Sensor 1                  Sensor 2
      +---------------------------+            +-----------+             +-----------+
      |                           |            |           |             |           |
      |       ESP32 DevKitC       |   +--------+ SCLK      |        +----+ SCLK      |
      |                           |   |    +---+ DATA      |        | +--+ DATA      |
      |                           |   |    |   |           |        | |  |           |
      |    SENSOR1_SCLK_GPIO(26)  +---+    |   |           |        | |  |           |
      |    SENSOR1_DATA_GPIO(25)  +--------+   +-----------+        | |  +-----------+
      |                           |                                 | |
      |    SENSOR2_SCLK_GPIO(17)  +---------------------------------+ |
      |    SENSOR2_DATA_GPIO(16)  +-----------------------------------+
      |                           |
      |    BOOTSTRAP_GPIO(35)     +----------------------------------+
      |                           |                                  |
   +--+ INFOLED_GPIO(2)           |                                  |
   |  |                           |                                  |
   |  |                 VCC 3.3V  +--------------------------+       |
   |  |                 GND       +-----+                    |       |
   |  +---------------------------+     |                   +-+      |
   |                                    |                   | |      |
   |                                    |             2k    | |      |
   |                                    |                   +-+      |
 +++++                                  |                    |       |
  +++                                   |                    *-------+
   +  LED                               |                    ++
  ---                                   |                     +
   |                                    |                      +    Push Button
  +-+                                   |                    *  +
  | |  2k                               |                    |
  | |                                   |                    |
  +-+                                   |                    |
   |                                    |                    |
   +------------------------------------*--------------------+

```

* the information LED is directly connected to a GPIO which can drive the 2mA current. 
* the bootstrap button is pulled up using the 2k resistor. A input only GPIO can be used here.

## Built With

* [ESP IDF](https://www.espressif.com/en/products/software/esp-sdk/overview) - The SDK used for developent
* [vue.js](https://vuejs.org) - Javascript frontend framework used for the SPA
* [node.js](https://nodejs.org/en/) - Used to compile the vue app
* [webpack](https://webpack.js.org) - Module bundler for the vue app

## Authors

* **Tim Hagemann** - *Initial work* - https://github.com/timbocgn

## License

This project is licensed under the MIT License.

## Acknowledgments

* The ESP IDF authors for the perfect source code examples they provide within their SDK
* John Burns (www.john.geek.nz) for parts of the SHT1x code
* Daesung Kim for the initial work on the SHT1x code base
