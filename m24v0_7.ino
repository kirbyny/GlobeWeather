/*
ESP32 GPIO pin assignments for reference
1 - Speaker
4 - Touch input (specific to ESP32)
5 - Time Meter
6 - Condition Meter
7 - Temp Meter
8 - PS2 Data
9 - PS2 Clock
10 - Baro Meter
11 - I2C Data (BME280)
12 - I2C Clock (BME280)
13 - Wind Speed Meter
14 - Wind Direction Meter
17 - Humidity Meter
18 - Storm Alert LED
21 - Backlight
38 - Red LED
47 - Green LED
*/

#include <Arduino.h>
#include <WiFiProvisioner.h>
#include <JSON_Decoder.h>
#include <OpenWeather.h>
#include <math.h>
#include <PS2MouseHandler.h>

#define DEBUG true                                    // Enables serial.print debug output
#define DEBUGRESET false                              // Erases saved wifi credentials at each reboot
#define DISPLAYENABLE 2                               // 0 off, 1 net LED only, 2 meters
#define DEBUGSTEP false                               // Pauses for user input at various debug points
#define BME280ENABLE true
#define WIFIENABLE true  
#define GLOBEENABLE true
#define BENCHMARK true                               // outputs loops-per-second

#if (DEBUG)
  #include <time.h>
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
  #define debugwr(x) Serial.write(x)
  #define enableSerialDebug(x) enableSerialDebug(true)
#else
  #define debug(x)
  #define debugln(x)
  #define debugwr(x)
  #define enableSerialDebug(x) enableSerialDebug(false)
#endif

#if (DEBUGRESET)
  #define debugReset() provisioner.resetCredentials()
#else
  #define debugReset()
#endif

#if (DEBUGSTEP)
  #define debugStep(x) letsWait(x)
#else
  #define debugStep(x)
#endif

#if (DISPLAYENABLE < 1)
  #define meterOutWrite(x, y) meterOut.write(x, y)
#else
  #define meterOutWrite(x, y)
#endif

#if (!WIFIENABLE)
  #define whichWx() getTestWx()
#else
  #define whichWx() getWx()
#endif

//bechmark and debug things
unsigned long loopTimer = millis();
unsigned long loopCounter = 0;


//mouse and globe things
const byte MOUSE_DATA = 8;
const byte MOUSE_CLOCK = 9;
const double GLOBE_R = 6.0; // globe raduis in inches
const double MOUSE_RES = 1000; // mouse resolution in pixels per inch.
const double GLOBE_SCALE = (MOUSE_RES * GLOBE_R); 
struct Point {
    double x;
    double y;
};
Point mousePoint = {0, 0}; 
Point currentLatLong = {40.865021,-72.798264}; //default location
const Point ORIGIN = {0, 0}; // Our origin is always 0.0
PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA, PS2_MOUSE_REMOTE);
const unsigned long MOUSE_INTERVAL = 20; 
unsigned long mouseTimer = millis();

//I2C For reference
const byte SDA_PIN = 11;
const byte SCL_PIN = 12;

//touchy-feelies
const byte TOUCH_PIN = 4;
const int TOUCH_DEBOUNCE = 100;                       //debounce
const unsigned long TOUCH_CAL = 10000;                 //determined empirically
const unsigned long TOUCH_TIMEOUT = 2 * 1000;         //resets the multi-touch counter
bool touchFlag = true;
int touchCounter = 10;                                // higher than threashold forces update on start
const int TOUCH_THRESH = 2;                           // number of touches in _TIMEOUT seconds
unsigned long touchTimer = millis();                  // remember last time we got touched

//I'm afraid of the dark
const byte BACKLT_PIN = 21;
const unsigned long BACKLT_TIMEOUT = 5 * 60 * 1000;   // minutes times seconds times millis
const int BACKLT_MAX = 128;                           // 50%
const int BACKLT_FADE = 10;                           // higher is slower
int backltLevel = 0;                                  // off
unsigned long backltTimer = millis();

//I'm Alive!
const byte GREEN_PIN = 47;                             // bi-pin bi-color LED
const byte RED_PIN = 38;
const byte OFF = 0;
const byte RED = 1;
const byte GREEN = 2;
bool netState = false;
const unsigned long BEAT_LENGTH = 50;
const unsigned long BEAT_INTERVAL = 1 * 1000;        // in seconds times millis
unsigned long beatTimer = 0;

//beeps and boops
const byte SPKR_PIN = 1;
const int SPKR_DUR = 50;
unsigned long spkrTimer = millis();

//meh
const unsigned long REFRESH_INTERVAL = 15 * 60 * 1000;// Default refresh rate in minutes times seconds times millis
unsigned long prevRefreshTime = 0;   
bool alertFlag = false;                               // wicked stahm comin (alert LED)

WiFiProvisioner::WiFiProvisioner provisioner;

char owLat[12];
char owLong[12];

OW_Weather ow;  

#if (BME280ENABLE)
  #include <Wire.h>
  #include <BME280I2C.h>

  const byte WX_BUFFER = 7;
  const byte DEFAULT_VIEW = 6;                           // max number of wx data memory locations + 1 for indoor weather
  byte currentView = DEFAULT_VIEW;                               // show indoor
  BME280I2C bme;
#else
  const byte WX_BUFFER = 6;
  const byte DEFAULT_VIEW = 0;
  byte currentView = DEFAULT_VIEW; 
#endif

struct wxDataObject {                                 //we're only using a few of these at this time
  float temperature;
  float feelsLike;
  float maxTemp;
  float minTemp;
  float windSpeed;
  int humidity;
  int conditionID;
  int windDirection;
  String description;
  float rainfall;
  float pressure;
  int visibility;
  long timeStamp;
  float gust;
  String conditionIcon;
  bool alert;
  String city;
  float moonphase;
};
struct wxDataObject wxData[WX_BUFFER];  

struct displayObject {
  const int ltrim;   
  const int utrim;
  const byte pin; 
  const int lscale;                                   
  const int uscale;  
};

const byte MAX_METERS = 8;                            //how many meters are we using?
displayObject meters[MAX_METERS] = {
  { 0, 255, 5, 0, 6 },                                //time - current, +6, +12, +18, +24, +48, Indoor (WX_BUFFER)
  { 0, 255, 6, 0, 100 },                              //sun, clouds, rain, snow, etc.. (9)
  { 0, 255, 7, 0, 100 },                             //temp in f/c
  { 0, 255, 13, 0, 50 },                              //windspeed in mph/kn/kkph
  { 0, 255, 14, 0, 359 },                             //wind direction in degrees
  { 0, 255, 10, 950, 1070 },                          //baro pressure in bar/in-mg/mm-hg
  { 0, 255, 17, 0, 100 },                             //humidity in %
  { 0, 255, 18, 0, 255 }                              //storm alert.  you can treat this like a bool
};

byte meterBuffer[2][MAX_METERS];                      //stores previous and current meter values

void setup() {

  Serial.begin(250000); //fast, so we don't block anything
  delay(3000);
  debugln("Kirby Instruments Model M24 - Beta 7");

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  touchAttachInterrupt(TOUCH_PIN, touched, TOUCH_CAL);
  for (byte i = 0; i < MAX_METERS; i++) {
    meterBuffer[i][0] = 0;
    meterBuffer[i][1] = 0;
  }

#if (BME280ENABLE)
  Wire.begin();
  debugln("Looking for BME280 sensor");
  while (!bme.begin()) delay(1);

  switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
      debugln("Found BME280 sensor");
      break;
    case BME280::ChipModel_BMP280:
      debugln("Found BMP280 but no humidity is available");
      break;
    default:
      debugln("Found unknown device on I2C bus");
  }
#else
  debugln("BME280 Disabled");
#endif

#if (WIFIENABLE)
  WiFi.onEvent(SysProvEvent);
  while (WiFi.status() != WL_CONNECTED) {
    debugReset();
    provisioner.enableSerialDebug(debug);
    provisioner.connectToWiFi();
  }
#else
  debugln("WiFi Disabled");
#endif

#if (GLOBEENABLE)
  if(mouse.initialise() != 0){
    Serial.println("Mouse Error");
  };
  
  mouse.set_resolution(MOUSE_RES);
#else
  debugln("Globe Disabled");
#endif

  globe();

#if (BENCHMARK)
  debugln("Benchmarking Enabled");
#else
  debugln("Benchmarking Disabled");
#endif

#if (DEBUG)
  debugln("Debug Output Enabled");
#endif

#if (DEBUGRESET)
  debugln("WiFi Reset Enabled");
#else
  debugln("WiFi Reset Disabled");
#endif

#if (DEBUGSTEP)
  debugln("Debug Stepping Enabled");
#else
  debugln("Debug Stepping Disabled");
#endif

#if (DISPLAYENABLE > 1)
  debugln("Meter Output Enabled");
#else
  debugln("Meter Output Disabled");
#endif

  debugln("I'm almost ready");
  whichWx();
  sweepDisplay();

  beep(true);
  debugln("I'm ready");
}


void loop() {                                         // individual tasks should contain no long loops or delays 

#if (BENCHMARK)                                       
  loopCounter++;
  if ((millis() - loopTimer) >= 1000 ) {
    loopTimer = millis();
    debug("Loops per second : ");
    debugln(loopCounter);
    loopCounter = 0;
  }
#endif

  touch();                                            //check for people - task 1

  showDisplay(currentView, false);                    //update the meters - task 2

  backlight();                                        //shut off the lights - task 3

  if (millis() > (spkrTimer + SPKR_DUR)) {            //Turn off the beep - task 4
    analogWrite(SPKR_PIN, 0); 
  }

  interval();                                         //time to update? - task 5

  heartbeat();                                        //we're alive - task 6

  globe();                                            //process globe - task 7

}

void touch() {                                        //Check for user input
  if (touchFlag) {
    backltTimer = millis();
    backltLevel = BACKLT_MAX;
    debugln("Backlight On");
    beep(false);

    if (millis() - touchTimer > TOUCH_TIMEOUT) {      // it's not multi-touch
      touchCounter = 0;
      touchTimer = millis();
    } else {
      touchCounter++;
      debugln(touchCounter);
    }

    if (touchCounter > TOUCH_THRESH) {
      debugln("OK, I'll go get the forecast.");
      touchCounter = 0;
      whichWx();                                      // and refresh the forecast if touched touchCounter times in _TIMEOUT seconds
      currentView = DEFAULT_VIEW;
    } else {                                          // we don't want to jump forward on the first time through the loop
      currentView++;
    }

    if (currentView >= WX_BUFFER) currentView = 0;

    showDisplay(currentView, true);
    printValues(currentView);                         //pretty.print
    touchFlag = false;
  }
}

void backlight() {                                    //Adjust the backlight
  if ((millis() - backltTimer > BACKLT_TIMEOUT) && !backltLevel)  { //using the backlight brightness like a bool here

    analogWrite(BACKLT_PIN, backltLevel);
    backltLevel--;                                    //linear fade to off
    backltTimer = backltTimer + BACKLT_FADE;          //slow it down without blocking

    if (backltLevel < 0) {
      debugln("Backlight off");
      backltTimer = millis();
      backltLevel = 0;
    }
  }
}

void interval() {                                     //Update and refresh when interval expires
  if (millis() - prevRefreshTime > REFRESH_INTERVAL) {

    if (whichWx()) {                                  //if we fail, we'll try again on the next loop
      currentView = DEFAULT_VIEW;
      showDisplay(currentView, true);
      printValues(currentView);                       //pretty.print
      prevRefreshTime = millis();
    } else {
      debugln("I didn't get the weather forecast");
    }

  }
}

void heartbeat() {
  if (millis() - beatTimer > BEAT_INTERVAL) {

    if (netState) {
      netStatus(GREEN);
    } else {
      netStatus(RED);
    }
    
    if (millis() - beatTimer > (BEAT_INTERVAL + BEAT_LENGTH)) {
      netStatus(OFF);
      beatTimer = millis();
      debug("Current Lat : ");
      debugln(owLat);
      debug("Current Long : ");
      debugln(owLong);
    }
  }
}

void touched() {
  if (millis() - touchTimer > TOUCH_DEBOUNCE) touchFlag = true;
}

void beep(bool block) {
  analogWrite(SPKR_PIN, 128);
  spkrTimer = millis();
  if (block) delay(SPKR_DUR);
}

long mymap(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}