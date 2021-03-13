/****************************************
 * Include Libraries
 ****************************************/
#include "heltec.h"
#include "images.h"
#include <BlynkSimpleEsp32.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"

/****************************************
 * Configure WiFi Connection
 ****************************************/
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Your Blynk Auth Token
char ssid[]= "YourNetwork"; // Your network name
char pass[]= "YourPassword";  //Your password

/****************************************
 * Define Constants
 ****************************************/

static const int RXPin = 17, TXPin = 2; // GPS transmisson lines
static const uint32_t GPSBaud = 9600; // Default baud rate for NEO-7M GPS module

TinyGPSPlus tinyGPS;  // Create a tinyGPS object
SoftwareSerial ssGPS(RXPin, TXPin); // Assigns pins to GPS module

#define BAND    915E6  //you can set band here directly,e.g. 868E6 (Europe),915E6 (North America)
#define BLYNK_PRINT Serial
#define DHTPIN 12
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
unsigned int counter = 0; // Available for use
String rssi = "RSSI --";  // Signal strength measured in dB
String packSize = "--";   // Packet size measured in bytes
String packet ;

/****************************************
 * Main Function Setup
 ****************************************/
void setup()
{
  Serial.begin(115200);
  ssGPS.begin(GPSBaud);
  Serial.println(F("Ublox NEO-7M GPS Module Test"));
  delay(1000);

   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  
  // Display images called from the "image.h" header file
  logo();         // This is only for looks and not necessary. Can be commented out?
  delay(1500);
  Heltec.display->clear(); 
  Heltec.display->drawString(0, 0, "LoRa32 GPS Pet Locator");
  Heltec.display->display();
  delay(1000);
  dht.begin();
  
  // Established wifi connection to access Blynk feature
  //Blynk.begin(auth, ssid, pass);
}

/****************************************
 * Loop Function Setup
 ****************************************/

void loop()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  // Send GPS data to OLED display
  gps_OLED();

  printTemp();
  
  // print position, altitude, speed, time/date, and satellites:
  printGPSInfo();

  // "Smart delay" looks for GPS data while the Arduino's not doing anything else
  smartDelay(1000);
  
  // send packet
  LoRa.beginPacket();
  
/*
 * LoRa.setTxPower(txPower,RFOUT_pin);
 * txPower -- 0 ~ 20
 * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
 *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("Lat: " +String(tinyGPS.location.lat(), 6)); // 6-digit accuaracy equates to approx. 3-meters when conditions are clear
  LoRa.print("Lng: " +String(tinyGPS.location.lng(), 6));
  LoRa.print("Date: " +String(tinyGPS.date.value()));     // Format: DDMMYYYY
  LoRa.print("Time: " +String(tinyGPS.time.value()-6E6));     // Numerical value in a 24-hour Format
  LoRa.endPacket();
  LoRa.sleep();
  //Blynk.run();  // Automatically run all Blynk functions

  counter++;                 // Not being used for anything currently
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second  
}

/****************************************
 * Auxiliar Functions
 ****************************************/

void logo()                   // Draws logos from the hexadecimal code included in the "image.h" file.
{
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

// Used to send GPS data to OLED display
void gps_OLED()
{
  Heltec.display->clear();
  Heltec.display->drawString(0, 0,"Lat: " +(String(tinyGPS.location.lat(), 6)));
  Heltec.display->drawString(0, 10,"Lng: " +String(tinyGPS.location.lng(), 6));
  Heltec.display->drawString(0, 20,"Date: " +String(tinyGPS.date.month()) +("/")
  +String(tinyGPS.date.day()) +("/") +String(tinyGPS.date.year()));
  Heltec.display->drawString(0, 30,"Time: " +String(tinyGPS.time.hour()-6) +(":")
  +String(tinyGPS.time.minute()) +(":") +String(tinyGPS.time.second()));
  Heltec.display->drawString(0, 40,"Temp(F): " +String(dht.readTemperature(true)) +("%"));
  Heltec.display->display();
  
}

void printGPSInfo()
{
  // Print latitude, longitude, date, time
  Serial.print("Lat: "); Serial.println(tinyGPS.location.lat(), 6);
  Serial.print("Long: "); Serial.println(tinyGPS.location.lng(), 6);
  Serial.print("Date: "); printDate();  // function details below
  Serial.print("Time: "); printTime();  // function details below 
  Serial.println();
}

// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (ssGPS.available())
      tinyGPS.encode(ssGPS.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinyGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

// printDate() formats the date into mm/dd/yy.
void printDate()
{
  Serial.print(tinyGPS.date.month());
  Serial.print("/");
  Serial.print(tinyGPS.date.day());
  Serial.print("/");
  Serial.println(tinyGPS.date.year());
}

// printTime() formats the time into "hh:mm:ss", and prints leading 0's
// where they're called for.
void printTime()
{
  Serial.print(tinyGPS.time.hour()-8);
  Serial.print(":");
  if (tinyGPS.time.minute() < 10) Serial.print('0');
  Serial.print(tinyGPS.time.minute());
  Serial.print(":");
  if (tinyGPS.time.second() < 10) Serial.print('0');
  Serial.println(tinyGPS.time.second());
}

void printTemp()
{
  delay(2000);
  float f = dht.readTemperature(true);
  Serial.print(F("Temperature: "));
  Serial.println(f);
}

// Writes GPS data to virtual pin V1 of Blynk the interface.
BLYNK_WRITE(V1) {
//  GpsParam gps(param);

  // Print 6 decimal places for Lat, Lon
  Serial.print("Lat: ");
  Serial.println(tinyGPS.location.lat(), 7);

  Serial.print("Lon: ");
  Serial.println(tinyGPS.location.lng(), 7);

  // Print 2 decimal places for Alt, Speed
  Serial.print("Altitute: ");
  Serial.println(tinyGPS.altitude.feet(), 2);

  Serial.print("Speed: ");
  Serial.println(tinyGPS.speed.mph(), 2);

  Serial.println();
}

/****************************************
 * End of Program
 ****************************************/
