#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>


//DEFINE WIFI THINGS
const char* ssid = "SSID Goes Here";
const char* password = "WiFi Password Goes Here";
WiFiClient client;
char server[] = "api.openweathermap.org";
String numRefresh = "";



//DEFINE SCREEN THINGS
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {

  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for a bit

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(2); //2 is sweet spot for one line to take up yellow color
  display.setTextColor(SSD1306_WHITE); // doesnt matter its monochrome
  display.setCursor(0, 0);
  display.cp437(true);

  //Start wifi module
  WiFi.begin(ssid, password);

  //Clear display, set cursor top left, print connecting... until connected
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Connecting");
  display.display();

  while(WiFi.status() != WL_CONNECTED) {
    
    delay(500);
    display.print(".");
    display.display();

  }

  //Clear display, set cursor top left, print connected and IP address
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connected!");
  display.print("IP Address");
  display.print(WiFi.localIP());
  display.display();

  //Wait a bit
  delay(2000);

}

void loop() {
  if(client.connect(server, 80)) {

  Serial.print("Connected to openweathermap");
  client.println("GET /data/2.5/weather?q=***YOURLOCATIONGOESHERE***&APPID=***YOURAPIKEYGOESHERE***&units=imperial HTTP/1.0");
  client.println("Host: api.openweathermap.org");
  client.println();
  
} else {
  Serial.println("Connection failed.");
}

//   // Skip HTTP headers
char endOfHeaders[] = "\r\n\r\n";
if (!client.find(endOfHeaders)) {
   Serial.println(F("Invalid response"));
   return;
 }

StaticJsonDocument<80> filter;
filter["weather"][0]["main"] = true;
filter["main"]["temp"] = true;

StaticJsonDocument<128> doc;

DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

const char* weather_0_main = doc["weather"][0]["main"]; // "Clear"

float main_temp = doc["main"]["temp"]; // 76.55

//cast as int to get rid of decimals
int temp = main_temp;

numRefresh += ".";

//Prepare display to show content
//Weather description is size 2 at the top in the yellow pixels
display.clearDisplay();
display.setCursor(0,0);
display.setTextSize(2);
display.print(weather_0_main);
display.display();
//Move the cursor down towards center of screen and make the temp text size 4
display.setCursor(25,25);
display.setTextSize(4);
display.print(temp);
//Draw Circle for degree symbol
display.drawCircle(83, 25, 5, SSD1306_WHITE);
//Make text size smaller to make the space between degree and F smaller - kinda hacky but it works
display.setTextSize(3);
display.print(" ");
//Change text size back to 4
display.setTextSize(4);
display.print("F");
display.display();
//Finally move the cursor towards the bottom of the screen and make the size 1 to display the number of refreshes
//I added this as a way to know that the device is updating as the refresh is every 5 minutes
display.setCursor(0,58);
display.setTextSize(1);
display.print(numRefresh);
display.display();

//Can only display 21 periods on the screen so reset it when it gets there
if(numRefresh.length() == 21)
  numRefresh = "";


delay(300000);//Wait 5 minutes

}
