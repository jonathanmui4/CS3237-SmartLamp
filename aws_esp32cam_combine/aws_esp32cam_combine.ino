#include "secrets.h" // AWS credentials and certificates
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <base64.h>


// ESP32 AM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// TimerWakeUp setup code
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 20          /* Time ESP32 will go to sleep (in seconds) */

// MQTT topics here (this should not be complete)
#define ESP32CAM_PUBLISH_TOPIC   "esp32/pub"
#define ESP32CAM_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient mqttClient = MQTTClient(20000);

// Task handle for MQTT loop task
TaskHandle_t mqttTaskHandle = NULL;

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }  
}

String takePicture() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
  }

  Serial.print("Captured image size: ");
  Serial.println(fb->len);


  String encoded = base64::encode(fb->buf, fb->len);
  Serial.print("Encoded image size: ");
  Serial.println(encoded.length());
  // Serial.println(encoded);
  Serial.println("Photo taken");
  return encoded;
}

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    } 
    Serial.println("Connected to WiFi!"); 
}

void connectToAWS() {
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

    // Subscribe to a topic
    mqttClient.subscribe(ESP32CAM_SUBSCRIBE_TOPIC);

    Serial.println("AWS IoT Connected!");
}

void goToSleep() {
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();  // initiate deep sleep
}

String encodeToBase64(String input) {
  String encoded = base64::encode(input);
  return encoded;
} 

void messageHandler(String &topic, String &payload) {
  Serial.println("Incoming: " + topic + " - " + payload);
}

void publishPhotoToAWS(String b64Photo) {
  // this should take in a topic to publish the message under
//   String msg = "this is a test";
//   String message = encodeToBase64(msg);
//   String jsonPayload = "{\"message\": \"" + message + "\"}";
  String jsonPayload = "{\"message\": \"" + b64Photo + "\"}";
  mqttClient.publish(ESP32CAM_PUBLISH_TOPIC, jsonPayload);
  Serial.println(b64Photo);
  if (mqttClient.publish(ESP32CAM_PUBLISH_TOPIC, jsonPayload)) {
    Serial.println("Message sent!");
  } else {
    Serial.println("Failed to send message");
    // Handle reconnection or retries here
  }
  
}

void mqttLoopTask(void* pvParameters) {
    Serial.print("task running on core ");
    Serial.println(xPortGetCoreID());
    for (;;) {
        if (!mqttClient.connected()) {
            Serial.println("MQTT disconnected. Attempting to reconnect...");
            connectToAWS();  // Reconnect to AWS
        } else {
            //Serial.println("MQTT connected.");
            mqttClient.loop();
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to free up CPU cycles
    }
}


void setup() {
  Serial.begin(115200);
  delay(1000); // do we need this delay?
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  initCamera();
  connectToWiFi();
  connectToAWS();

  xTaskCreatePinnedToCore(mqttLoopTask, "MQTTTask", 10000, NULL, 1, &mqttTaskHandle, 1);

  String photoBytes = takePicture();

  //delay(5000);

  publishPhotoToAWS(photoBytes);

  // kill task 
  if (mqttTaskHandle != NULL) {
      vTaskDelete(mqttTaskHandle);
      mqttTaskHandle = NULL;
      Serial.println("MQTT Task killed.");
  }

  // deep sleep
  esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:
}
