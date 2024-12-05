//This library allows you to use the IR reciver
#include "IRremote.h"

//Define IR reciver pin
int IRReciverPin = 11;

#define ledIsOn 10
#define ledVolume 9
#define ledMacete 8

void setup()
{
  Serial.begin(9600);
  IrReceiver.begin(IRReciverPin);
  pinMode(ledIsOn, OUTPUT);
  pinMode(ledVolume, OUTPUT);
  pinMode(ledMacete, OUTPUT);
  Serial.println("Apareceu!");
}


bool isOn = false;
int volume = 0;

#define COMMAND_ON 0xA2
#define COMMAND_VOLUME_UP 0x02
#define COMMAND_VOLUME_DOWN 0x98

const int COMMANDS_MACETAO[] = { 0x30, 0x18, 0x7A, 0xA8 };
int index_next_command_macetao = 0;
const int SIZE_COMMANDS_MACETAO = 4;
bool hasMacetaoPressed = false;

int millisLastMacetao = 0;

void manageLeds() {
  digitalWrite(ledIsOn, isOn);
  analogWrite(ledVolume, volume);
  digitalWrite(ledMacete, hasMacetaoPressed);
}

void manageMacetao() {
  if(hasMacetaoPressed == false) {
    return;
  }

  const int FIVE_SECONDS = 5000;
  if((millis() - millisLastMacetao) > FIVE_SECONDS) {
    hasMacetaoPressed = false;
  }
}

void loop()
{

  manageMacetao();
  manageLeds();

  int irData;
  if (!IrReceiver.decode()) {
    return;
  }

  irData = IrReceiver.decodedIRData.command;
  IrReceiver.resume();

  Serial.print("Comando: ");
  Serial.print(irData, HEX);

  if (irData == COMMAND_ON) {
    isOn = !isOn;
    Serial.print(" | Turn ON/OFF: ");
    Serial.print(isOn);
  }

  if (irData == COMMAND_VOLUME_UP) {
    volume = min(255, volume + 4);
    Serial.print(" | Volume UP, new volume ");
    Serial.print(volume);
  }

  if (irData == COMMAND_VOLUME_DOWN) {
    volume = max(0, volume - 4);
    Serial.print(" | Volume DOWN, new volume ");
    Serial.print(volume);
  }

  if (irData == COMMANDS_MACETAO[index_next_command_macetao]) {
    index_next_command_macetao += 1;

    if(index_next_command_macetao >= SIZE_COMMANDS_MACETAO) {
      hasMacetaoPressed = true;
      millisLastMacetao = millis();
      index_next_command_macetao = 0;
      Serial.print(" | Macetão completed");
    } else {
      Serial.print(" | Macetão command");
    }
  } else {
    if (index_next_command_macetao > 0) {
      Serial.print(" | Macetão cancelled");
    }

    hasMacetaoPressed = false;
    index_next_command_macetao = 0;
  }

  Serial.println("");

}
