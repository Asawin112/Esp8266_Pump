#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
int ddht = 14; //D5
int relay = 12; //D6

#define DHTTYPE DHT22 //---> หรือ DHT11

DHT dht(ddht, DHTTYPE);

const char* ssid = "aomsin_lab";
const char* password = "123456789";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "503a8713-d146-4748-b2cd-3bf4f38ec581";
const char* mqtt_username = "Qw3No6NMMuoua9qQs33PjoLsGdZaYFRi";
const char* mqtt_password = "C-TAxvXqP6GcS8JOmg~9magyyAx7aSs~";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[255];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  tpc = getMsg(topic, message);
}

void setup() {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  Serial.begin(115200);
  dht.begin();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (h > 80) {
    digitalWrite(relay, LOW);
  } else {
    digitalWrite(relay, HIGH);
  }
  int r = digitalRead(relay);
  client.loop();
  String data = "{\"data\":{\"Humidity\": " + String(h) + ",\"Temperature\": " + String(t) + ",\"relay\": " + String(r) + "}}";
  data.toCharArray(msg , (data.length() + 1));
  client.publish("@shadow/data/update", msg);
  delay(1000);
}
   
String getMsg(String topic_, String message_) {     //datasources["???"]["shadow"]["Relay1"]     Indicator Light : datasources["???"]["shadow"]["Relay1"]=='1'
  if (topic_ == "@msg/relay") {
    Serial.println("kk");
    if (message_ == "on") {                  //netpie["???"].publish("@msg/relay","on")
      digitalWrite(relay, LOW);
      Serial.print(" on");
    } else if (message_ == "off") {          //netpie["???"].publish("@msg/relay","off")
      digitalWrite(relay, HIGH); 
      Serial.println(" off");
    }
  }
}
