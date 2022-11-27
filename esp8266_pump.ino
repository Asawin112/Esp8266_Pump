#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
int ddht = 14; //D5
int relay = 12; //D6
String autopump = "on";

#define DHTTYPE DHT22

DHT dht(ddht, DHTTYPE);


const char* ssid = "______";
const char* password = "_______";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "___________________________";
const char* mqtt_username = "___________________________";
const char* mqtt_password = "___________________________";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");  /*
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
  digitalWrite(relay, HIGH); // relay ปิด
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
  if (autopump == "on") {
    if (h > 80) {
      digitalWrite(relay, LOW); //เปิด
    } else {
      digitalWrite(relay, HIGH); //ปิด
    }
  }
  int r = digitalRead(relay);
  String data = "{\"data\":{\"Humidity\": " + String(h) + ",\"Temperature\": " + String(t) + ",\"relay\": " + String(r) + "}}";
  Serial.println(data);
  data.toCharArray(msg , (data.length() + 1));
  client.publish("@shadow/data/update", msg);
  client.loop();
  delay(1000);
}

String getMsg(String topic_, String message_) {     
  if (topic_ == "@msg/relay") {
    if (message_ == "on") {                  //netpie["???"].publish("@msg/relay","on")
      digitalWrite(relay, LOW); //relay เปิด
      autopump = "off";
    } else if (message_ == "off") {          //netpie["???"].publish("@msg/relay","off")
      digitalWrite(relay, HIGH); //relay ปิด
      autopump = "on";
    }
  }
  return autopump;
}
