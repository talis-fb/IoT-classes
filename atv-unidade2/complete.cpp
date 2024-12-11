#include <WiFi.h>
#include <PubSubClient.h>

#include "FS.h"
#include "SPIFFS.h"

// Sensores...
#include "DHTesp.h"

// Sensor 1
const int DHT_PIN_1 = 15;
DHTesp dhtSensor1;
#define TOPIC_SENSOR1 "JoaCuscuz/feeds/temp1"

// Sensor 2
const int DHT_PIN_2 = 16;
DHTesp dhtSensor2;
#define TOPIC_SENSOR2 "JoaCuscuz/feeds/temp2"

// Media
#define TOPIC_SENSOR_MEDIA "JoaCuscuz/feeds/tempmean"


// Update these with values suitable for your network.
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "io.adafruit.com";
const char* mqtt_user = "JoaCuscuz";
const char* mqtt_password = "aio_****";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  int tries = 3;
  while (!client.connected() && tries > 0) {
    tries--;
    Serial.print("Attempting MQTT connection... left try:" + String(tries));
    
    // Create a random client ID
    String clientId = "ESP8266Client-";

    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void saveDataToSPIFFS(String topic, String data) {
  File file = SPIFFS.open("/data.log", FILE_APPEND);
  if (!file) {
    Serial.println("[ERROR] Failed to open file for writing");
    return;
  }
  String line = topic + "," + data;
  file.println(line);
  file.close();
  Serial.println("Data saved to SPIFFS -> '" + line  + "'");
}

void sendStoredData() {
  File file = SPIFFS.open("/data.log", FILE_READ);
  if (!file) {
    Serial.println("No saved data to send");
    return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    int separatorIndex = line.indexOf(',');
    if (separatorIndex > 0) {
      String topic = line.substring(0, separatorIndex);
      String data = line.substring(separatorIndex + 1);
      if (client.publish(topic.c_str(), data.c_str())) {
        Serial.println("Resent: " + topic + " -> " + data);
      } else {
        Serial.println("[ERROR] Failed to resend: " + line);
        break;
      }
    }
  }
  file.close();
  SPIFFS.remove("/data.log");
}

void setup() {
  Serial.begin(115200);

  delay(200);

  if (!SPIFFS.begin(true)) {
    Serial.println("[ERROR] An error occurred while mounting SPIFFS");
    return;
  }
  Serial.println("[OK] SPIFFS Mounted");

  dhtSensor1.setup(DHT_PIN_1, DHTesp::DHT22);
  dhtSensor2.setup(DHT_PIN_2, DHTesp::DHT22);

  delay(200);

  setup_wifi();
  Serial.println("[OK] WIFI connected");

  client.setServer(mqtt_server, 1883);
  Serial.println("[OK] Adafruit connected");
}

void loop() {
  TempAndHumidity  data_sensor1 = dhtSensor1.getTempAndHumidity();
  TempAndHumidity  data_sensor2 = dhtSensor2.getTempAndHumidity();
  
  String temp1 = String(data_sensor1.temperature);
  Serial.println("Temp sensor 1: " + temp1 + "°C");

  String temp2 = String(data_sensor2.temperature);
  Serial.println("Temp sensor 2: " + temp2 + "°C");

  int mean = (float)(data_sensor1.temperature + data_sensor2.temperature) / (float) 2;
  String temp_media = String(mean);
  Serial.println("Temp media   : " + temp_media + "°C");

  if (!client.connected()) {
    saveDataToSPIFFS(TOPIC_SENSOR1, temp1);
    saveDataToSPIFFS(TOPIC_SENSOR2, temp2);
    saveDataToSPIFFS(TOPIC_SENSOR_MEDIA, temp_media);
    reconnect();
    return;
  }

  client.publish(TOPIC_SENSOR1, temp1.c_str());
  client.publish(TOPIC_SENSOR2, temp2.c_str());
  client.publish(TOPIC_SENSOR_MEDIA, temp_media.c_str());
  sendStoredData();

  /*
  if (!client.publish(TOPIC_SENSOR1, temp1.c_str())) {
    Serial.println("[ERROR] Failed to sending to " + String(TOPIC_SENSOR1));
    //saveDataToSPIFFS(TOPIC_SENSOR1, temp1);
  }
  if (!client.publish(TOPIC_SENSOR2, temp2.c_str())) {
    Serial.println("[ERROR] Failed to sending to " + String(TOPIC_SENSOR2));
    //saveDataToSPIFFS(TOPIC_SENSOR2, temp2);
  }
  if (!client.publish(TOPIC_SENSOR_MEDIA, temp_media.c_str())) {
    Serial.println("[ERROR] Failed to sending to " +  String(TOPIC_SENSOR_MEDIA));
    //saveDataToSPIFFS(TOPIC_SENSOR_MEDIA, temp_media);
  }
  */
  
  delay(10000);
}
