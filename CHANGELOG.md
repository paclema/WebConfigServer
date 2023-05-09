WebConfigServer: Changelog
==========================

HEAD
----

* Add WebConfigMQTT class and replace MQTTClient with it.
* Fix getMQTTClient client pointer retrun and enable loop method only  for ESP8266 target
* Bump MQTTClient to v0.0.6

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
