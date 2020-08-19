// ardunino code for temperature reader
// based on https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806

#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup(void)
{
 Serial.begin(9600);
 Serial.println("Dallas Temperature IC continuos sensor");
 sensors.begin();
}

void loop(void)
{
 sensors.requestTemperatures(); // Send the command to get temperature readings
 Serial.println(sensors.getTempCByIndex(0));
 delay(1000);
}
