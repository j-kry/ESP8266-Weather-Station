#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

#include <ArduinoJson.h>


//DEFINE WIFI THINGS
const char* ssid = "YOURSSIDGOSHERE";
const char* password = "YOURPASSWORDGOESHERE";
WiFiClient client;
char server[] = "api.openweathermap.org";

#define SEALEVELPRESSURE_HPA (1013.25)

//Declare bme sensor
Adafruit_BME280 bme;

//DEFINE SCREEN THINGS
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //Address for left screen
#define SCREEN2_ADDRESS 0x3D //Address for right screen

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Declare function up here because c++ is stupid
void UpdateWeather();

//Use these for temperature sensor
float bmeTemp = 0;
int bmeTempInt = 0;

void setup() {

  Serial.begin(9600);

  //Start up both displays
  //
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  if(!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN2_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Start up bme sensor
  bool status = bme.begin(0x76);
  if(!status) {
    Serial.println("Could not find BME280 sensor");
    while(1);
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  display2.display();
  delay(2000); // Pause for a bit

  // Clear the buffer
  display.clearDisplay();
  display2.clearDisplay();

  display.setTextSize(2); // 2 is sweet spot for one line to take up yellow color
  display2.setTextSize(2);//
  display.setTextColor(SSD1306_WHITE); // doesnt matter its monochrome
  display2.setTextColor(SSD1306_WHITE);//
  display.setCursor(0,0);// set cursor to top left of display
  display2.setCursor(0,0);//
  display.cp437(true);//This fixes characters in gfx library. not really needed
  display2.cp437(true);//

  /////////////////////////////////////////////////////////////
  //=/=/=/=/=/=/=/=/=BEGIN ACTUAL PROGRAM=/=/=/=/=/=/=/=/=/=/=/
  ////////////////////////////////////////////////////////////

  //Start wifi module
  WiFi.begin(ssid, password);

  //Clear display, set cursor top left, print connecting... until connected
  display.clearDisplay();
  display2.clearDisplay();
  display.setCursor(0,0);
  display2.setCursor(0,0);
  display.print("Connecting");
  display2.print("Connecting");
  display.display();
  display2.display();

  while(WiFi.status() != WL_CONNECTED) {
    
    delay(500);
    display.print(".");
    display2.print(".");
    display.display();
    display2.display();

  }

  //Clear display, set cursor top left, print connected and IP address
  display.clearDisplay();
  display2.clearDisplay();
  display.setCursor(0,0);
  display2.setCursor(0,0);
  display.println("Connected!");
  display2.println("Connected!");
  display.print("IP Address");
  display2.print("IP Address");
  display.print(WiFi.localIP());
  display2.print(WiFi.localIP());
  display.display();
  display2.display();

  //Wait a bit
  delay(2000);

  //Pull the outside weather data for first run
  UpdateWeather();

}

void loop() {

  //Loop 10 times for 30 seconds each === 5 minutes then update weather
  //For loop displays indoor temp and humidity
  for(int i = 0; i < 100; i++) {

    display2.clearDisplay();
    display2.setCursor(0,0);
    display2.setTextSize(2);
    display2.print(bme.readHumidity());
    display2.println("%  IN");
    display2.setTextSize(4);
    display2.setCursor(25,25);
    bmeTemp = (1.8 * bme.readTemperature()) + 32;
    bmeTempInt = bmeTemp;
    display2.print(bmeTempInt);
    display2.drawCircle(83, 25, 5, SSD1306_WHITE);
    display2.setTextSize(3);
    display2.print(" ");
    display2.setTextSize(4);
    display2.println("F");
    display2.display();
    

    delay(3000); //Wait 3 seconds

  }

  UpdateWeather();

}

void UpdateWeather() {

   if(client.connect(server, 80)) {

      Serial.print("Connected to openweathermap");
      client.println("GET /data/2.5/weather?q=Chicago&APPID=*************YOURAPPIDGOESHERE***********&units=imperial HTTP/1.0");
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
    //Degree Symbol
    display.drawCircle(83, 25, 5, SSD1306_WHITE);
    display.setTextSize(3);
    display.print(" ");
    display.setTextSize(4);
    display.print("F");
    display.display();
      
}
