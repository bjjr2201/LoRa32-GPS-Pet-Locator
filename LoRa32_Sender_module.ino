/******
 * By: Rui Santos
 * Complete project details @ 
 * https://RandomNerdTutorials.com/ttgo-lora32-sx1276-arduino-ide/
 * added the notes from the page to explain code
 ******/

//Libraries that interact with LoRa chip
#include <SPI.h>
#include <LoRa.h>

//Libraries to interface with I2C OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define MISO 19
#define MOSI 27
#define SCK 5
#define CS 18
#define IRQ 26
#define RST 14

//915E6 for North America (select LoRa frequency)
#define BAND 915E6

//Define OLED pins
#define OLED_SCL 15
#define OLED_SDA 4
#define OLED_RST 16
#define SCREEN_WIDTH 128 //Define OLED display width, in pixels
#define SCREEN_HEIGHT 64 //Define OLED display height, in pixels

//create counter variable to keep track # of LoRa packets sent
int counter = 0;

//Create Adafruit_SSD1306 object called display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() 
{
  //reset OLED display via software (to start using the OLED you need to do a manual
  //reset via software using the RST pin.  To do this reset, you need to declare the
  //RST pin as an output, set it to LOW for a few milliseconds and then, set it to
  //HIGH again. 
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //Start an I2C communication using the defined SDA & SCL pins using Wire.begin()
  Wire.begin(OLED_SDA, OLED_SCL);
  
  //initialize the display with the following parameters. the parameters set as false
  //ensure that the library doesnt use the default I2C pins and use instead GPIO4 & 15
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) 
  { //Address 0x3c for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //Don't proceed, loop forever
  }
  //write the message "LORA SENDER" to the display
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();

  //initialize Serial Monitor for debugging purposes
  Serial.begin(115200);
  Serial.println("LoRa Sender Test");

  //define SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, CS);
  //setup LoRa transceiver module
  LoRa.setPins(CS, RST, IRQ);

  //finally, initialize the LoRa transceiver module using the begin() method on the
  //LoRa object and pass the frequency as argument
  if(!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while(1);
  }
  //if we succeed in initializing the display, we write a success message on OLED
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

//in the loop() is where we'll send the packets.you initialize a packet with the 
//beginpacket() method. you write data into the packet using print() method.
//close packet with the endPacket() method.
void loop() 
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("Hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  //write counter on the OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("LoRa packet sent.");
  display.setCursor(0,30);
  display.print("Counter: ");
  display.setCursor(50,30);
  display.print(counter);
  display.display();

  //after this, the counter message is incremeted by one in every loop, which 
  //happens every 10 seconds. 
  counter++; 
  delay(10000);
}
