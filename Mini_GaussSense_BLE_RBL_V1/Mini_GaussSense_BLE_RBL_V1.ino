// Using Serial1 on Arduino Leonardo
#define BLEMini Serial1

// GaussSense setup

/*  If using 4 Mini GaussSense modules (e.g. forming a 2x2 grid), the values should be changed as follows:
 *  const int MINI_GAUSSSENSE_HEIGHT = 2;     //Set the height of Mini GaussSense
 *  const int MINI_GAUSSSENSE_WIDTH  = 2;     //Set the width of Mini GaussSense
 *  const int analogInPin[] = {A0,A1,A2,A3};  //Specify the corresponding pins of each module in use.
 */

const int MINI_GAUSSSENSE_HEIGHT = 1;      //Set the height of Mini GaussSense
const int MINI_GAUSSSENSE_WIDTH  = 1;      //Set the width of Mini GaussSense
const int analogInPin[]   = {A0};          //Specify the corresponding pins of each module in use.
const int selectionPin[]  = {2, 3, 4, 5};  //Set the selection pins used
const int compressRate = 1;

// Analog values from Mini GaussSense sensor
int sensorVal[16 * MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH];

int sendDataMode = 0;

void setup()
{
	Serial.begin(57600);
  BLEMini.begin(57600);

  // GaussSenss setup
  for (int i = 0; i < 4; i++) pinMode(selectionPin[i], OUTPUT);
}

void loop()
{
  checkBTSerialPort();
  delay(10);
  if (sendDataMode == 1)
  {
    getGaussSenseData();
    sendGaussSenseData();
  }
  else if (sendDataMode == 2)
  {
    sendMetaData();
  }
}

void checkBTSerialPort() {
  while (BLEMini.available())
  {
    String str = BLEMini.readStringUntil('\n');
    if (str == "start" || str.indexOf("start") >= 0)
    {
      sendDataMode = 1;
    }
    else if (str == "stop" || str.indexOf("stop") >= 0)
    {
      sendDataMode = 0;
    }
    else if ("handshaking" || str.indexOf("handshaking") >= 0)
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
        // Centered at 512, ranged from [512-110, 512+110]
        int v = analogRead(analogInPin[y]) - 512;
        sensorVal[x + y * 16] = constrain(v, -100, 100);
    }
  }
}

void sendMetaData() {
  BLEMini.write(0x01);
  BLEMini.write(0x01);
  BLEMini.write(0x02);
  BLEMini.write(0x03);

  BLEMini.write(MINI_GAUSSSENSE_WIDTH);
  BLEMini.write(MINI_GAUSSSENSE_HEIGHT);
  BLEMini.write(0x00);
  BLEMini.write(compressRate);

  for (int i = 0; i < 12; i++) BLEMini.write(0x00);
}

void sendGaussSenseData() {
    for (int mini = 0; mini < MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH; mini++) {
      BLEMini.write(0x00);
      BLEMini.write(0x00);
      BLEMini.write(mini);
      BLEMini.write(MINI_GAUSSSENSE_HEIGHT * MINI_GAUSSSENSE_WIDTH);

      for (int i = 0; i < 16; i++) {
        BLEMini.write(sensorVal[i + mini*16]);
//        Serial.print(sensorVal[i + mini*16]);
//        Serial.print(" ");
      }
//      Serial.println();
      delay(10);
    }
//  }
}
