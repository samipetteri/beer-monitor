/*

*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define SSID "<ssid>"
#define PASS "<passwd>"
#define IP "<targetip>"

int oneWireInput = D3;

int pirInput = D2;
int pirState = LOW;
int val = 0;

OneWire oneWire(oneWireInput);
DallasTemperature sensors(&oneWire);

WiFiClient client;
PubSubClient mqttClient(client);

char msg[10];

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  pinMode(pirInput, INPUT);    

  
  connectWifi();


  Serial.println("Starting sensors");
  sensors.begin();
  Serial.println("Setup done.");
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void connectWifi() {
  WiFi.begin(SSID, PASS);

  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

int counter = 0;
int skips = 30;

void loop() {  
  counter++;

  mqttClient.setServer(IP, 1883);

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  // Movement
  val = digitalRead(pirInput);
  if (val == HIGH && pirState == LOW) {
    Serial.println("Motion detected!");      
    pirState = HIGH;
  } else if (val == LOW && pirState == HIGH) {
    Serial.println("Motion ended!");
    pirState = LOW;
  }

  String(pirState).toCharArray(msg, 10);
  mqttClient.publish("koti/liike/kalja", msg);

  // temperature, skip some because of uselessness of data
  if (counter % skips == 0) {
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
  
    Serial.print("Temperature for the device 1 (index 0) is: ");
    float temp0 = sensors.getTempCByIndex(0);
    Serial.println(temp0);
    
    Serial.print("Temperature for the device 2 (index 1) is: ");
    float temp1 = sensors.getTempCByIndex(1);
    Serial.println(temp1);

    Serial.println("publishing temps");
    
    String(temp1).toCharArray(msg, 10);
    mqttClient.publish("koti/lammot/kalja2", msg);

    String(temp0).toCharArray(msg, 10);    
    mqttClient.publish("koti/lammot/kalja1", msg);

  }  
  
  delay(1000);
}

