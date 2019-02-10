#include <ShutterButton.h>
#include <Contact.h>
#include <Relay.h>
#include <CnC.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char nodeName[] PROGMEM = "kitchen";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "hk";
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

static uint32_t loopNb = 0;

void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, loopNb); }
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
}


void loop() {
  delay(1);
  doorShutterButton.run(false);
  lightRelay.run(false);
  entryRelay.run(false);

  /* HK @ 0.1Hz */
  if(0 == loopNb%10000) {
    windowWindowContact.run(true);
    windowShutterContact.run(true);
    doorWindowContact.run(true);
    doorShutterContact.run(true);
    tempSensors.begin();
    tempSensorsNb = tempSensors.getDeviceCount();
    tempSensors.requestTemperatures();
    for(uint8_t i=0; i<tempSensorsNb; i++)  {
      DeviceAddress sensorAddr;
      tempSensors.getAddress(sensorAddr, i);
      cnc_print_hk_temp_sensor(tempSensorsName, sensorAddr, tempSensors.getTempCByIndex(i));
    }
  }
  cncPoll();
  loopNb++;
  if(1000000000 <= loopNb) { loopNb = 0; }
}
