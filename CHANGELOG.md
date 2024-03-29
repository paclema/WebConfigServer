WebConfigServer: Changelog
==========================

v2.3.2 (2023-12-22)
------

* Fix GPIO state change when LED_BUILTIN is not defined or -1

v2.3.1 (2023-12-19)
------

* Trigger WiFi reconnect on IP_EVENT_STA_LOST_IP event

v2.3.0 (2023-06-02)
------

* Fix reconfigure NAPT after IP_EVENT_STA_GOT_IP for esp32
* Replace LinkedList with std::vector. Fix issue #8
* Replace ESPAsyncWebServer fork dependency
* Replace const JsonDocument with JsonVariantConst for config parse methods
* Check if config json contains IWebConfig object or service before parsing it
* Use ESPAsyncWebServer git repo as current version. Close issue #2
* Update example using IWebConfig class as pointer or reference

v2.2.2 (2023-05-19)
------

* Fix log typo
* Bump MQTTClient to v0.3.0

v2.2.1 (2023-05-16)
------

* Fix precompiler definition checks
* Fix compare_versions script checking the correct empty second argument
* Bump MQTTClient to v0.2.0

v2.2.0 (2023-05-10)
------

* Exclude .github folder for pio pkg
* Update basic_class_with_mqtt to basic_class_with_MQTTClient
* Update basic example and Bump MQTTClient to v0.1.1

v2.1.1 (2023-05-09)
------

* Fix save new mqtt client id into WebConfig when setMQTTClientId
* Bump MQTTClient to v0.0.8

v2.1.0 (2023-05-09)
------

* Add WebConfigMQTT class and replace MQTTClient with it.
* Fix getMQTTClient client pointer retrun and enable loop method only  for ESP8266 target
* Bump MQTTClient to v0.0.6
* Fix basic example partition table for esp32 and default mqtt josn config
* Bump MQTTClient to v0.0.7

v2.0.1 (2023-05-08)
------

* Fix ESP§" typo
* Clean up preprocesor defines
* Add default_4MB partition table for esp32 target examples
* Replace framework-arduinoespressif32 with latest custom framework and change WebConfigServer @ ^2.0.1 to min version on examples
* Bump MQTTClient to v0.0.4

v2.0.0 (2023-05-05)
------

* Added release github action workflow and necessary scripts
* Add Release build badge on Readme
* Add version to PlatformIO Registry badge
* Add PayPal donate badge
* Replace WebConfigMQTT with MQTTClient and remove PubSubClient dependencies
* Update basic example replacing PubSubClient with MQTTClient client and auto reconnect
* Fix basic example to use littlefs instead spiffs
* Create basic_class_with_mqtt example to show a class using WebConfig configuration and MQTTClient observer callbacks and client
* Include basic_class_with_mqtt example into PatformIO examples
* Include mqtt client setup for ESP32 target a WebConfigServer::begin method and enable mqtt loop only for ESP8266
* Fix nested preprocessing directives
* Add executable permissions to github Release action scripts
* Fix compare_versions.sh script to compare an initial empty library version
* Rename platformio example to Basic_Class_with_MQTTClient
* Bump MQTTClient to v0.0.2
