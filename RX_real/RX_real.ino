#include <gamma.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int myId = 1;
char* myIdString = "1";

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

//  while (!Serial) {
//    delay(1);
//  }
//  
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
  //Serial.println("LoRa radio init OK!");

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
  matrix.setBrightness(100);
  matrix.setTextColor(colors[0]);
  matrix.fillScreen(0);
  matrix.show();
}

void setup()
{
  start_radio();
  start_matrix();
  //Serial.begin(115200);
}


struct radio_packet {
  int id = 0;
  int color = 0;
  int speed_limit = 0;
};


struct radio_packet parse_message(char* recieved) {
  // format for update string: 123456 01_0_25
  struct radio_packet packet;

  char *token, *subtoken, *saveptr1, *saveptr2;

  token = strtok_r(recieved, " ", &saveptr1); 
  packet.id = atoi(token);
  
  token = strtok_r(NULL, " ", &saveptr1);

  while(token != NULL)
  {
//    Serial.print("Token: ");
//    Serial.println(token);
    subtoken = strtok_r(token, "_", &saveptr2);
    if (atoi(subtoken) == myId) {
      char* col = strtok_r(NULL, "_", &saveptr2);
      packet.color = atoi(col);
//      Serial.print("Color: ");
//      Serial.println(col);
      
      char* speedLim = strtok_r(NULL, "_", &saveptr2);
//      Serial.print("SpeedLim: ");
//      Serial.println(speedLim);
//      Serial.println();
      packet.speed_limit = atoi(speedLim);
      return packet;
      
    }
    
    token = strtok_r(NULL, " ", &saveptr1);
  }
  

  return packet;
}

void updateSign(int color, int speed_limit) {

  matrix.setTextColor(colors[color]);
//  Serial.print("speed limit: ");
//  Serial.println(speed_limit);
  if (speed_limit != 0) {
    matrix.fillScreen(0);
    matrix.setCursor(2, 1);
    char speedLimString[3];
    itoa(speed_limit, speedLimString, 10);
    matrix.print(speedLimString[0]);
    matrix.setCursor(9, 1);
    matrix.print(speedLimString[1]);
  } else {
    matrix.fillScreen(colors[color]);
  }
   
  matrix.show();
}


void loop()
{

  if (rf95.available())
  {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      struct radio_packet packet = parse_message((char*)buf);
      
      updateSign(packet.color, packet.speed_limit);
      
////      char packetID[7];
////      itoa(packet.id, packetID, 10); 
//      char* respond = "Sign 1 has recieved ok";
//      rf95.send((uint8_t *)respond, strlen(respond) + 1);
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
