#include <Arduino.h>
#include <UbxGpsNavPvt.h>
#include <math.h>


#define GPS_BAUDRATE 115200L
#define PC_BAUDRATE 115200L


UbxGpsNavPvt<HardwareSerial> gps(Serial2);


const double pi = 3.14159265358979323;
const double radius_earth = 6371000.0;

double lat = 0.0;
double lon = 0.0;

double h1 = 0.0;
double h_t = 0.0;
double delta_h = 0.0;

double lat_t = 31.8349561;
double lon_t = 54.3539401;

float dis = 0.0;

unsigned long int t1 = 0;
int interval = 1000;

void setup()
{
  Serial.begin(PC_BAUDRATE);
  gps.begin(GPS_BAUDRATE);
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
  lat1 = radians(lat1);
  lon1 = radians(lon1);
  lat2 = radians(lat2);
  lon2 = radians(lon2);

  double dlon = lon2 - lon1;
  double dlat = lat2 - lat1;
  double a = pow(sin(dlat/2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon/2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  double distance = c * radius_earth;
  return distance;
}

void loop()
{
  if (gps.ready())
  {
    lon = gps.lon / 10000000.0;

    lat = gps.lat / 10000000.0;

    h1 = 90 - gps.heading / 100000.0;

    if (h1 > 180)
    {
      h1 = -360 + h1;
    }
    else 
    {
      h1 = h1;
    }

    h_t = degrees(atan2(lat_t - lat, lon_t - lon));

    dis = haversine_distance(lat, lon,lat_t, lon_t);
  }

  if (millis() - t1 > interval)
  {
    t1 = millis();

    Serial.print(lon, 7);
    Serial.print(", ");
    Serial.print(lat, 7);
    Serial.print(", ");
    Serial.print(h1, 2);
    Serial.print(", ");
    Serial.print(h_t, 2);
    Serial.print(", ");
    Serial.print(h1 - h_t, 2);
    Serial.print(", ");
    Serial.print(dis, 2);
    Serial.println();
  }
}