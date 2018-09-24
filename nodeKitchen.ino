#include <ShutterButton.h>
#include <Contact.h>
#include <Relay.h>
#include <CnC.h>
#include <OneWire.h>
#include <DallasTemperature.h>

ShutterButton windowShutterButton(String("kitchen_windowShutterButton"), 12, 11); // 6, 5);
Contact windowSensor_window(String("kitchen_windowSensor_window"), 10);
Contact windowSensor_shutter(String("kitchen_windowSensor_shutter"), 9);

ShutterButton doorShutterButton(String("kitchen_doorShutterButton"), A3, A2);
Contact doorSensor_window(String("kitchen_doorSensor_window"), A1);
Contact doorSensor_shutter(String("kitchen_doorSensor_shutter"), A0);

Relay light("kitchen_lightRelay", 13);
Relay entry("kitchen_entryRelay", 4);
Relay up("u", 6);
Relay down("d", 5);

OneWire oneWire(2);
DallasTemperature tempSensors(&oneWire);

void windowShutterButton_cmdGet(int arg_cnt, char **args) { windowShutterButton.cmdGet(arg_cnt, args); }
void windowSensor_window_cmdGet(int arg_cnt, char **args) { windowSensor_window.cmdGet(arg_cnt, args); }
void windowSensor_shutter_cmdGet(int arg_cnt, char **args) { windowSensor_shutter.cmdGet(arg_cnt, args); }
void doorShutterButton_cmdGet(int arg_cnt, char **args) { doorShutterButton.cmdGet(arg_cnt, args); }
void doorSensor_window_cmdGet(int arg_cnt, char **args) { doorSensor_window.cmdGet(arg_cnt, args); }
void doorSensor_shutter_cmdGet(int arg_cnt, char **args) { doorSensor_shutter.cmdGet(arg_cnt, args); }
void light_cmdGet(int arg_cnt, char **args) { light.cmdGet(arg_cnt, args); }
void light_cmdSet(int arg_cnt, char **args) { light.cmdSet(arg_cnt, args); }
void entry_cmdGet(int arg_cnt, char **args) { entry.cmdGet(arg_cnt, args); }
void entry_cmdSet(int arg_cnt, char **args) { entry.cmdSet(arg_cnt, args); }
uint8_t tempSensorsNb = 0;

void setup() {
  Serial.begin(115200);
  //oneWire.setPin(2);
  tempSensors.setOneWire(&oneWire);
  tempSensors.begin();
  DeviceAddress deviceAddress;
  if (tempSensors.getAddress(deviceAddress, 0)) tempSensors.setResolution(deviceAddress, 12);
  tempSensorsNb = tempSensors.getDeviceCount();
  
  cncInit();
  cncAdd("kitchen_windowShutterButton_get" , windowShutterButton_cmdGet);
  cncAdd("kitchen_windowSensor_window_get" , windowSensor_window_cmdGet);
  cncAdd("kitchen_windowSensor_shutter_get", windowSensor_shutter_cmdGet);
  cncAdd("kitchen_doorShutterButton_get" , doorShutterButton_cmdGet);
  cncAdd("kitchen_doorSensor_window_get" , doorSensor_window_cmdGet);
  cncAdd("kitchen_doorSensor_shutter_get", doorSensor_shutter_cmdGet);
  cncAdd("kitchen_light_cmdGet", light_cmdGet);
  cncAdd("kitchen_light_cmdSet", light_cmdSet);
  cncAdd("kitchen_entry_cmdGet", entry_cmdGet);
  cncAdd("kitchen_entry_cmdSet", entry_cmdSet);
  up.open();
  down.open();
}


void loop() {
  static uint32_t loopNb = 0;
  delay(1);
  windowShutterButton.run(false);
  doorShutterButton.run(false);
  light.run(false);
  entry.run(false);

  /* HK @ 1Hz */
  if(0 == loopNb%1000) {
    windowSensor_window.run(true);
    windowSensor_shutter.run(true);
    doorSensor_window.run(true);
    doorSensor_shutter.run(true);
    tempSensorsNb = tempSensors.getDeviceCount();
    tempSensors.requestTemperatures();
    for(uint8_t i=0; i<tempSensorsNb; i++)  {
      Serial.print("kitchen_tempSensors_hk ");
      Serial.print(i, DEC);
      Serial.print(" ");
      Serial.println(tempSensors.getTempCByIndex(i));
    }
  }
  cncPoll();
  loopNb++;
}

