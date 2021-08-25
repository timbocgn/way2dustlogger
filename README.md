# ESPDustLogger

ESP32 based IoT Device for air quality logging featuring an MQTT client and REST API acess. Works in conjunction with a [VINDRIKTNING](https://www.ikea.com/de/de/p/vindriktning-luftqualitaetssensor-70498242/) air sensor from IKEA.
    
## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. 

### Prerequisites

You will need a running copy of the ESP IDF SDK. Please follow these instructions:

https://docs.espressif.com/projects/esp-idf/en/latest/get-started/

Be sure that you have the `idf.py` application in your path and that all environment variables are setup, e.g. by providing a `.zshrc` file (example for macOS Catalina):

```
#ESP32 stuff

export IDF_PATH=<your IDF path>/esp-idf

source $IDF_PATH/export.sh
```

### Installing and Compiling

Download or clone the repository to your system

```
https://github.com/timbocgn/way2dustlogger.git
```

Firstly you should install all the dependencies for the vue based web application

```
cd front/webapp
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

Under "ESP Dust Logger Configuration" you will be able to define the number of sensors connected and the respective GPIO pins. Please see the wiring instructions for the bootstrap switch and the info LED.

When configured, compile and flash to your device:

```
idf.py build
...
idf.py -p <your serial device> flash
```

My WEMOS mini board (see below) is equipped with a CP2104 USB/UART converter and in my case it defaults to /dev/tty.usbserial-00E3A8A2. 

Please use 
```
ls /dev/tty.*
```
to find the device name of your programming port.

### Monitor your device

A lot of debugging output will be generated which you can see if you monitor the ESP:

```
idf.py -p /dev/tty.usbserial-00E3A8A2 monitor
```

## How to use

### Bootstrap

* Close the bootstrap switch for more than 10 seconds. The system reboots...
* The LED will blink in a 500ms on - 2500ms off sequence, which indicated that the build in access point is up and running
* Connect your system to this AP's IP-Address - the password is "let-me-in-1234"
* Go to the configuration page, provide your WLAN access point SSID (press 'Scan' to get a list) and provide the password
* Reboot the system (power off and on)
* When the LED blinks in a 100ms on - 100ms off - 100ms on - 2700ms off fashion, the system is connecting to your AP
* When the LED blinks in a 100ms on - 2900ms off fashion, the system is connected to your AP

### Access the web interface

Please take a look at your router, DHCP server or the monitor output to get the IP address of the ESP32. Point your browser to the IP address to check if the sensors are working. 

It might take a while until the Vindriktning sensor reveives its first measurement.

### Access the sensor data 

The sensor provides a REST-API for the device. See the postman examples in `Dust Logger.postman_collection.json`.

* `pm1` is the number of 1um particles per m^3
* `pm2` is the number of 2.5um particles per m^3
* `pm10` is the number of 10um particles per m^3

### Push the sensor data to MQTT

Just provide the necessary data in the MQTT section and enable the MQTT client. The sensor will provide the data as JSON struct:

```
{
  "pm1" : 24,
  "pm2" : 55,
  "pm10" : 14
}
```

## Development

### Changing the UI

To change the UI it is very handy to you the build in web server of webpack:

```
npm run serve
```

It will start a local web server on your machine which forwards the API calls to your device. Please check the `vue.config.js` file in `front/webapp` and enter the IP adress the requests should be forwarded to (to the address of your ESP).

Please follow the vue.js guides and how to's on how to change the front end code.

## Wiring

I used a ESP32 MINI board, sometimes called WEMOS ESP32 mini board although it is not a WEMOS board. I bought mine here: https://www.komputer.de/zen/index.php?main_page=product_info&products_id=530 . They are wideley available, just google for it. GPIO2 is directly connected to a SMD led on this board, so this connection has already been been made.

The default configuration is:

```
SENSOR1_UART_PORT_NUM   = 1 
SENSOR1_DATA_GPIO       = 25
BOOTSTRAP_GPIO          = 35
INFOLED_GPIO            = 2
```

The wiring is quite simple as you can see in the wiring.png file in the repository.


* the information LED is internally (!) connected to a GPIO which can drive the 2mA current. 
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
