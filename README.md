# WebConfigServer
[![Release Build](https://github.com/paclema/WebConfigServer/actions/workflows/release.yml/badge.svg)](https://github.com/paclema/WebConfigServer/actions/workflows/release.yml)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/paclema/library/WebConfigServer.svg?version=3.1.0)](https://registry.platformio.org/libraries/paclema/WebConfigServer)
[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg?color=yellow)](https://www.paypal.com/donate/?business=8PXZ598XDGAS2&no_recurring=0&currency_code=EUR&source=url)

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
* Enabled lwip NAT features for ESP32 (Not available for ESP8266 yet, probably never due memory limitations. Check this branch: [iot_button napt_esp8266](https://github.com/paclema/iot_button/tree/napt_esp8266)).
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

## License
<p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><a property="dct:title" rel="cc:attributionURL" href="https://github.com/paclema/WebConfigServer">WebConfigServer</a> © 2019 by <a rel="cc:attributionURL dct:creator" property="cc:attributionName" href="http://www.paclema.com/">paclema</a> is licensed under <a href="http://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">CC BY-NC-SA 4.0<!-- <img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1">--></a></p> 

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg
