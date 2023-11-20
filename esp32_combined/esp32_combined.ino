#include "Arduino.h"
#include <WiFi.h>
#include <FastLED.h>
#include <string.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQTTClient.h>
#include "secrets.h"     // AWS credentials and certificates
#include "OLEDImages.h"  // OLED expressions

FASTLED_USING_NAMESPACE

// LED Parameters
#define DATA_PIN 5
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 24
CRGB leds[NUM_LEDS];

// Photoresistor Parameters
#define PHOTO_PIN 33
#define STANDARD_BRIGHTNESS 96
#define STANDARD_PHOTO_READING 600  //measured in a standard environment where brightness=96
uint8_t PhotoPinReading;
uint8_t BRIGHTNESS = 96;  // brightness indicator for keep-on modes, to be adjusted in order to get PhotoPinReading close to STANDARD_PHOTO_READING
uint8_t brightness = 0;   //brightness indicator for breathing effect
int flag_up = 1;

float max(float x, float y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}

// OLED Parameters
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
int oled_state = 0;
int interval = 200;
//{0: default, blinking; 1: sitting, smile; 2: crouching/playphone, angry; 3: sleeping, sleep}
int loop_flag = 0;  // set to 0 when each transit occurs

void draw(const unsigned char *experession) {
  display.drawBitmap(0, 0, experession, 64, 48, 1);
  display.display();
  display.clearDisplay();
}

// Flag for mode selection
int activity = 0;
int posture = 0;
// ML algorithm would classify activity as:
// {1: computer, 2: reading, 3: sleeping, 4: not present}
// ML algorithm would classify posture as:
// {1: good, 2: bad}
int count = 0;

// Wifi and MQTT Parameters
WiFiClientSecure net = WiFiClientSecure();
MQTTClient mqttClient = MQTTClient(20000);

char *subscribeTopic1 = "laptop/activity";
char *subscribeTopic2 = "laptop/posture";
char *publishTopic = "hello/esp";

String msg;
int receivedActivity = 0;
int receivedPosture = 0;
int sendReady = 0;

// RTOS parameters
// this triggers the reconnection to WiFi after a specific period of time (via interrupt)
hw_timer_t *My_timer = NULL;
int initiateConnect = 0;
int connected = 0;

void IRAM_ATTR onTimer() {
  initiateConnect = 1;
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting Wifi...");
  }
  Serial.println("Connected to WiFi!");
}

void connectToAWS() {
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  Serial.println("Connect begin");
  mqttClient.begin(AWS_IOT_ENDPOINT, 8883, net);
  // create a call back to receive messages
  mqttClient.onMessage(messageHandler);
  Serial.println("Connecting to AWS IoT");
  while (!mqttClient.connect("ESP32_test")) {
    Serial.print("Connecting AWS...");
    vTaskDelay(100);
  }
  // Subscribe to a topic
  mqttClient.subscribe(subscribeTopic1);
  mqttClient.subscribe(subscribeTopic2);
  Serial.println("AWS IoT Connected!");
}

void messageHandler(String &topic, String &payload) {
  Serial.println("message received");

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* message = doc["message"];

  if (!message) {
      Serial.println("message is null!");
  }

  if (topic == subscribeTopic1) {
    Serial.printf("From %s received message: %s\n", subscribeTopic1, payload.c_str());

    if (strcmp(message, "computer_use") == 0) { activity = 1; }
    if (strcmp(message, "reading") == 0) { activity = 2; }
    if (strcmp(message, "asleep") == 0) { activity = 3; brightness=0; flag_up=1;}
    if (strcmp(message, "not_present") == 0) { activity = 4; }

    Serial.println(activity);
    loop_flag = 0;
    receivedActivity = 1;
  }

    if (topic == subscribeTopic2) {
    Serial.printf("From %s received message: %s\n", subscribeTopic2, message);
    if (strcmp(message, "good_posture") == 0) { posture = 1; }
    if (strcmp(message, "bad_posture") == 0) { posture = 2; brightness=0; flag_up=1;}

    Serial.println(posture);
    loop_flag = 0;
    receivedPosture = 1;
  }
}

void handleLoop(void *parameter) {
  Serial.println("handleLoop running");
  for (;;) {
    if (!mqttClient.connected()) {
      Serial.println("MQTT disconnected. Attempting to reconnect...");
      //connectToAWS();  // Reconnect to AWS
    } else {
      Serial.println("MQTT connected.");
      mqttClient.loop();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void handleConnect(void *parameter) {
  Serial.println("handleConnect running");
  while (1) {
    while (WiFi.status() == WL_CONNECTED) {
      Serial.println("Start of delay");
      vTaskDelay(10000);
      Serial.println("End of delay");
      mqttClient.disconnect();
      Serial.println("mqttClient disconnected");
      WiFi.disconnect();
      Serial.println("Wifi disconnected");
    }

    if (initiateConnect == 1 && WiFi.status() != WL_CONNECTED) {
      initiateConnect = 0;
      Serial.println("If statement entered");
      connectToWiFi();
      Serial.println("Wifi connected");
      connectToAWS();
      Serial.println("mqttClient connected");
    }
    vTaskDelay(100);
  }
}

void handleLED(void *parameter) {
  Serial.println("handleLED running");
  while (1) {
    BRIGHTNESS = max(96 + 1 * (600 - analogRead(PHOTO_PIN)), 0.0);
    //Not present
    if (activity == 4) {
      // Turn off all the LEDS.
      FastLED.setBrightness(0);
      FastLED.show();
      // Turn off the OLED.
      display.clearDisplay();
      display.display();

    } else if (activity == 3) {
      //LED breathing effect
      for (int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB::White; }
      if (count % 100 == 0) {
        if (flag_up == 1 && brightness < 10) {
          brightness++;
          if (brightness == 10) { flag_up = 0; }
        }
        if (flag_up == 0 && brightness > 0) {
          brightness--;
          if (brightness == 0) { flag_up = 1; }
        }
        // ledcWrite(LEDCHANNEL, brightness);
        FastLED.setBrightness(brightness);
        FastLED.show();
      }
      //OLED sleeping
      if (loop_flag == 0) {
        if (count == 0) { draw(start); }
        if (count == interval) { draw(sleep1); }
        loop_flag = 1;
      } else {
        if (count == 0) { draw(sleep2); }
        if (count == interval) { draw(sleep3); }
        if (count == interval * 2) { draw(sleep4); }
        if (count == interval * 3 || count == interval * 4) { draw(sleep5); }
      }
    } else if (posture == 2) {
      //LED warning effect
      for (int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB::Red; }
      if (flag_up == 1 && brightness < 254) {
        brightness++;
        if (brightness == 254) { flag_up = 0; }
      }
      if (flag_up == 0 && brightness > 0) {
        brightness--;
        if (brightness == 0) { flag_up = 1; }
      }
      FastLED.setBrightness(brightness);
      FastLED.show();
      //OLED angry
      if (loop_flag == 0) {
        if (count == 0) { draw(start); }
        if (count == interval) { draw(angry1); }
        if (count == interval * 2) { draw(angry2); }
        loop_flag = 1;
      } else {
        if (count == 0 || count == interval * 2) { draw(angry3); }
        if (count == interval || count == interval * 3 || count == interval * 4) { draw(angry4); }
      }
    } else if (activity == 0 || activity == 1 || activity == 2) {
      //OLED Blinking
      if (count == 0 || count == interval * 3 || count == interval * 4) { draw(start); }
      if (count == interval) { draw(blink1); }
      if (count == interval * 2) { draw(blink2); }
      //LED brightness affected by photoresistor
      if (activity == 2) {
        //Warm light for reading
        for (int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB::Yellow; }
        FastLED.setBrightness(BRIGHTNESS);
        FastLED.show();
      } else {
        //White light for startup and computer use
        for (int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB::White; }
        FastLED.setBrightness(BRIGHTNESS);
        FastLED.show();
      }
    }
    count = (count + 1) % 1000;
    vTaskDelay(1);
  }
}


void setup() {
  Serial.begin(115200);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // initialize OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000 * 20, true);
  timerAlarmEnable(My_timer);  //Just Enable

  connectToWiFi();
  connectToAWS();

  xTaskCreatePinnedToCore(handleLoop, "LoopTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(handleLED, "HandleLED", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(handleConnect, "HandleConnect", 10000, NULL, 1, NULL, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
}
