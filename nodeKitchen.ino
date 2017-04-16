#include <Shutter.h>
#include <Relay.h>
#include <Cmd.h>
#include <OneWire.h>
#include <DallasTemperature.h>

Shutter kitchenShutter(11, 12, 6, 5);
Relay light(13);
OneWire oneWire(2);
DallasTemperature tempSensors(&oneWire);
Relay entry(4);

void kitchenShutterUp(int arg_cnt, char **args) {
  kitchenShutter.up();
  Serial.println("kitchenShutter.up OK");
}

void kitchenShutterDown(int arg_cnt, char **args) {
  kitchenShutter.down();
  Serial.println("kitchenShutter.down OK");
}

void lightOn(int arg_cnt, char **args) {
  light.close();
  Serial.println("light.on OK");
}

void lightOff(int arg_cnt, char **args) {
  light.open();
  Serial.println("light.off OK");
}

void kitchenTemp(int arg_cnt, char **args) {
  Serial.print("kitchen.temp=");
  Serial.println(tempSensors.getTempCByIndex(0));
}

void verandaTemp(int arg_cnt, char **args) {
  Serial.print("veranda.temp=");
  Serial.println(tempSensors.getTempCByIndex(1));
}

void hovenTemp(int arg_cnt, char **args) {
  Serial.print("hoven.temp=");
  Serial.println(tempSensors.getTempCByIndex(2));
}

void powerKitchenTemp(int arg_cnt, char **args) {
  Serial.print("powerKitchen.temp=");
  Serial.println(tempSensors.getTempCByIndex(3));
}

void entryTemp(int arg_cnt, char **args) {
  Serial.print("entry.temp=");
  Serial.println(tempSensors.getTempCByIndex(4));
}

void entryOn(int arg_cnt, char **args) {
  entry.close();
  Serial.println("entry.on OK");
}

void entryOff(int arg_cnt, char **args) {
  entry.open();
  Serial.println("entry.off OK");
}

void setup() {
  Serial.begin(115200);
  tempSensors.begin();
  cmdInit();
  cmdAdd("kitchenShutter.up", "Open kitchen shutter", kitchenShutterUp);
  cmdAdd("kitchenShutter.down", "Close kitchen shutter", kitchenShutterDown);
  cmdAdd("light.on", "Power light ON", lightOn);
  cmdAdd("light.off", "Power light OFF", lightOff);
  cmdAdd("kitchen.temp", "Kitchen temperature", kitchenTemp);
  cmdAdd("veranda.temp", "Veranda temperature", verandaTemp);
  cmdAdd("hoven.temp", "Hoven temperature", hovenTemp);
  cmdAdd("powerKitchen.temp", "Power kitchen temperature", powerKitchenTemp);
  cmdAdd("entry.temp", "Entry temperature", entryTemp);
  cmdAdd("entry.on", "Entry light ON", entryOn);
  cmdAdd("entry.off", "Entry light OFF", entryOff);
}

void loop() {
  delay(1);
  kitchenShutter.run();
  tempSensors.requestTemperatures();
  cmdPoll();
}

