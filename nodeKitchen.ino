#include <ShutterButton.h>
#include <Contact.h>
#include <Relay.h>
#include <Cmd.h>
#include <OneWire.h>
#include <DallasTemperature.h>

ShutterButton windowShutter(String("kitchen_windowShutter"), 12, 11); // 6, 5);
Contact windowSensor_window(9);
Contact windowSensor_shutter(10);

ShutterButton doorShutter(String("kitchen_doorShutter"), A2, A3);
Contact doorSensor_window(A0);
Contact doorSensor_shutter(A1);

Relay light(13);
Relay entry(4);
OneWire oneWire(2);
DallasTemperature tempSensors(&oneWire);

#define ONEWIRE_TEMP_SHUTTER 1

void hello(int arg_cnt, char **args)
{
  Serial.println("Hello world.");
}

void setup() {
  Serial.begin(115200);
  tempSensors.begin();

  cmdInit();
  //cmdAdd("kitchen_doorShutter", "Turn OFF power", doorShutter.cmd);
  cmdStart();
}

void loop() {
  delay(1000);
  doorShutter.run();
  
  tempSensors.requestTemperatures();
  cmdPoll();
  Serial.println("kitchen_doorShutter 1");
}

