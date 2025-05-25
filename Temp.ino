#include <WiFi.h>  
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi and MQTT Configuration
const char* ssid = "WIFISSID GOES HERE"; // WIFI SSID
const char* password = "WIFI PASSWORD GOES HERE"; //WIFI PASSWORD
const char* mqtt_server = "IP FOR YOUR MQTT SERVER HERE";  // Raspberry Pi's IP
const int READ_INTERVAL_MIN = 30; // Interval in minutes for your device to report back outside of a publish message being received

// MQTT Topics
const char* control_topic = "home/sensor/control";
const char* temp_topic = "home/sensor/temperature";

// Temperature Sensor Setup
#define ONE_WIRE_BUS D8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // If message is "get_temp", publish temperature
  if (message == "get_temp") {
    publishTemperature();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(control_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  
  char tempString[8];
  dtostrf(tempC, 6, 2, tempString);
  
  client.publish(temp_topic, tempString);
  Serial.print("Published temperature: ");
  Serial.print(tempString);
  Serial.println(" °C");
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Optional: Auto-publish every 60 seconds
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > (READ_INTERVAL_MIN *60000)) {
    publishTemperature();
    lastPublish = millis();
  }
  
  delay(100);
}
