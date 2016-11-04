#include <SPI.h>
#include <Ethernet.h>


//network variables
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server(192, 168, 1, 100);
IPAddress ip(192, 168, 0, 177);

EthernetClient client;
//end network variables



//sensor variables
const String SENSOR_HASH = "c4ca4238a0b923820dcc509a6f75849b";

const int SEND_DATA_TIMER = 5;
const int CONNECTED_SENSORS[] = {A0,A1};
const int NUMBER_OF_SENSORS = sizeof(CONNECTED_SENSORS) / sizeof(sizeof(int));
const int SENSOR_SAMPLE_TIME = 1000;

int sensorIds[1];

int mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

double totalUsage[sizeof(CONNECTED_SENSORS)];  //array that will keep total usage of each sensor for SEND_DATA_TIMER time.

int numOfReadings = 1; //counts number of readings of the sensor
//end sensor variables



void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  connect();
  getUserData();
  
}

void loop() {

  if (numOfReadings++ >= SEND_DATA_TIMER) {
    postMeasurement();
    numOfReadings = 1;
  }

  //Loops through all registered sensors and reads the values of each and adds the value to total of the sensor.
  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    totalUsage[i] += getCurrentSensorValue(CONNECTED_SENSORS[i]);
    Serial.println(totalUsage[i]);
  }


}

void connect() {
   if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  Serial.println("connecting...");
  delay(2000);
}


void postMeasurement() {
  Serial.println("Sending data");
  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    if (client.connect(server, 80)) {
      Serial.println("Sending data connected");
      String request = "GET /diploma/php/store_data.php?";
      request += "value=";
      char str[80];
      dtostrf(totalUsage[i], 0, 3, str);
      request += str;
      request += "&";
      sprintf(str, "idSensor=%i",  sensorIds[i]);
      request += str;
      request += " HTTP/1.1";
      client.println(request);
      client.println("Host: 192.168.1.100");
      client.println("Connection: close");
      client.println();
      client.stop();
    } else {
     // connect();
    }

  }
  for(int i = 0;i < NUMBER_OF_SENSORS;i++) {
    totalUsage[0] = 0;
    }
}

boolean getUserData() {
  if (client.connect(server, 80)) {
    Serial.println("connected");
    String request = "GET /diploma/php/getSensorIdsArduino.php?hash=" + SENSOR_HASH + " HTTP/1.1";
    client.println(request);
    client.println("Host: 192.168.1.100");
    client.println("Connection: close");
    client.println();
    delay(1000);
  }

  boolean startReading = false;
  int numOfChar = 0;
  String tmp = "";


  while (client.available()) {
    char c = client.read();

    if (c == '#') {
      numOfChar++;
      if (numOfChar == 3 && !startReading) {
        startReading = true;
      } else if (startReading = true && c == '#') {
        startReading = false;
      }
    } else {
      numOfChar = 0;
    }
    if (startReading) {
      tmp += c;
    }
  }

  splitStringIds(tmp.substring(1, tmp.length() - 1));
  client.stop();
  
}


float getVPP(int sensor)
{
  float result;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < SENSOR_SAMPLE_TIME) //sample for 1 Sec
  {
    readValue = analogRead(sensor);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5.0) / 1024.0;

  return result;
}

double getCurrentSensorValue(int sensor) {
  Voltage = getVPP(sensor);
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;

  return AmpsRMS - 0.1;

}



int splitStringIds(String inputString) {
  String tmp = "";
  int tmpCount = 0;
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString[i] != ';') {
      tmp += inputString[i];
    } else {
      sensorIds[tmpCount++] = tmp.toInt();
      tmp = "";
    }


  }
  sensorIds[tmpCount++] = tmp.toInt();

}



void printIntArray(int tmp[]) {
  Serial.print("Printing array: ");
  for(int i = 0;i< sizeof(tmp);i++) {
    Serial.print(tmp[i]);
    Serial.print(", ");
    }
    Serial.println();
  }

  void printStringArray(String tmp[]) {
  Serial.print("Printing array: ");
  for(int i = 0;i< sizeof(tmp);i++) {
    Serial.print(tmp[i]);
    Serial.print(", ");
    }
    Serial.println();
  }






