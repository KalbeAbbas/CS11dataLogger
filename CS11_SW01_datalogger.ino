/*
* Datalogger example using SW01 Weather Station 
* 
* The code here logs weather values (Temperature, Humidity and Pressure) to the 
* SD card attached to CS11.
* 
* Format SD card to FAT32 and create config.txt in the SD card.
* Paste this into config.txt file: {"filename":"<filename>"}
*/


#define Serial SerialUSB

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <xCore.h>
#include <xSW01.h>
#include <ArduinoJson.h>

xSW01 SW01; //SW01 Object

File myFile; //File object

String datafile;
const char *filename = "/config.txt";  // <- SD library uses 8.3 filenames

struct Config {
  char filename[64];
};

Config config;                         // <- global configuration object


//Function used to load filename stored in config.txt file
void loadConfiguration(const char *filename, Config &config) {
  File file = SD.open(filename);
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);

  if (!root.success())
    Serial.println("No config, using default");
  strlcpy(config.filename,
          root["filename"] | "def", //max 4 char long to make room for 4 digit seqno
          min(5, sizeof(config.filename)));
  file.close();
}

//Function to obtain weather measures and log into SD card. You can also see the measure in Serial monitor
void readWeather() {

    float tempC,hum,pressure;

    SW01.poll();
    
    tempC = SW01.getTempC();
    hum = SW01.getHumidity();
    pressure = SW01.getPressure();

    //Table data
    myFile.print(millis()); myFile.print(",");
    myFile.print(tempC); myFile.print(",");
    myFile.print(hum); myFile.print(",");
    myFile.print(pressure);
    myFile.println();

    Serial.print("Temperature(C): ");Serial.println(tempC);
    Serial.print("Humidity(%): ");Serial.println(hum);
    Serial.print("Pressure(Pa): ");Serial.println(pressure);
}


void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(200);

  SW01.begin();

  Serial.println("Setup:");
  Serial.println("Initializing");
  Serial.println("SD card...");

  //Begin SD card
  if (!SD.begin(3)) {
    Serial.println("SD card FAILED!");
    while (1);
  }
  
  Serial.println("Loading config");
  loadConfiguration(filename, config);
  Serial.println("done!");

  int i = 1;
  do {
    char seqno[5];
    sprintf(seqno, "%04d", i++);
    datafile = String(config.filename) + String(seqno) + ".csv";
  } while (SD.exists(datafile));

  myFile = SD.open(datafile, FILE_WRITE);
  
  if (myFile) {
    //Table Header
    myFile.print("Time"); myFile.print(",");
    myFile.print("Temperature(C)"); myFile.print(",");
    myFile.print("Humidity(%)"); myFile.print(",");
    myFile.println("Pressure(Pa)");
    myFile.close();
    Serial.print("Writing to:");
    Serial.print(datafile);
  } else {
    Serial.println("Error creating");
    Serial.println(datafile);
    while (true);
  }
  Serial.println("Starting in 1 sec");
  delay(1000);
}

void loop() {
  myFile = SD.open(datafile, O_RDWR | O_APPEND);
  if (myFile) {
    readWeather();
    myFile.close();
  } else {
    Serial.println("Error opening");
    Serial.println(datafile);
  }
  delay(1);
}
