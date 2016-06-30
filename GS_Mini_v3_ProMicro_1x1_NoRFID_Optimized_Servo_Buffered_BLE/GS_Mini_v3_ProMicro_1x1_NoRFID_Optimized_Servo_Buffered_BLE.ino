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
#define RFID_BITNUM 5

RFID rfid(SS_PIN, RST_PIN);

// BLE setup (height & width must correspond with SEN_NUM)
SoftwareSerial BTSerial(9, 8); // RX | TXSoftwareSerial BTSerial(9, 8); // RX | TX
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

byte buffer[SEN_NUM + RFID_BITNUM];

int e0 = 0, e1 = 0, e2 = 0, e3 = 0;
int x = 0 , y = 0 , z = 0;

int initiated = false;
boolean sendAllowed = false;

long lastTime = micros();

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
//  SPI.begin();
//  rfid.init();
  IDSmoothCnt = 0;
  for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = 255;
}

void loop()
{
  checkBTSerialPort();
  if (sendDataMode == 1)
  {
    getGaussSenseData();
    sendGaussSenseData();
  }
  else if (sendDataMode == 2)
  {
    sendMetaData();
  }
  Serial.println("haha");
  delay(20);
}

void checkBTSerialPort() { // Function for checking the serial commands
  while (BTSerial.available())
  {
    String str = BTSerial.readStringUntil('\n');
//    Serial.println(str);
    if (str == "start")
    {
//      Serial.println("Start");
      sendDataMode = 1;
    }
    else if (str == "stop")
    {
      sendDataMode = 0;
    }
    else if ("handshaking")
    {
//      Serial.println("Handshake");
      sendDataMode = 2;
    }
  }
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

  for (int i = 0; i < 12; i++) BTSerial.write(zero);
}

void sendGaussSenseData() {
  for (int mini = 0; mini < MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH; mini++) {
      BTSerial.write(zero);
      BTSerial.write(zero);
      BTSerial.write(mini);
      BTSerial.write(MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH);
      
      for (int i = 0; i < 16; i++) {
        BTSerial.write(sensorVal[i + mini*16]);
//        Serial.print(sensorVal[i + mini*16]);
//        Serial.print(" ");
      }
//      Serial.println();
//      delay(10);
  }
  
}


/*void loop()
{
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'a') {
      sendAllowed = true;
    }
    if (inChar == 'w') {
      String str = Serial.readStringUntil('\n');
      if (str.length() >= 2) {
        int vType = str[0];
        if (vType == 'v') {
          pinMode(7, OUTPUT);
          int v0 = str[1] - '0';
          if (v0 >= 0 && v0 <= 1) {
            sendAllowed = true;
            if (v0 == 0) vibratorOn = false;
            else vibratorOn = true;
          }
        }
        if (vType == 'l') {
          if (str.length() >= 3) {
            int v0 = str[1] - '0';
            int v1 = str[2] - '0';
            if (v0 >= 0 && v0 <= 5) {
              if (v1 >= 0 && v1 <= 9) {
                sendAllowed = true;
                ledBrightness = 5 * (10 * v0 + v1);
                if (ledBrightness > 255) ledBrightness = 255;
              }
            }
          }
        }
        if (vType == 's') {
          if (str.length() >= 3) {
            int v0 = str[1] - '0';
            int v1 = str[2] - '0';
            if (v0 >= 0 && v0 <= 5) {
              if (v1 >= 0 && v1 <= 9) {
                sendAllowed = true;
                counter = CNT;
                angle = 3 * (10 * v0 + v1);
              }
            }
          }
        }
      }
    }
  }
  if (sendAllowed) {
    if (counter == 0) {
//      lastTime = micros();
//      if (rfid.isCard()) {
//        if (rfid.readCardSerial()) {
//          for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = rfid.serNum[i];
//          IDSmoothCnt=0;
//        }
//      } else {
//        if(IDSmoothCnt>0) for (int i = 0 ; i < RFID_BITNUM ; i++)  rfidSN[i] = -128;
//        IDSmoothCnt++;
//      }
//      rfid.halt();
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
      int n = 0;
      for (int i = 0; i < SEN_NUM; i++) {
        buffer[n] = sensorVal[i];
        n++;
      }
      for (int i = 0; i < RFID_BITNUM; i++) {
        buffer[n] = rfidSN[i];
        n++;
      }
      Serial.write(buffer, SEN_NUM + RFID_BITNUM);
      sendAllowed = false;
    }
  }
}
*/


