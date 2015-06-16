/*
	SmartLamp Code
    
    Copyright (C) 2015 - Francesco Salamone
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//An IR LED must be connected to Arduino MEGA2560 PWM pin 9.

#include <IRremote.h>
#include <DHT.h>
#include <EEPROM.h>  //verify if your EEPROM library support EEPROM.update. Otherwise download new library from :https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr/libraries/EEPROM
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>

IRsend irsend;

#define DHTPIN 2
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

int value = 0;

RTC_DS1307 rtc;

File myFile; //SD card: define myFile

//Here below you have to define the operation time of PdC (24 hour format)
int Start = 7; 
int Stop = 18;

void setup() {
  Serial.begin(9600);
  pinMode(56, OUTPUT); // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  if (!SD.begin(4)) {
    Serial.println(F("initialization failed!"));
    return;
  }
  Serial.println(F("initialization done."));

  dht.begin();
  Wire.begin();
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  //adjust time and date of RTC with these of your PC in the first upload. After uploading add "//" and consider this line as a comment. Then reupload.

  Serial.println("Year, Month, Day, Hour, Minute, Second, Hum%, Temp*C, Heat, Cool, Deum");

  //instruction to write all data to sd card
  myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
  //then, write on it
  if (myFile) {
    myFile = SD.open("data.txt", FILE_WRITE);
    myFile.println("Year, Month, Day, Hour, Minute, Second, Hum%, Temp*C, Heat, Cool, Deum");
    myFile.close();
  }

  DateTime now = rtc.now();
  //delay(1000); // Reading temperature or humidity takes about 250 milliseconds!
  if (now.hour() >= Start && now.hour() < Stop) {
    IR_I_control();  //at startup, if it's in the range of hours defined above, it checks and controls PdC immediately for the first time
    delay(60000);   //wait a minute.
  }
}

int khz = 38; //NB Change this default value as neccessary to the correct modulation frequency
//Here below you have to insert your PdC signals
// ____________________Heating ON 24Â°C:____________________
unsigned int Signal_1_1[] = {
6596,7588,680,3344,680,3348,684,3340,684,1376,656,3344,676,1380,652,3348,688,3312,680,1380,652,1376,656,1376,656,3344,676,1380,652,3352,680,1376,660,1424,656,3344,680,3344,680,1380,652,1380,652,3348,684,3340,684,3344,676,1356,656,1372,660,1372,660,3340,684,3344,676,1380,652,1380,652,1380,652,3328,684,1348,684,3340,684,1348,684,3344,676,1356,676,3348,676,1356,676,1312,680,3344,676,1356,676,3352,684,1344,624,3404,628,1404,628,3396,628,3332,628,7412,636
}; //AnalysIR Batch Export - RAW

// ____________________Heating ON 22Â°C:____________________
unsigned int Signa2_1_1[] = {
6588,7592,676,3348,688,3340,680,3348,684,1344,680,3348,684,1372,660,3340,680,3320,684,1348,684,1372,660,1372,656,3344,680,1352,680,3348,676,1352,680,1404,676,3352,680,3344,680,1352,680,1352,680,3348,684,1344,676,1356,676,3324,680,1352,680,1348,684,3344,676,3352,680,1348,684,3344,680,3348,684,1328,684,1348,684,3340,680,1352,680,3348,676,1356,676,3348,684,1348,684,1328,652,3348,684,1352,680,3344,680,1348,684,3344,676,1356,676,3348,684,3272,680,7364,680
}; //AnalysIR Batch Export - RAW

// ____________________Dehumidification ON 22Â°C:____________________
unsigned int Signa3_1_1[] = {
6592,7588,680,3348,684,3344,676,3348,684,1376,656,3344,680,1376,652,3348,684,3316,676,1380,652,1380,652,1380,652,3348,684,1372,660,3340,680,1376,656,1432,652,3344,688,1372,656,3344,680,1376,656,3344,676,1384,648,1380,652,3324,676,1380,652,3348,684,1372,660,3340,680,1380,652,3348,676,3348,684,1356,652,1380,652,3348,684,1372,660,3340,680,1380,652,3348,676,1380,648,1312,680,3348,672,1384,648,3352,680,1376,656,3344,680,1380,600,3400,680,3276,676,7368,676
}; //AnalysIR Batch Export - RAW

// ____________________PdC off :____________________
unsigned int Signa3_0_0[] = {
6584,7596,684,3344,680,3344,688,3340,680,1352,680,3344,688,1344,676,3348,688,3312,684,1348,684,1348,684,1348,684,3340,680,1352,680,3348,684,1344,676,1412,684,3340,680,1352,680,3344,676,3352,680,3348,676,1352,676,1356,676,3324,680,1352,680,3344,684,1348,684,1348,684,1348,684,3340,680,3348,676,1336,684,1348,684,3340,680,1352,680,3348,684,1348,672,3352,680,1352,680,1304,676,3352,680,1352,680,3344,676,1356,676,3352,680,1352,680,3344,676,3280,680,7364,680
}; //AnalysIR Batch Export - RAW

// ____________________Dehumidification ON 25Â°C:____________________
unsigned int Signa4_1_1[] = {
6588,7592,676,3348,684,3344,680,3348,684,1344,688,3340,680,1352,680,3344,684,3316,688,1344,688,1344,676,1352,680,3348,680,1352,680,3344,680,1352,676,1408,676,3352,680,1352,680,3344,676,1356,676,1352,680,3348,684,3344,676,1328,684,1344,688,3340,680,1352,680,3344,688,3340,680,1352,680,1352,680,3328,684,1344,684,3344,680,1352,680,3344,684,1348,676,3352,680,1348,684,1304,684,3344,676,1352,632,3396,624,1408,624,3400,632,1400,632,3396,624,3332,628,7416,628
}; //AnalysIR Batch Export - RAW

// ____________________Cooling ON 25Â°C:____________________
unsigned int Signa5_1_1[] = {
6592,7588,680,3344,688,3340,680,3348,684,1344,688,3340,680,1352,680,3344,676,3324,680,1352,676,1356,676,1352,680,3348,684,1348,684,3340,680,1352,680,1404,680,1352,680,3344,688,3340,680,1352,680,1348,680,3348,684,3344,680,1324,676,3348,684,1348,684,1348,688,3336,680,3348,684,1348,684,1344,688,3324,676,1352,680,3348,680,1352,680,3344,680,1352,680,3348,680,1348,684,1304,676,3352,680,1348,684,3344,676,1356,624,3400,632,1400,624,3404,628,3324,636,7408,632
}; //AnalysIR Batch Export - RAW

// ____________________Cooling ON 23Â°C:____________________
unsigned int Signa6_1_1[] = {
6584,7596,684,3344,676,3348,684,3344,676,1356,676,3348,684,1348,684,3344,676,3324,676,1352,680,1352,680,1352,680,3344,688,1344,688,3340,680,1348,684,1400,680,1352,680,3348,684,3340,680,1352,680,1352,680,1348,684,1348,684,3316,684,3344,676,1352,680,1352,680,3348,684,3340,680,3348,684,3340,680,1336,676,1356,672,3352,680,1352,680,3344,680,1352,676,3352,680,1352,680,1304,624,3404,680,1348,632,3396,624,1408,624,3400,632,1400,632,3396,624,3332,628,7416,624
}; //AnalysIR Batch Export - RAW

void loop() {
  DateTime now = rtc.now();
  if (now.hour() >= Start && now.hour() < Stop) {
    if (now.minute() == 0 || now.minute() == 15 || now.minute() == 30 || now.minute() == 45) {
      IRcontrol();
      //delay(60000*15);   //check each 15 minutes.
      delay(60000);  //check each minute.
    }

    //else if (now.minute() != 0 && now.minute() && 15 && now.minute() && 30 || now.minute() && 45) {
    else {
      Serial.print(int(now.year()));
      Serial.print(',');
      Serial.print(int(now.month()));
      Serial.print(',');
      Serial.print(int(now.day()));
      Serial.print(',');
      Serial.print(int(now.hour()));
      Serial.print(',');
      Serial.print(int(now.minute()));
      Serial.print(',');
      Serial.print(int(now.second()));
      Serial.print(',');
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius
      float t = dht.readTemperature();

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }

      Serial.print(h);
      Serial.print(",");
      Serial.print(t);
      Serial.print(",");
      Serial.println("aa,aa,aa"); //"aa" = as above
      //Serial.println(EEPROM.read(1)); //debug

      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.print(int(now.year()));
        myFile.print(',');
        myFile.print(int(now.month()));
        myFile.print(',');
        myFile.print(int(now.day()));
        myFile.print(',');
        myFile.print(int(now.hour()));
        myFile.print(',');
        myFile.print(int(now.minute()));
        myFile.print(',');
        myFile.print(int(now.second()));
        myFile.print(',');
        myFile.print(h);
        myFile.print(',');
        myFile.print(t);
        myFile.print(',');
        myFile.println("aa,aa,aa"); //"aa" = as above
        myFile.close();
      }
      delay(60000);   //check each minute.
    }
  }

  else {
    irsend.sendRaw(Signa3_0_0, sizeof(Signa3_0_0) / sizeof(int), khz); //turn off the PdC
    value = 30;
    EEPROM.update(1, value);
    Serial.println("0,0,0"); //Heating, Cooling and Dehumidification off
    //Serial.println(EEPROM.read(1));  //debug
    //instruction to write all data to sd card
    myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
    //then, write on it
    if (myFile) {
      myFile = SD.open("data.txt", FILE_WRITE);
      myFile.println("0,0,0");
      myFile.close();
    }
    delay(60000 * 60 * (Stop - Start)); //check each 60000 (millisecond in 1 minute) * 60 (minutes) * (Stop - Start) (hours between Start and Stop).
  }
}

void IR_I_control() {
  DateTime now = rtc.now();
  Serial.print(int(now.year()));
  Serial.print(',');
  Serial.print(int(now.month()));
  Serial.print(',');
  Serial.print(int(now.day()));
  Serial.print(',');
  Serial.print(int(now.hour()));
  Serial.print(',');
  Serial.print(int(now.minute()));
  Serial.print(',');
  Serial.print(int(now.second()));
  Serial.print(',');
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print(h);
  Serial.print(",");
  Serial.print(t);
  Serial.print(",");

  //instruction to write all data to sd card
  myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
  //then, write on it
  if (myFile) {
    myFile = SD.open("data.txt", FILE_WRITE);
    myFile.print(int(now.year()));
    myFile.print(',');
    myFile.print(int(now.month()));
    myFile.print(',');
    myFile.print(int(now.day()));
    myFile.print(',');
    myFile.print(int(now.hour()));
    myFile.print(',');
    myFile.print(int(now.minute()));
    myFile.print(',');
    myFile.print(int(now.second()));
    myFile.print(',');
    myFile.print(h);
    myFile.print(',');
    myFile.print(t);
    myFile.print(',');
    myFile.close();
  }


  if (t < 21) {
    irsend.sendRaw(Signal_1_1, sizeof(Signal_1_1) / sizeof(int), khz); //Accensione modalità riscaldamento 24°C
    Serial.println("241,0,0");  //Riscaldamento a 24°C ON (24-1), Raffrescamento e deumidificazione off
    value = 11;
    EEPROM.update(1, value);
    //instruction to write all data to sd card
    myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
    //then, write on it
    if (myFile) {
      myFile = SD.open("data.txt", FILE_WRITE);
      myFile.println("241,0,0");
      myFile.close();
    }
  }

  if (t >= 21 && t < 22) {
    irsend.sendRaw(Signa2_1_1, sizeof(Signa2_1_1) / sizeof(int), khz); //Accensione modalità riscaldamento 22°C
    Serial.println("221,0,0"); //Riscaldamento a 22°C ON (22-1), Raffrescamento e deumidificazione off
    value = 21;
    EEPROM.update(1, value);
    //instruction to write all data to sd card
    myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
    //then, write on it
    if (myFile) {
      myFile = SD.open("data.txt", FILE_WRITE);
      myFile.println("221,0,0");
      myFile.close();
    }
  }


  if (t >= 22 && t < 25) {
    if (h > 60) {
      if (now.month() >= 9 || now.month() < 5) {  //from september to april dehumidification 22 Â°C on
        irsend.sendRaw(Signa3_1_1, sizeof(Signa3_1_1) / sizeof(int), khz); //Accensione modalità deumidifica a 22°C
        Serial.println("0,0,221"); //Riscaldamento e Raffrescamento off, deumidificazione a 22°C on (22-1)
        value = 31;
        EEPROM.update(1, value);
        //instruction to write all data to sd card
        myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
        //then, write on it
        if (myFile) {
          myFile = SD.open("data.txt", FILE_WRITE);
          myFile.println("0,0,221");
          myFile.close();
        }
      }
      else { //from may to august dehumidification 25 Â°C on
        irsend.sendRaw(Signa4_1_1, sizeof(Signa4_1_1) / sizeof(int), khz); //Dehumidification 25Â°C on
        Serial.println("0,0,251"); //Heating and Cooling off, Dehumidification 25Â°C on (25-1)
        value = 41;
        EEPROM.update(1, value);
        //instruction to write all data to sd card
        myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
        //then, write on it
        if (myFile) {
          myFile = SD.open("data.txt", FILE_WRITE);
          myFile.println("0,0,251");
          myFile.close();
        }
      }
    }

    if (h <= 60) {
      irsend.sendRaw(Signa3_0_0, sizeof(Signa3_0_0) / sizeof(int), khz); //Spegnimento PdC
      Serial.println("0,0,0"); //Riscaldamento, Raffrescamento e deumidificazione off
      value = 30;
      EEPROM.update(1, 30);
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("0,0,0");
        myFile.close();
      }
    }
  }


  if (t >= 25 && t < 26 ) {
    irsend.sendRaw(Signa5_1_1, sizeof(Signa5_1_1) / sizeof(int), khz); //Accensione modalità raffrescamento 25°C
    //EEPROM.write(1, value);
    Serial.println("0,251,0"); //Riscaldamento off, Raffrescamento a 25°C on (25-1), deumidificazione off
    value = 51;
    EEPROM.update(1, value);
    //instruction to write all data to sd card
    myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
    //then, write on it
    if (myFile) {
      myFile = SD.open("data.txt", FILE_WRITE);
      myFile.println("0,251,0");
      myFile.close();
    }
  }

  if (t >= 26) {
    irsend.sendRaw(Signa6_1_1, sizeof(Signa6_1_1) / sizeof(int), khz); //Accensione modalità raffrescamento 23°C
    //EEPROM.write(1, value);
    Serial.println("0,231,0"); //Riscaldamento off, Raffrescamento a 23°C on (23-1), deumidificazione off
    value = 61;
    EEPROM.update(1, value);
    //instruction to write all data to sd card
    myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
    //then, write on it
    if (myFile) {
      myFile = SD.open("data.txt", FILE_WRITE);
      myFile.println("0,231,0");
      myFile.close();
    }
  }
}

void IRcontrol() {
  DateTime now = rtc.now();
  Serial.print(int(now.year()));
  Serial.print(',');
  Serial.print(int(now.month()));
  Serial.print(',');
  Serial.print(int(now.day()));
  Serial.print(',');
  Serial.print(int(now.hour()));
  Serial.print(',');
  Serial.print(int(now.minute()));
  Serial.print(',');
  Serial.print(int(now.second()));
  Serial.print(',');
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print(h);
  Serial.print(",");
  Serial.print(t);
  Serial.print(",");

  //instruction to write all data to sd card
  myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
  //then, write on it
  if (myFile) {
    myFile = SD.open("data.txt", FILE_WRITE);
    myFile.print(int(now.year()));
    myFile.print(',');
    myFile.print(int(now.month()));
    myFile.print(',');
    myFile.print(int(now.day()));
    myFile.print(',');
    myFile.print(int(now.hour()));
    myFile.print(',');
    myFile.print(int(now.minute()));
    myFile.print(',');
    myFile.print(int(now.second()));
    myFile.print(',');
    myFile.print(h);
    myFile.print(',');
    myFile.print(t);
    myFile.print(',');
    myFile.close();
  }

  if (t < 21) {
    int value1 = EEPROM.read(1);
    //float value2 = EEPROM.read(2) / 100;
    //if (value1 != 11  || (value1 == 11 && value2 > t)) {
    if (value1 != 11) {
      irsend.sendRaw(Signal_1_1, sizeof(Signal_1_1) / sizeof(int), khz); //Heating 24Â°C ON
      value = 11;
      EEPROM.update(1, value);  //Why update and not write? http://www.arduino.cc/en/Reference/EEPROMUpdate
      //EEPROM.update(2, int(t * 100));
      Serial.println("241,0,0");  //Heating 24Â°C ON (24-1), Cooling and Dehumidification off
      //Serial.println(EEPROM.read(1)); //debug
      //Serial.println(value);
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("241,0,0");
        myFile.close();
      }
    }
    else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
       }
  }

  if (t >= 21 && t < 22) {
    int value1 = EEPROM.read(1);
    //float value2 = EEPROM.read(2) / 100;
    //if (value1 != 21  || (value1 == 21 && value2 > t)) {
      if (value1 != 21) {
      irsend.sendRaw(Signa2_1_1, sizeof(Signa2_1_1) / sizeof(int), khz); //Heating 22Â°C ON
      value = 21;
      EEPROM.update(1, value);
      //EEPROM.update(2, int(t * 100));
      Serial.println("221,0,0"); //Heating 22Â°C ON (22-1), Cooling and Dehumidification off
      //Serial.println(EEPROM.read(1)); //debug
      //Serial.println(value);
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("221,0,0");
        myFile.close();
      }
    }
  }

  if (t >= 22 && t < 25) {
    if (h > 60) {
      int value1 = EEPROM.read(1);
      if (now.month() >= 9 || now.month() < 5) {  //from september to april dehumidification 22 Â°C on
        //float value2 = EEPROM.read(2) / 100;
        //if (value1 != 31  || (value1 == 31 && value2 > t)) {
          if (value1 != 31) {
          irsend.sendRaw(Signa3_1_1, sizeof(Signa3_1_1) / sizeof(int), khz); //Dehumidification 22Â°C on
          value = 31;
          EEPROM.update(1, value);
          //EEPROM.update(2, int(t * 100));
          Serial.println("0,0,221"); //Heating and Cooling off, Dehumidification 22Â°C on (22-1)
          //Serial.println(EEPROM.read(1)); //debug
          //Serial.println(value);
          //instruction to write all data to sd card
          myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
          //then, write on it
          if (myFile) {
            myFile = SD.open("data.txt", FILE_WRITE);
            myFile.println("0,0,221");
            myFile.close();
          }
        }
       else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
     } 
      }
      else { //from may to august dehumidification 25 Â°C on
        //float value2 = EEPROM.read(2) / 100;
        //if (value1 != 41  || (value1 == 41 && value2 < t)) {
        if (value1 != 41) {
          irsend.sendRaw(Signa4_1_1, sizeof(Signa4_1_1) / sizeof(int), khz); //Dehumidification 25Â°C on
          value = 41;
          EEPROM.update(1, value);
          //EEPROM.update(2, int(t * 100));
          Serial.println("0,0,251"); //Heating and Cooling off, Dehumidification 25Â°C on (25-1)
          //Serial.println(EEPROM.read(1)); //debug
          //Serial.println(value);
          //instruction to write all data to sd card
          myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
          //then, write on it
          if (myFile) {
            myFile = SD.open("data.txt", FILE_WRITE);
            myFile.println("0,0,251");
            myFile.close();
          }
        }
        else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
     }
      }
    }
    if (h <= 60) {
      int value1 = EEPROM.read(1);
      //float value2 = EEPROM.read(2) / 100;
      //if (value1 != 30  || (value1 == 30 && value2 < t)) {
      if (value1 != 30) {
        irsend.sendRaw(Signa3_0_0, sizeof(Signa3_0_0) / sizeof(int), khz); //turn off Pdc
        value = 30;
        EEPROM.update(1, 30);
        //EEPROM.update(2, int(t * 100));
        Serial.println("0,0,0"); //Heating, Cooling and Dehumidification off
        //Serial.println(EEPROM.read(1)); //debug
        //Serial.println(value);

        //instruction to write all data to sd card
        myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
        //then, write on it
        if (myFile) {
          myFile = SD.open("data.txt", FILE_WRITE);
          myFile.println("0,0,0");
          myFile.close();
        }
      }
      else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
     }
    }
  }

  if (t >= 25 && t < 26 ) {
    int value1 = EEPROM.read(1);
    //float value2 = EEPROM.read(2) / 100;
    //if (value1 != 51  || (value1 == 51 && value2 < t)) {
      if (value1 != 51) {
      irsend.sendRaw(Signa5_1_1, sizeof(Signa5_1_1) / sizeof(int), khz); //Cooling 25Â°C on
      value = 51;
      EEPROM.update(1, value);
      //EEPROM.update(2, int(t * 100));
      Serial.println("0,251,0"); //Heating off, Cooling 25Â°C on (25-1), Dehumidification off
      //Serial.println(EEPROM.read(1)); //debug
      //rial.println(value);
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("0,251,0");
        myFile.close();
      }
    }
    else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
     }
  }

  if (t >= 26) {
    int value1 = EEPROM.read(1);
    //float value2 = EEPROM.read(2) / 100;
    //if (value1 != 61  || (value1 == 61 && value2 < t)) {
      if (value1 != 61) {
      irsend.sendRaw(Signa6_1_1, sizeof(Signa6_1_1) / sizeof(int), khz); //Cooling 23Â°C on
      value = 61;
      EEPROM.update(1, value);
      //EEPROM.update(2, int(t * 100));
      Serial.println("0,231,0"); //Heating off, Cooling 23Â°C on (23-1), Dehumidification off
      //Serial.println(EEPROM.read(1)); //debug
      //Serial.println(value);
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("0,231,0");
        myFile.close();
      }
    }
    else{
      Serial.println("aa,aa,aa"); //"aa" = as above
      //instruction to write all data to sd card
      myFile = SD.open("data.txt", FILE_WRITE); //first open the file.
      //then, write on it
      if (myFile) {
        myFile = SD.open("data.txt", FILE_WRITE);
        myFile.println("aa,aa,aa");
        myFile.close();
      }
     }
  }
}
