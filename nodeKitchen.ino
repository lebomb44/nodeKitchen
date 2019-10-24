#include <ShutterButton.h>
#include <Contact.h>
#include <Relay.h>
#include <CnC.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char nodeName[] PROGMEM = "kitchen";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "val";
const char cmdGetName[] PROGMEM = "get";
const char cmdSetName[] PROGMEM = "set";

const char pingName[] PROGMEM = "ping";
const char windowShutterButtonName[] PROGMEM = "windowShutterButton";
const char windowShutterUpRelayName[] PROGMEM = "windowShutterUpRelay";
const char windowShutterDownRelayName[] PROGMEM = "windowShutterDownRelay";
const char windowWindowContactName[] PROGMEM = "windowWindowContact";
const char windowShutterContactName[] PROGMEM = "windowShutterContact";
const char doorShutterButtonName[] PROGMEM = "doorShutterButton";
const char doorWindowContactName[] PROGMEM = "doorWindowContact";
const char doorShutterContactName[] PROGMEM = "doorShutterContact";
const char lightRelayName[] PROGMEM = "lightRelay";
const char entryRelayName[] PROGMEM = "entryRelay";
const char tempSensorsName[] PROGMEM = "tempSensors";

ShutterButton windowShutterButton(windowShutterButtonName, 12, 11);
Relay windowShutterUpRelay(windowShutterUpRelayName, 6);
Relay windowShutterDownRelay(windowShutterDownRelayName, 5);
Contact windowWindowContact(windowWindowContactName, 10);
Contact windowShutterContact(windowShutterContactName, 9);

ShutterButton doorShutterButton(doorShutterButtonName, A3, A2);
Contact doorWindowContact(doorWindowContactName, A1);
Contact doorShutterContact(doorShutterContactName, A0);

Relay lightRelay(lightRelayName, 13);
Relay entryRelay(entryRelayName, 4);

OneWire oneWire(2);
DallasTemperature tempSensors(&oneWire);

uint32_t previousTime_1s = 0;
uint32_t previousTime_10s = 0;
uint32_t currentTime = 0;

void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, currentTime); }
void windowWindowContact_cmdGet(int arg_cnt, char **args) { windowWindowContact.cmdGet(arg_cnt, args); }
void windowShutterContact_cmdGet(int arg_cnt, char **args) { windowShutterContact.cmdGet(arg_cnt, args); }
void doorShutterButton_cmdGet(int arg_cnt, char **args) { doorShutterButton.cmdGet(arg_cnt, args); }
void doorWindowContact_cmdGet(int arg_cnt, char **args) { doorWindowContact.cmdGet(arg_cnt, args); }
void doorShutterContact_cmdGet(int arg_cnt, char **args) { doorShutterContact.cmdGet(arg_cnt, args); }
void lightRelay_cmdGet(int arg_cnt, char **args) { lightRelay.cmdGet(arg_cnt, args); }
void lightRelay_cmdSet(int arg_cnt, char **args) { lightRelay.cmdSet(arg_cnt, args); }
void entryRelay_cmdGet(int arg_cnt, char **args) { entryRelay.cmdGet(arg_cnt, args); }
void entryRelay_cmdSet(int arg_cnt, char **args) { entryRelay.cmdSet(arg_cnt, args); }
uint8_t tempSensorsNb = 0;

void setup() {
  Serial.begin(115200);
  cncInit(nodeName);
  cnc_hkName_set(hkName);
  cnc_cmdGetName_set(cmdGetName);
  cnc_cmdSetName_set(cmdSetName);
  cnc_sepName_set(sepName);
  cnc_cmdGet_Add(pingName, ping_cmdGet);
  cnc_cmdGet_Add(windowWindowContactName , windowWindowContact_cmdGet);
  cnc_cmdGet_Add(windowShutterContactName, windowShutterContact_cmdGet);
  cnc_cmdGet_Add(doorShutterButtonName, doorShutterButton_cmdGet);
  cnc_cmdGet_Add(doorWindowContactName, doorWindowContact_cmdGet);
  cnc_cmdGet_Add(doorShutterContactName, doorShutterContact_cmdGet);
  cnc_cmdGet_Add(lightRelayName, lightRelay_cmdGet);
  cnc_cmdSet_Add(lightRelayName, lightRelay_cmdSet);
  cnc_cmdGet_Add(entryRelayName, entryRelay_cmdGet);
  cnc_cmdSet_Add(entryRelayName, entryRelay_cmdSet);
  windowShutterUpRelay.open();
  windowShutterDownRelay.open();
  previousTime_1s = millis();
  previousTime_10s = previousTime_1s;
}

void loop() {
  currentTime = millis(); cncPoll();
  /* HK @ 2.0Hz */
  if((uint32_t)(currentTime - previousTime_1s) >= 500) {
    windowWindowContact.run(true); cncPoll();
    windowShutterContact.run(true); cncPoll();
    doorWindowContact.run(true); cncPoll();
    doorShutterContact.run(true); cncPoll();
    doorShutterButton.run(true); cncPoll();
    lightRelay.run(true); cncPoll();
    entryRelay.run(true); cncPoll();
    previousTime_1s = currentTime;
  }
  /* HK @ 0.1Hz */
  if((uint32_t)(currentTime - previousTime_10s) >= 10000) {
    tempSensors.begin(); cncPoll();
    tempSensorsNb = tempSensors.getDeviceCount(); cncPoll();
    tempSensors.requestTemperatures(); cncPoll();
    for(uint8_t i=0; i<tempSensorsNb; i++)  {
      DeviceAddress sensorAddr;
      tempSensors.getAddress(sensorAddr, i); cncPoll();
      float temp_ = tempSensors.getTempCByIndex(i);
      if(DEVICE_DISCONNECTED_C != temp_) {
        cnc_print_hk_temp_sensor(tempSensorsName, sensorAddr, temp_); cncPoll();
      }
    }
    previousTime_10s = currentTime;
  }
}
