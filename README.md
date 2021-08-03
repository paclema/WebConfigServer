# WebConfigServer

## Features:

Web application:
* Angular 10 and Bootstrap 4 web application to configure the device and visualize data.
* Nebular UI Angular lib.
* ng2-charts to visualize WS data graphs on dashboard web server endpoint.
* Gzipped compression to store and serve the Webapp.

Firmware:
* ESP8266/ESP32 support.
* Store configs as JSON and certs on SPI Flash File System (SPIFFS).
* AP+STA wifi mode with wifimulti and mDNS support
* FTP server to easily modify files stored on SPIFFS.
* WrapperOTA class to handle OTA updates.
* WrapperWebSockets class to handle WS communication between device and webserver dashboard endpoint.
* MQTT client as QoS2 to detect disconnection and enabled connection using user&pass and/or certificates.
* Added configurable DeepSleep modes for ESP8266.
* Webserver or AsyncWebserver support.
* Enabled lwip NAT features for ESP8266 and ESP32.
* Added configurable NTP server.


### Add a new configuration object:
1. Update the new object in the configuration file _/data/config.json_
2. Add the struct object in the configuration class _/lib/WebConfigServer/WebConfigServer.h_
3. Update the parse function to link the json object with the class struct in the configuration class _/lib/WebConfigServer/WebConfigServer.cpp_


### Upload _/data_ folder to ESP SPIFFS File System:

Using platformio run the next command: `pio run -t uploadfs`


### Build webserver to _/data_ fodler:
If you can not run angular-cli from platformio PowerShell using windows, activate it with:
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

Under _webserver/_ folder:
```console
ng build --prod --optimization=true --outputHashing=none; npm run postbuild
```

To build and upload SPIFFS
```console
cd ./webserver; ng build --prod --optimization=true --outputHashing=none; npm run postbuild;cd ..; pio run --target uploadfs --environment d1_mini
```

To build for xammp:
```console
ng build --prod --optimization=true --outputHashing=none --outputPath=C:/xampp/htdocs --deleteOutputPath=false
```

These build options can be added in angular.json in the future.

### Compress _/data_ fodler with gzip:
Under webserver folder:

```console
npm run postbuild
```
