#include "DHT.h"
#define DHTPIN 7 // DHT11 sensor is connected on pin 7
#define DHTTYPE DHT11 // Define DHT11 sensor

#include <Ethernet.h> // include ethernet shield library
#include <SPI.h>

#include <Servo.h> 

Servo myservo;  // create servo object to control a servo. A maximum of eight servo objects can be created 

int RELAY = A5;
int servoPin = 8;
int timer = 0;
int pos;
float humGround = 0.0;
float tempAir = 0.0;
float humAir = 0.0;

int groundHumiditySensorInputPin = 0; //analogRead 0 is the analog pin where A0 is connected
int scheduler = 20000;
int defaultDelay = 2000;
int id = 1; // id of the data that read by sensors

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//char serverName[] = "45.55.40.47/planten"; // server domain
IPAddress server(45,55,40,47);
//String ipServer = "45.55.40.47/planten";
int serverPort = 80; // server\"s port
char pageName[] = "/create"; // page on the server
IPAddress ip(192, 168, 0, 177); // Set the static IP address to use if the DHCP fails to assign
EthernetClient client;

int totalCount = 0; 
String jsonRequest = "";

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  pinMode(RELAY, OUTPUT);
  Serial.begin(9600);
  myservo.attach(servoPin);
  digitalWrite(RELAY,HIGH);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println(".....:::::Sensor Manager Started:::::.....");

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.println("Starting ethernet...");

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip); // try to congifure using IP address instead of DHCP:
  }

  delay(defaultDelay);
  Serial.println(F("connecting..."));  
  dht.begin();
}

void loop()
{
  pompOff();
  Serial.println();
  Serial.print("ID: ");
  Serial.println(id);

  jsonRequest = "{";

  delay(defaultDelay);
  luchtVochtTemp();

  delay(defaultDelay);
  grondMeting();

  postData();
  jsonRequest = "";

  id = id + 1;
  setSeedState(true);
  delay(scheduler);
  checkValues();
}

void grondMeting() {
  float firstSensor = analogRead(groundHumiditySensorInputPin);
  Serial.print("Ground Humidity: ");
  Serial.println(firstSensor);

  float sensor0P = 100.00 - ( ( 100.00 * firstSensor ) / 1023.00 );
  int sensorInt = (int) sensor0P;
  humGround = sensorInt;
  Serial.print("Ground Humidity Procent: "); //vochtigheid van water is max 80 procent
  Serial.println("");
  jsonRequest = jsonRequest + "\"ground_humidity\": [{ \"value\": " + sensorInt + "}]}";
  Serial.print(sensorInt);
  Serial.print("%");
  Serial.println();
}


void luchtVochtTemp() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds \"old\" (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  tempAir = t;
  humAir = h;
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  String airTemp = String((int)t, (unsigned char)DEC);
  String airHum = String((int)h, (unsigned char)DEC);

  float hif = dht.computeHeatIndex(f, h); // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(t, h, false); // Compute heat index in Celsius (isFahreheit = false)

  jsonRequest = jsonRequest + "\"temperature\": [{ \"value\": " + airTemp + ",\"unit\": \"C\"}], \"air_humidity\": [{ \"value\": " + airHum + "}],";

  Serial.print("Air Humidity: ");
  Serial.print(h);
  Serial.println(" %\t");
  Serial.print("Air Temperature: ");
  Serial.print(t);
  Serial.print("C ");
  Serial.print(f);
  Serial.println("F\t");
  Serial.print("Air Heat index: ");
  Serial.print(hic);
  Serial.print("C ");
  Serial.print(hif);
  Serial.println("F");
}

//Waardes om te bepalen hoelang er gepompt moet worden, moet nog naar worden gekeken en bepaald wat juist is
//aan het eind half uur pauze voordat er weer wordt gekeken
void checkValues() {
  int totalPompTijd = 0;
  if(humGround < 30){
      totalPompTijd += 15;         
    }
  if(humAir < 30){
      totalPompTijd += 5;
    }
  if(tempAir < 25){
      totalPompTijd += 5;
    }
    pompOn();
    delay(totalPompTijd * 1000);
    pompOff();
    Serial.println("humGround");
    Serial.println(humGround);
    Serial.println("humAir");
    Serial.println(humAir);
    Serial.println("tempAir");
    Serial.println(tempAir);
    delay(1800000);
  }


/**
 * Method to postData to Webservice
 */
void postData() {
  //String jsonRequest = "{\"temperature\": [{ \"value\": 6,\"unit\": \"C\", \"timestamp\": 1444498560}], \"air_humidity\": [{ \"value\": 70, \"timestamp\": 1444498560}], \"ground_humidity\": [{ \"value\": 80, \"timestamp\": 1444498560}]}";

  if (client.connect(server, serverPort)) { // REPLACE WITH YOUR SERVER ADDRESS
    Serial.println(F("Making HTTP request..."));
    client.println("POST /planten/create HTTP/1.1");
    client.println("Host: funda.ga"); // SERVER ADDRESS HERE TOO
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json charset=UTF-8"); 
    client.print("Content-Length: ");
    client.println(jsonRequest.length()); 
    client.println();
    Serial.println(jsonRequest);
    client.print(jsonRequest); 
    Serial.println("posted...");
  } 
  else {
    Serial.println("Connection failed to post");
    Serial.println("Disconnecting.");
    client.stop();
  }

  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }
}

void pompOn(){
  digitalWrite(RELAY, LOW);
}

void pompOff(){
  digitalWrite(RELAY, HIGH);
}

void setSeedState(bool state){
  if(!state){
    digitalWrite(servoPin,LOW);
  }
  if(state){
    digitalWrite(servoPin,HIGH);
    RotateServo();
  }
}

void RotateServo(){
  for(timer = 0; timer < 400; timer += 1) 
  {
    if (timer > 0 && timer < 120) // start rotating servo
    {
      myservo.write(timer);
      delay(15);
    }

    if (timer > 120 && timer < 130) // shake servo
    {
      for(pos = 60; pos < 160; pos += 5)
      {
        myservo.write(pos);
        delay(15);
      }
    }

    if (timer > 130 && timer < 400) // stop servo
    {
      myservo.write(0);
      delay(15);
    }

    if (timer == 400) // restart servo process
    {
      timer = 1;
    }
    setSeedState(false);
  }
}
