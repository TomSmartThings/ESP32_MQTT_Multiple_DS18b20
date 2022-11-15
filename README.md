# ESP32_MQTT_Multiple_DS18b20
This is a labour of love (and plenty of hate) that took me far too many hours.  The code 
uses MQTT to send multiple DS18B20 temperature sensor readings from an ESP32 to an MQTT broker running on Home Assistant.

Somehow I couldn't find a simple example online that would help me do this, so I've done it myself by merging many different examples together, and along with 
a huge amount of trial and error I've finally made it work. Hopefully it'll help someone else and save them some time. 

You'll need to plug your ESP32 into your computer and upload this code to it using ArduinoIDE
