// Load Wi-Fi library
#include <WiFi.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE


// Replace with your network credentials
const char* ssid     = "haihong";
const char* password = "yuhaihong";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

#define DATA_PIN    13
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

int greenValue = 0;
int redValue = 0;
int blueValue = 0;

String redValueString = String(redValue);
String greenValueString = String(greenValue);
String blueValueString = String(blueValue);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("GET /-r") >=0 ) redValue -=5;
            else if(header.indexOf("GET /+r") >=0) redValue +=5;
            else if(header.indexOf("GET /+g") >=0) greenValue +=5;
            else if(header.indexOf("GET /-g") >=0) greenValue -=5;
            else if(header.indexOf("GET /+b") >=0)  blueValue +=5;
            else if(header.indexOf("GET /-b") >=0) blueValue -=5;
            else if(header.indexOf("GET /Red") >=0) if(redValue == 0) redValue = 254; else redValue = 0;
            else if(header.indexOf("GET /Green") >=0) if(greenValue == 0) greenValue = 254; else greenValue = 0;
            else if(header.indexOf("GET /Blue") >=0) {if(blueValue == 0) blueValue = 254; else blueValue = 0;}

            for (int i=0; i<NUM_LEDS; i++) {
              leds[i].r = redValue; 
              leds[i].g = greenValue; 
              leds[i].b = blueValue;
            }
            FastLED.show();
            redValueString = String(redValue);  
            greenValueString = String(greenValue);
            blueValueString = String(blueValue);

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/css/bootstrap.min.css\">");
            
            client.println("</head>");
            
            // Web Page Heading
            
            client.println("<center>");
            client.println("<body><h1>ESP32 RGB Web Server</h1>"); 
            client.println("<br>");

            client.println("<a href = \" /+r \" ");
            if(redValue == 254)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">+</button>"); 
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">+</button>"); 
            client.println("</a>");

            client.println("<a href = \" /Red\" ");
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">RED(" + redValueString + ")</button>");  
            client.println("</a>");
            
            client.println("<a href = \" /-r \" ");
            if(redValue == 0)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">-</button>");  
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">-</button>");  
            client.println("</a>");

            client.println("<br>");
            client.println("<br>");

            client.println("<a href = \" /+g \" ");
            if(greenValue == 254)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">+</button>"); 
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">+</button>"); 
            client.println("</a>");
            
            client.println("<a href = \" /Green \" ");
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">GREEN(" + greenValueString + ")</button>");  
            client.println("</a>");

            client.println("<a href = \" /-g \" ");
            if(greenValue == 0)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">-</button>");  
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">-</button>");
            client.println("</a>");

            client.println("<br>");
            client.println("<br>");

            client.println("<a href = \" /+b \" ");
            if(blueValue == 254)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">+</button>");  
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">+</button>");  
            client.println("</a>");

            client.println("<a href = \" /Blue \" ");
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">BLUE(" + blueValueString + ")</button>");  
            client.println("</a>");

            client.println("<a href = \" /-b \" ");
            if(blueValue == 0)
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md disabled\">-</button>");  
            else
            client.println("<button type=\"button\" class=\"btn btn-primary btn-md\">-</button>");  
            client.println("</a>");

            client.println("<center>");
            client.println("</body></html>");

            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
