#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include "Arduino.h"
#include <base64.h>

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient mqttClient = MQTTClient(256);

void connectESPtoAWS() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }  

    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);  

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    mqttClient.begin(AWS_IOT_ENDPOINT, 8883, net);

    // create a call back to receive messages
    mqttClient.onMessage(messageHandler);

    Serial.print("Connecting to AWS IoT");

    while (!mqttClient.connect("ESP32_test")) {
        Serial.print(".");
        delay(100);
    }  

    if (!mqttClient.connected()) {
        Serial.println("AWS IoT Timeout!");
        return;
    }

    // Subscribe to a topic
    mqttClient.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

    Serial.println("AWS IoT Connected!");
}

String encodeToBase64(String input) {
    String encoded = base64::encode(input);
    return encoded;
}

void publishMessage() {
    String message = "This is a test message!";
    String encodedMessage = encodeToBase64(message);
    String jsonPayload = "{\"message\": \"" + encodedMessage + "\"}";
    mqttClient.publish(AWS_IOT_PUBLISH_TOPIC, jsonPayload);
    Serial.println(encodedMessage);
    Serial.println("Message sent!");
}

void messageHandler(String &topic, String &payload) {
    Serial.println("Incoming: " + topic + " - " + payload);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    connectESPtoAWS();
    publishMessage();
}

void loop() {
    // put your main code here, to run repeatedly:
    
    mqttClient.loop();
    //delay(10000);
}
