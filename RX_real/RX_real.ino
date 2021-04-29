#include <gamma.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>

// for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// unique id for this sign
char* myId = "01";

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 2, 1, 10,
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(255, 255, 0), matrix.Color(255, 255, 255) };


void start_radio()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial) {
    delay(1);
  }
  
  delay(100);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  rf95.setTxPower(23, false);
}


void start_matrix()
{
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(20);
  matrix.setTextColor(colors[0]);
  matrix.fillScreen(0);
  matrix.show();
}

void setup()
{
  start_radio();
  start_matrix();
  Serial.begin(115200);
}


struct radio_packet {
  char* id;
  int color;
  int speed_limit;
};


struct radio_packet parse_message(char* recieved) {
  // format for update string: 123456 01_0_25
  struct radio_packet packet;
  
  Serial.print("Whole packet: ");
  Serial.println(recieved);

  char *ptr = strtok(recieved, " "); 
  packet.id = ptr;
  
  ptr = strtok(NULL, " ");
  while(ptr != NULL)
  {
    
    //cur_inst will be in format: 01_0_25
    char *com_tok = strtok(ptr, "_");
    if (strcmp(com_tok, myId)) {
      packet.color = atoi(strtok(NULL, "_"));
      packet.speed_limit = atoi(strtok(NULL, "_"));
      Serial.print("Color: ");
      Serial.println(packet.color);
      Serial.print("Speed Limit: ");
      Serial.println(packet.speed_limit);
    }
    
    ptr = strtok(NULL, " ");
  }
  

  return packet;
}

void updateSign(int color, int speed_limit) {

  matrix.setTextColor(colors[color]);
  
  if (speed_limit != 0) {
    matrix.print("" + speed_limit);
  } else {
    matrix.fillScreen(colors[color]);
  }
   
  matrix.show();
}

void loop()
{
  char* recieved = "";
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      recieved = (char*)buf;
      Serial.println(recieved);
      struct radio_packet packet = parse_message(recieved);
      
      updateSign(packet.color, packet.speed_limit);
 
      //Serial.println(rf95.lastRssi(), DEC);
     
//      uint8_t data[27] = {strcat(strcat(myId, " has received "), packet.id)};
//      rf95.send(data, sizeof(data));
//      delay(100);
//      rf95.waitPacketSent();
      
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
