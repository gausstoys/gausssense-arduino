/*  ARDUINO CODE for MINI GAUSSSENSE (VER. 1) 
 *  Copyright (C) <2016> <GAUSSTOYS INC., TAIWAN (http://gausstoys.com)>
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Servo.h>

/*  If using 4 Mini GaussSense modules (e.g. forming a 2x2 grid), the values should be changed as follows:
 *  const int MINI_GAUSSSENSE_AMOUNT = 4;     //Specify the amount of the modules in use.
 *  const int analogInPin[] = {A0,A1,A2,A3};  //Specify the corresponding pins of each module in use.
 */
const int MINI_GAUSSSENSE_AMOUNT = 1;     //Set the amount of Mini GaussSense
const int analogInPin[] = {
  A2};          //Set the analog pins used
const int selectionPin[]  = {
  3,5,7,A0};  //Set the selection pins used

int sensorVal[16 * MINI_GAUSSSENSE_AMOUNT]; 
Servo myservo; 

void setup()
{
  Serial.begin(115200);
  for (int i = 0 ; i < 4 ; i++) pinMode(selectionPin[i], OUTPUT);
}

void loop()
{
  if (checkSerialPort()) {
    getGaussSenseData();  // Get data from all GaussSense modules
    sendGaussSenseData(); // Write data to the serial port

    /* If using additional digital pin(s) for input (e.g., reading a button from pin 7 [x1.DigitalInput])*/
    //      int digitalInPin = 7;
    //      pinMode(digitalInPin, INPUT_PULLUP);
    //      int v = digitalRead(digitalInPin); // Digital read value from button
    //      Serial.write(v);                   // Write button value to serial port

    /* If using additional analog pin(s) for input (e.g., reading a potentiometer from pin A4 [x2.AnalogInput])*/
    int analogInPin = A4;
    int v = analogRead(analogInPin);    // Digital read value from button
    v = map(v,0,1023,-128,127);         // Write resistor value to serial port
    Serial.write(v);                    // Write button value to serial port

  }
}

boolean checkSerialPort() { //Function for checking the serial commands
  boolean sendData = false;
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'a') sendData = true; //read
    if (inChar == 'w') { //write
      String str = Serial.readStringUntil('\n');
      if ( (str.length() >= 4) && (str[0] == 'd') ) { //digital write
        pinMode((str[1] - '0') * 10 + (str[2] - '0'), OUTPUT);
        digitalWrite((str[1] - '0') * 10 + (str[2] - '0'), (str[3] - '0'));
      }
      if ( (str.length() >= 6) && (str[0] == 'a') ) { //analog write
        pinMode((str[1] - '0') * 10 + (str[2] - '0'), OUTPUT);
        int v = (str[3] - '0') * 100 + (str[4] - '0') * 10 + (str[5] - '0');
        if (v >= 0 && v <= 255) analogWrite((str[1] - '0') * 10 + (str[2] - '0'), v);
      }
      if ( (str.length() >= 6) && (str[0] == 's') ) {//servo write
        myservo.attach((str[1] - '0') * 10 + (str[2] - '0'));
        int v = (str[3] - '0') * 100 + (str[4] - '0') * 10 + (str[5] - '0');
        if (v >= 0 && v < 180) myservo.write(v);
      }
    }
  }
  return sendData;
}

void getGaussSenseData() { //Function for getting data from all GaussSense modules
  int muxBits[4];
  for (int x = 0; x < 16; x++) {
    for (int i = 0; i < 4; i++) {
      muxBits[i] = (x >> i) & 0x01;
      digitalWrite(selectionPin[i], muxBits[i]);
    }
    for (int y = 0; y < MINI_GAUSSSENSE_AMOUNT ; y++) {
      int v = analogRead(analogInPin[y]) - 512;
      sensorVal[x + y * 16] = constrain(v, -127, 127);
//      Serial.print(constrain(v, -127, 127) + " ");
    }
//    Serial.println();
  }
}

byte buffer[16];
void sendGaussSenseData(){ //Function for writing data to the serial port
  int n = 0;
  for (int i = 0; i < 16 * MINI_GAUSSSENSE_AMOUNT; i++) {
    buffer[n] = sensorVal[i];
    n++;
  }
  Serial.write(buffer,16 * MINI_GAUSSSENSE_AMOUNT); 
}

