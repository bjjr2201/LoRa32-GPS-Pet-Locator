/*********
 * Same dude Santos
 * same link
 *********/

 //Libraries for LoRa
 #include <SPI.h>
 #include <LoRa.h>

 //Libraries for OLED Display
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

 #define BAND 915E6

 //OLED pins
 #define OLED_SCL 15
 #define OLED_SDA 4
 #define OLED_RST 16
 #define SCREEN_WIDTH 128 
 #define SCREEN_HEIGHT 64
 
 Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
 String LoRaData;

void setup() {
  // put your setup code here, to run once:
  pinMode(OLED_RST, OUTPUT);;
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //Dont proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();

  Serial.begin(115200);
  Serial.println("LoRa Receiver Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, CS);
  LoRa.setPins(CS, RST, IRQ);

  if(!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while(1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {//received packet
    Serial.print("Received packet ");
   //read packet
   while (LoRa.available())
   {
    LoRaData = LoRa.readString();
    Serial.print(LoRaData);
   }

   //print RSSI of packet
   int rssi = LoRa.packetRssi();
   Serial.print(" with RSSI ");
   Serial.println(rssi);

   //Display information
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(LoRaData);
   display.setCursor(0,40);
   display.print("RSSI: ");
   display.setCursor(30,40);
   display.print(rssi);
   display.display();
  }
}
