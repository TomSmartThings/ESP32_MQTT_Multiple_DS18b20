/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mqtt-publish-ds18b20-temperature-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define WIFI_SSID "IOT-LAN2G"
#define WIFI_PASSWORD "StewartIOTLAN2015"

// Raspberry Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 30, 46)
#define MQTT_PORT 1883

DeviceAddress sensor1 = { 0x28, 0x18, 0x7D, 0x1, 0xA, 0x0, 0x0, 0x80 };
DeviceAddress sensor2 = { 0x28, 0x84, 0xBA, 0x0, 0xA, 0x0, 0x0, 0xB0 };
DeviceAddress sensor3= { 0x28, 0x91, 0x68, 0x7, 0xD6, 0x1, 0x3C, 0x20 };
DeviceAddress sensor4= { 0x28, 0xC7, 0x92, 0x7, 0xD6, 0x1, 0x3C, 0xA7 };

// Temperature MQTT Topic
#define MQTT_PUB_TEMP_1 "esp/ds18b20/temperature/sensor1"
#define MQTT_PUB_TEMP_2 "esp/ds18b20/temperature/sensor2"
#define MQTT_PUB_TEMP_3 "esp/ds18b20/temperature/sensor3"
#define MQTT_PUB_TEMP_4 "esp/ds18b20/temperature/sensor4"


// GPIO where the DS18B20 is connected to
const int oneWireBus = 33;          
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
// Temperature value
float temp;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

/*void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}*/

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  // Start the DS18B20 sensor
  sensors.begin();
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials("MQTT", "MQTTBROKER");
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New temperature readings
    sensors.requestTemperatures(); 
    // Temperature in Celsius degrees
    temp = sensors.getTempCByIndex(0);
       
    // Publish an MQTT message on topic esp32/ds18b20/temperature_1
    float temp1 = sensors.getTempC(sensor1);
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP_1, 1, true, String(temp1).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP_1);
    Serial.println(packetIdPub1);
    Serial.printf("Message: %.2f /n", sensors.getTempC(sensor1));

    // Publish an MQTT message on topic esp32/ds18b20/temperature_2
    float temp2 = sensors.getTempC(sensor2);
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_TEMP_2, 1, true, String(temp2).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP_2);
    Serial.println(packetIdPub2);
    Serial.printf("Message: %.2f /n", sensors.getTempC(sensor2));

    // Publish an MQTT message on topic esp32/ds18b20/temperature_3
    float temp3 = sensors.getTempC(sensor3);
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_TEMP_3, 1, true, String(temp3).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP_3);
    Serial.println(packetIdPub3);
    Serial.printf("Message: %.2f /n", sensors.getTempC(sensor3));

    // Publish an MQTT message on topic esp32/ds18b20/temperature_4
    float temp4 = sensors.getTempC(sensor4);
    uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_TEMP_4, 1, true, String(temp4).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP_4);
    Serial.println(packetIdPub4);
    Serial.printf("Message: %.2f /n", sensors.getTempC(sensor4));

  }
}
