/*  ARDUINO CODE for MINI GAUSSSENSE (VER. 1) 
 *  Copyright (C) <2016> <GAUSSTOYS INC., TAIWAN (http://gausstoys.com)>
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // RX | TX

const int MINI_GAUSSSENSE_HEIGHT = 1;     //Set the height of Mini GaussSense
const int MINI_GAUSSSENSE_WIDTH  = 1;     //Set the width of Mini GaussSense
const int analogInPin[]   = {A0};           //Set the analog pins used
const int selectionPin[]  = {2, 3, 4, 5};  //Set the selection pins used
const int compressRate = 1;

int sensorVal[16 * MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH];
int zero = 0;

int sendDataMode = 0;

void setup() {
  Serial.begin(57600);
  BTSerial.begin(9600);

  for (int i = 0 ; i < 4 ; i++) pinMode(selectionPin[i], OUTPUT);
}

void loop() {
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
  delay(20);
}


void checkBTSerialPort() {
  while (BTSerial.available())
  {
    String str = BTSerial.readStringUntil('\n');
    if (str == "start")
    {
      sendDataMode = 1;
    }
    else if (str == "stop")
    {
      sendDataMode = 0;
    }
    else if ("handshaking")
    {
      sendDataMode = 2;
    }
  }
}

void getGaussSenseData() { //Function for getting data from all GaussSense modules
  int muxBits[4];
  for (int x = 0; x < 16; x++) {
    for (int i = 0; i < 4; i++) {
      muxBits[i] = (x >> i) & 0x01;
      digitalWrite(selectionPin[i], muxBits[i]);
    }
    for (int y = 0; y < MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH; y++) {
      int v = analogRead(analogInPin[y]) - 512;
      sensorVal[x + y * 16] = constrain(v, -127, 127);
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

void sendGaussSenseData(){ //Function for writing data to the serial port
  for (int mini = 0; mini < MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH; mini++) {
    if (mini > 0) delay(10);
    BTSerial.write(zero);
    BTSerial.write(zero);
    BTSerial.write(mini);
    BTSerial.write(MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH);
    
    for (int i = 0; i < 16; i++) {
      BTSerial.write(sensorVal[i + mini*16]);
//      Serial.print(sensorVal[i + mini*16]);
//      Serial.print(" ");
    }
//    Serial.println();
  }
}


