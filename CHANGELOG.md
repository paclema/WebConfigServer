WebConfigServer: Changelog
==========================

HEAD
----

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
