#include "DHT.h"
#define DHTPIN 7 // DHT11 sensor is connected on pin 7
#define DHTTYPE DHT11 // Define DHT11 sensor

#include <Ethernet.h>
#include <SPI.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10, 0, 0, 177 };
byte server[] = { 64, 233, 187, 99 }; // Google

EthernetClient client;

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(9600);
    dht.begin();
}

void loop()
{
  delay(2000);
  luchtVochtTemp();
  delay(1000);
  grondMeting();
}

void grondMeting(){
//analogRead 0 is de analog pin waar A0 staat 
 float firstSensor = analogRead(0);
   Serial.print("Vochtigheid: ");   
   Serial.println(firstSensor);
     
   float sensor0P = 100.00 - ( ( 100.00 * firstSensor ) / 1023.00 );
   int sensorInt = (int) sensor0P;
   Serial.print("Vochtigheid procent: "); //vochtigheid van water is max 80 procent  
   Serial.println(sensorInt);               // wait for a second
   
   secondSensor = digitalRead(0);
   Serial.print("TEMP/MOIS: ");   
   Serial.println(secondSensor);
  }

void luchtVochtTemp(){
  
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

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  }
