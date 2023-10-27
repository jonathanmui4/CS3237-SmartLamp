
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <FastLED.h>
#include <string.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

FASTLED_USING_NAMESPACE


#define DATA_PIN    13
#define PHOTO_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];


#define STANDARD_BRIGHTNESS 96
#define STANDARD_PHOTO_READING 600 //measured in a standard environment where brightness=96
uint8_t BRIGHTNESS = 96; // to be adjusted in order to get PhotoPinReading close to STANDARD_PHOTO_READING
uint8_t PhotoPinReading = 0;

uint8_t brightness = 0; //brightness indicator for breathing effect
int flag_up = 1;
int posture=0;
// ML algorithm would classify posture as:
// {0: default, 1:'sitting', 2:'crouching', 3:'sleeping', 4:'playphone', 5:'empty'}

const char *ssid = "haihong";
const char *pass = "yuhaihong";
char *server = "mqtt://172.20.10.2";

char *subscribeTopic = "hello/laptop";
char *publishTopic = "hello/esp";


ESP32MQTTClient mqttClient; // all params are set later

// Initialize Telegram BOT
#define BOTtoken "6430964381:AAGG6iPYHaT3j_TpbavN9vLWBqhG9PN7pV8"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "1169036837"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void setup() {
  Serial.begin(115200);
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    WiFi.mode(WIFI_STA);
      // Connect esp32 to WiFi
    WiFi.begin(ssid, pass);
    WiFi.setHostname("c3test");
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
  
      // Connecting esp32 to MQTT broker
    log_i();
    log_i("setup, ESP.getSdkVersion(): ");
    log_i("%s", ESP.getSdkVersion());
    mqttClient.enableDebuggingMessages();
    mqttClient.setURI(server);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
    mqttClient.loopStart();
    Serial.print("Connecting to MQTT broker");
    while (!mqttClient.isConnected()) {
        Serial.print('.');
        delay(1000);
    }
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    bot.sendMessage(CHAT_ID, "Bot started up", "");

}

int count=0;
void loop()
{ 
  PhotoPinReading = analogRead(PHOTO_PIN);
  BRIGHTNESS = STANDARD_BRIGHTNESS + (STANDARD_BRIGHTNESS / STANDARD_PHOTO_READING) * (STANDARD_PHOTO_READING - PhotoPinReading);
  if (posture==0 || posture==1){
    for( int i = 0; i < NUM_LEDS; i++) {leds[i] = CRGB::White;}
      FastLED.setBrightness(BRIGHTNESS);
      FastLED.show();
  }
  else if(posture==2){
      for( int i = 0; i < NUM_LEDS; i++) {leds[i] = CRGB::Red;}
      FastLED.setBrightness(BRIGHTNESS);
      FastLED.show();
    }
    else if(posture==3){
      for( int i = 0; i < NUM_LEDS; i++) {leds[i] = CRGB::White;}
      if(count%100==0){
        if(flag_up==1 && brightness<10){
          brightness++;
          if(brightness==10){flag_up=0;}
        }
        if(flag_up==0 && brightness>0){
          brightness--;
          if(brightness==0){flag_up=1;}
        }
        FastLED.setBrightness(brightness);
        FastLED.show();
      }
    } 

  else if(posture==4){
    if(count%1000==0){
      bot.sendMessage(CHAT_ID, "Stop playing with your phone and get back to work!!!", "");
    }
      for( int i = 0; i < NUM_LEDS; i++) {leds[i] = CRGB::Red;}
        if(flag_up==1 && brightness<254){
          brightness++;
          if(brightness==254){flag_up=0;}
        }
        if(flag_up==0 && brightness>0){
          brightness--;
          if(brightness==0){flag_up=1;}
        }
        FastLED.setBrightness(brightness);
        FastLED.show();
    }
    else if(posture==5){
      FastLED.setBrightness(0);
      FastLED.show();
  }
  count++;
  delay(1);
}


void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(subscribeTopic, [](const String &payload)
                             {Serial.printf("From %s received message: %s\n", subscribeTopic, payload.c_str()); 
                             if(strcmp(payload.c_str(), "sitting")==0){posture=1;}
                             if(strcmp(payload.c_str(), "crouching")==0){posture=2;}
                             if(strcmp(payload.c_str(), "sleeping")==0){posture=3;}
                             if(strcmp(payload.c_str(), "playphone")==0){posture=4;}
                             if(strcmp(payload.c_str(), "empty")==0){posture=5;}
                             Serial.println(posture);
                             });
// {0: default, 1:'sitting', 2:'crouching', 3:'sleeping', 4:'playphone', 5:'empty'}

        mqttClient.subscribe("bar/#", [](const String &topic, const String &payload)
                             {Serial.printf("From %s received message: %s\n", subscribeTopic, payload.c_str()); 
                             Serial.println(strcmp(payload.c_str(), "sleeping"));
                             if(strcmp(payload.c_str(), "sitting")==0){posture=1;}
                             if(strcmp(payload.c_str(), "crouching")==0){posture=2;}
                             if(strcmp(payload.c_str(), "sleeping")==0){posture=3;}
                             if(strcmp(payload.c_str(), "playphone")==0){posture=4;}
                             if(strcmp(payload.c_str(), "empty")==0){posture=5;}
                             Serial.println(posture);
                             });

    }
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
    mqttClient.onEventCallback(event);
    return ESP_OK;
}

