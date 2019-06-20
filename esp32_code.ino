#include <Adafruit_MQTT.h>            //Adafruit MQTT Libraries
#include <Adafruit_MQTT_Client.h>     //
#include <WiFi.h>              //ESP8266 Wi-Fi libraries
#include "DHT.h"

#define DHTPIN A13   //esp32 G15 pin

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
int sensorValue;
#define lightPin 16
#define fanPin 17

#define WLAN_SSID "shuvo"          //enter your WiFi network name within the double quotes & 
#define WLAN_PASS "123456789"          //password here

#define AIO_SERVER       "io.adafruit.com"
#define AIO_SERVERPORT   1883               // use 8883 is used for SSL
#define AIO_USERNAME     "shuvo_podder"       //Username of Adafruit IO (for this click the Golden Key in Adafruit IO Dashboard)
#define AIO_KEY          "bea57cbc07694953bcd7c94b34dec7da"       //key of Adafruit IO (for this click the Golden Key in Adafruit IO Dashboard)

WiFiClient client;                                                                                  // Create an ESP8266 WiFiClient class to connect to the MQTT server.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);              // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led_test");  // Setup a feed called 'onoff' for subscribing to changes.
void MQTT_connect();
void setup()
{
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);

  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");       // Connect to WiFi access point.
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.subscribe(&onoffbutton);

  // Setup MQTT subscription for onoff feed.

  dht.begin();
}

uint32_t x = 0;
void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      String response = (char*)onoffbutton.lastread; //converts the received 1 or 0 to string to compare in the if-else statement
      Serial.println(response);
      if (response == "ON")
      {
        digitalWrite(lightPin, HIGH); //LED Glows
      }
      else if (response == "OFF")
      {
        digitalWrite(lightPin, LOW); //LED off
      }
      else if (response == "f_on")
      {
        digitalWrite(fanPin, HIGH); //LED off
      }
      else if (response == "f_off")
      {
        digitalWrite(fanPin, LOW); //LED off
      }
      else if (response == "auto")
      {
        AutoMode();
      }
    }
  }
}
void AutoMode()
{



  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription != &onoffbutton)
  {
Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);

    
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  if (t > 30)
  {
    digitalWrite(fanPin, HIGH);
  }
  else {
    digitalWrite(fanPin, LOW);
  }


  sensorValue = analogRead(A0); // read analog input pin 0
  Serial.print(sensorValue, DEC); // prints the value read
  Serial.print(" \n"); // prints a space between the numbers
  delay(1000);

  if (sensorValue < 10)
  {
    digitalWrite(lightPin, HIGH);
  } else {
    digitalWrite(lightPin, LOW);
  }
    
  }

  loop();

  //AutoMode();

}
void MQTT_connect()
{
  int8_t ret;
  if (mqtt.connected())     // Stop if already connected.
  {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));                  // connect will return 0 for connected
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);              // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      while (1);              // basically die and wait for WDT to reset me
    }
  }
  Serial.println("MQTT Connected!");
}
