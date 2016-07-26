#include <SPI.h>
#include "RFID.h"
#include <Servo.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN A0
//SPI PINS OF ProMicro
//SCK: Pin 15
//MOSI: Pin 16
//MISO: Pin 14

#define SEN_ROW 16
#define SEN_COL 1
#define SEN_NUM SEN_ROW*SEN_COL
// Read only 2 RFID instead of 5
#define RFID_BITNUM 2
#define BLE_NUM 20

RFID rfid(SS_PIN, RST_PIN);

// BLE setup (height & width must correspond with SEN_NUM)
SoftwareSerial BTSerial(9, 8); // RX | TX
const int MINI_GAUSSSENSE_HEIGHT = 1;     //Set the height of Mini GaussSense
const int MINI_GAUSSSENSE_WIDTH  = 1;     //Set the width of Mini GaussSense
int sendDataMode = 0;
const int compressRate = 1;
int zero = 0;
// BLE setup ends

int analogPins[] = {A3};

int sensorVal[SEN_NUM];
int rfidSN[RFID_BITNUM];
int IDSmoothCnt;
const int selectionPin[]  = {2, 3, 4, 5};  //Set the selection pins used
const int senID[] = {1, 3, 2, 6, 7, 5, 4, 12, 13, 15, 14, 10, 11, 9, 8, 0};
const int pinNum[] = {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 3};
const int pinVal[] = {1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0};

//byte buffer[SEN_NUM + RFID_BITNUM];
byte buffer[BLE_NUM];

int e0 = 0, e1 = 0, e2 = 0, e3 = 0;
int x = 0 , y = 0 , z = 0;

int initiated = false;
boolean sendAllowed = false;

long lastTime = millis();

boolean vibratorOn = false;

Servo myservo;
int counter = 0;
int CNT = 1;
int angle = -1;

int ledBrightness = 0;

void setup()
{
  Serial.begin(115200);
  BTSerial.begin(9600);
  
  for (int i = 0 ; i < 4 ; i++) {
    pinMode(selectionPin[i], OUTPUT);
    digitalWrite(selectionPin[i], 0);
  }
  pinMode(A0,OUTPUT);
  digitalWrite(A0, LOW);
  SPI.begin();
  rfid.init();
  IDSmoothCnt = 0;
  for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = 255;
}

void loop()
{
  checkBTSerialPort();
  if (sendDataMode == 1)
  {
    getRFIDData();
    getGaussSenseData();
    sendGaussSenseRFIDData();
  }
  else if (sendDataMode == 2)
  {
    sendMetaData();
  }
//  logRefreshRate();

//  Removed delay initially for BLE since RFID would already be delaying when reading tag IDs.
//  delay(20);
}

void checkBTSerialPort() { // Function for checking the serial commands
  while (BTSerial.available())
  {
//    String str = BTSerial.readStringUntil('\n');
    String str = BTSerial.readString();
    if (str == "start" || str.indexOf("start") >= 0)
    {
//      Serial.println("Start");
      sendDataMode = 1;
    }
    else if (str == "stop" || str.indexOf("stop") >= 0)
    {
      sendDataMode = 0;
    }
    else if (str == "handshaking" || str.indexOf("handshaking") >= 0)
    {
//      Serial.println("Handshake");
      sendDataMode = 2;
    }
  }
}

void getRFIDData() {
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = rfid.serNum[i];
      IDSmoothCnt=0;
    }
  } else {
    if(IDSmoothCnt>0) for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = -128;
    IDSmoothCnt++;
  }
  rfid.halt();

//  Serial.print("Tag ID: ");
//  for (int i = 0 ; i < RFID_BITNUM ; i++) Serial.print(rfidSN[i]);
//  Serial.println();
}

void getGaussSenseData() {
  if (counter == 0) {
    for (x = 0; x < SEN_ROW; x++) {
      digitalWrite(selectionPin[pinNum[x]], pinVal[x]);
      for (y = 0; y < SEN_COL; y++) {
        int i = analogPins[y];
        int v = analogRead(i) - 384;
        if(v<=1) v = 1;
        if(v>254) v = 254;        
        v -= 127;
        sensorVal[senID[x] + y * SEN_ROW] = v;
      }
    }
  }
}

void sendMetaData() {
  BTSerial.write(0x01);
  BTSerial.write(0x01);
  BTSerial.write(0x02);
  BTSerial.write(0x03);

  BTSerial.write(MINI_GAUSSSENSE_WIDTH);
  BTSerial.write(MINI_GAUSSSENSE_HEIGHT);
  BTSerial.write(zero);
  BTSerial.write(compressRate);

  BTSerial.write('r');
  for (int i = 0; i < 11; i++) BTSerial.write(zero);
}

void sendGaussSenseRFIDData() {
  for (int mini = 0; mini < MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH; mini++) {
//    BTSerial.write(zero);
//    BTSerial.write(zero);
    int n = 0;
    buffer[n++] = mini;
    buffer[n++] = MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH;
//    BTSerial.write(mini);
//    BTSerial.write(MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH);
    for (int i = 0; i < RFID_BITNUM; i++) {
//      BTSerial.write(rfidSN[i]);
      buffer[n++] = rfidSN[i];
    }
    
    for (int i = 0; i < 16; i++) {
//      BTSerial.write(sensorVal[i + mini*16]);
      buffer[n++] = sensorVal[i + mini*16];
    }
    BTSerial.write(buffer, BLE_NUM);
  }  
}

void logRefreshRate() {
  long currentTime = millis();
  Serial.println("Rate: " + String(1000/(currentTime - lastTime)));
  lastTime = currentTime;
}

