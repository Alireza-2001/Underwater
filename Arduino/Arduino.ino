#include <ArduinoJson.h>

const unsigned long serial_monitor_baudrate = 115200;
const unsigned long raspi_serial_baudrate = 115200;


DynamicJsonDocument joystick(1024);
DynamicJsonDocument setting(1024);


void setup()
{
  Serial.begin(serial_monitor_baudrate);
  Serial1.begin(raspi_serial_baudrate);
}

void get_serial_data()
{
  if (Serial1.available() > 0)
  {
    String tmp = Serial1.readStringUntil('>');
    if (tmp.length() == 59)
    {
      deserializeJson(joystick, tmp);

    }
    else if (tmp.length() == 12)
    {
      deserializeJson(setting, tmp);

    }
  }
  else
  {

  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
