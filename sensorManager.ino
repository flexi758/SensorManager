#include "DHT.h"
#define DHTPIN 7 // DHT11 sensor is connected on pin 7
#define DHTTYPE DHT11 // Define DHT11 sensor

#include <Ethernet.h> // include ethernet shield library
#include <SPI.h>

int groundHumiditySensorInputPin = 0; //analogRead 0 is the analog pin where A0 is connected
int defaultDelay = 2000;
int id = 1; // id of the data that read by sensors

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char serverName[] = "www.google.com"; // server domain
int serverPort = 80; // server's port
char pageName[] = "/create"; // page on the server
IPAddress ip(192, 168, 0, 177); // Set the static IP address to use if the DHCP fails to assign

EthernetClient client;

int totalCount = 0; 
char params[100]; // insure params is big enough to hold your variables

// set this to the number of milliseconds delay
// this is 30 seconds
#define delayMillis 30000UL
unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(9600);
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
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  delay(defaultDelay);
  Serial.println(F("connecting..."));
  
  // if you get a connection, report back via serial:
  if (client.connect(serverName, serverPort)) {
    Serial.println("connected");
    // Make a HTTP request:
    //client.println("GET /search?q=arduino HTTP/1.1");
    //client.println("Host: www.google.com");
    //client.println("Connection: close");
    //client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    delay(defaultDelay);
    exit(0); // Exit the program
  }
  
  dht.begin();
}

void loop()
{
  Serial.println();
  Serial.print("ID: ");
  Serial.println(id);

  delay(defaultDelay);
  luchtVochtTemp();

  delay(defaultDelay);
  grondMeting();

  id = id + 1;
}

void grondMeting() {
  float firstSensor = analogRead(groundHumiditySensorInputPin);
  Serial.print("Ground Humidity: ");
  Serial.println(firstSensor);

  float sensor0P = 100.00 - ( ( 100.00 * firstSensor ) / 1023.00 );
  int sensorInt = (int) sensor0P;
  Serial.print("Ground Humidity Procent: "); //vochtigheid van water is max 80 procent
  Serial.print(sensorInt);
  Serial.print("%");
  Serial.println();
}

void luchtVochtTemp() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

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
