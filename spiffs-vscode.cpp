#include "SPIFFS.h"
#include "WiFi.h"

#define led1 13
#define led2 12
int ledsStatus[] = {0, 0};

enum Mode {
  JUNTOS = 0,
  ALTERNAR = 1,
};

// char* input_ssid=
struct ConfigInput {
  String wifi;
  String password;
  enum Mode modo;
  uint delay;
  bool estado_inicial;
};

struct ConfigInput globalConfigInput;

void setConfigByLine(String line) {
  int separadorIndex = line.indexOf('=');
  if(separadorIndex == -1)
    return;

  String key = line.substring(0, separadorIndex);
  String value = line.substring(separadorIndex + 1);

  if(key == "wifi") {
    globalConfigInput.wifi = value;
  } else if (key == "password") {
    globalConfigInput.password = value;
  } else if (key == "delay") {
    globalConfigInput.delay = value.toInt();
  } else if (key == "estado_inicial") {
    globalConfigInput.estado_inicial = value.toInt();
  } else if (key == "modo") {
    globalConfigInput.modo = (enum Mode) value.toInt();
  }
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("INIT!");

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  if (!SPIFFS.begin(true)) {
    Serial.println("Not able to mount SPIFFS");
    return;
  }

  File file = SPIFFS.open("/config");
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }

  Serial.println(" Content:");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.print("Line readed: ");
    Serial.println(line);
    setConfigByLine(line);
  }

  file.close();

  WiFi.mode(WIFI_STA);
  WiFi.begin(globalConfigInput.wifi, globalConfigInput.password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }


  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  WiFi.disconnect();

  if(globalConfigInput.modo == JUNTOS) {
    ledsStatus[0] = 0;
    ledsStatus[1] = 0;
  } else if(globalConfigInput.modo == ALTERNAR) {
    ledsStatus[0] = 0;
    ledsStatus[1] = 1;
  }

  if(globalConfigInput.estado_inicial == 1) {
    ledsStatus[0] = !ledsStatus[0];
    ledsStatus[1] = !ledsStatus[1];
  }

}



void loop(){
  digitalWrite(led1, ledsStatus[0]);
  digitalWrite(led2, ledsStatus[1]);

  delay(globalConfigInput.delay);

  ledsStatus[0] = !ledsStatus[0];
  ledsStatus[1] = !ledsStatus[1];
}
