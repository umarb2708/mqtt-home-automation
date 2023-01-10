#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error "Board not found"
#endif
#include "auth.h"
#include <PubSubClient.h>

//Relays for switching appliances
/*
 * Appliance       +    NodeMCU    +    ESP-01  
 * ----------------+   pin + gpio  +  pin + gpio-------------
 * Light           |    
 */
#define IN1            14
#define IN2            12
#define IN3            13
#define IN4            0
#define BATT           A0
#define led            16

int sensorValue;          // Analog Output of Sensor
float calibration = 0.36; // Check Battery voltage using multimeter & add/subtract the value
int bat_percentage;

#define MSG_BUFFER_SIZE  (150)
//Public Topics
#define pub0 "hiraWheels/battery"
// Subscribed Topics
#define sub1 "hiraWheels/roboControl"

//#define sub2 "hiraWheels/control/right"
//#define sub3 "hiraWheels/control/forward"
//#define sub4 "hiraWheels/control/backward"




WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

char msg[MSG_BUFFER_SIZE];
int value = 0;
String newHostname="HIRA-WHEELS";
// Connecting to WiFi Router

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(newHostname.c_str());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

  }

  randomSeed(micros());
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void callback(char* topic, byte* payload, unsigned int length)
{
  digitalWrite(led, LOW);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  delay(10);
  digitalWrite(led, HIGH);
  if (strstr(topic, sub1))
  {
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    if ((char)payload[0] == "forward"){
      moveForward();
    } else if ((char)payload[0] == "backward"){
      moveBack();
    }else if ((char)payload[0] == "left"){
      moveLeft();
    }else if ((char)payload[0] == "right"){
      moveRight();
    }
  }
}

// Connecting to MQTT broker
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "HIRA-WHEELS";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str() , username, pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("HIRA-WHEELS", "Connected");
      sensorValue = analogRead(analogInPin);
      float voltage = (((sensorValue * 3.3) / 1024) * 2 + calibration);
      bat_percentage = mapfloat(voltage, 2.8, 4.2, 0, 100);
      client.publish(pub1, bat_percentage);
      // ... and resubscribe
      client.subscribe(sub1);
     //client.subscribe(sub4);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      digitalWrite(led, LOW);
      delay(1000);
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(1000);
      digitalWrite(led, HIGH);
      delay(2000);
    }
  }
}



void setup()
{

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(led, OUTPUT);
  
  digitalWrite(led, LOW);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Connected to MQTT server");

 client.setCallback(callback);
 digitalWrite(led, HIGH);
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

}
