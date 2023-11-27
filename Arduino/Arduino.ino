#include <Arduino.h>
#include <ArduinoJson.h>
#include <UbxGpsNavPvt.h>
#include <math.h>


#define GPS_BAUDRATE 115200L
#define PC_BAUDRATE 115200L


#define ROV 1
#define AUV 2
#define JYRO 3
#define RESET 4

#define ROV_RUN 10
#define ROV_STOP 11

#define AUV_1 20
#define AUV_2 21
#define AUV_STOP 22

#define JYRO_ENABLE 30
#define JYRO_DISABLE 31

DynamicJsonDocument joystick(1024);
DynamicJsonDocument setting(1024);

UbxGpsNavPvt<HardwareSerial> gps(Serial2);

double points_lat [30];
double points_lon [30];
int points_time [30];
int current_point = 0;



const double pi = 3.14159265358979323;
const double radius_earth = 6371000.0;

int state = RESET;
int rov_state = 0;
int auv_state = 0;
int auv1_run = 0;
int jyro_state = 0;

double lat = 0.0;
double lon = 0.0;

double h1 = 0.0;
double h_t = 0.0;
double delta_h = 0.0;

double lat_t = 31.8349561;
double lon_t = 54.3539401;

float dis = 0.0;


void setup()
{
  Serial.begin(PC_BAUDRATE);
  gps.begin(GPS_BAUDRATE);

  start_menu();
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

void update_gps()
{
  if (gps.ready())
  {
    lon = gps.lon / 10000000.0;

    lat = gps.lat / 10000000.0;

    h1 = 90 - gps.heading / 100000.0;
  }
}


unsigned long int t1 = 0;
int interval = 1000;

void AUV1_func()
{
  if (millis() - t1 > interval)
  {
    t1 = millis();

    if (h1 > 180)
    {
      h1 = -360 + h1;
    }
    else 
    {
      h1 = h1;
    }

    h_t = degrees(atan2(points_lat[current_point] - lat, points_lon[current_point] - lon));

    dis = haversine_distance(lat, lon, points_lat[current_point], points_lon[current_point]);

    if (dis > 2)
    {
      current_point ++;
    }

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

void start_menu()
{
  Serial.println(String(ROV) + " : ROV");
  Serial.println(String(AUV) + " : AUV");
  Serial.println(String(JYRO) + " : JYRO");
  Serial.println(String(RESET) + " : RESET");
}


void ROV_func()
{
  if (Serial1.available() > 0)
  {
    String tmp = Serial1.readStringUntil('>');
    if (tmp.length() == 59)
    {
      deserializeJson(joystick, tmp);

    }
  }
}


void loop()
{
  update_gps();


  if (state == RESET)
  {
    if (Serial.available() > 0)
    {
      String tmp = Serial.readString();

      if (tmp == String(ROV))
      {
        state = ROV;
        Serial.println("ROV state started...");
      }
      else if (tmp == String(AUV))
      {
        state = AUV;

        Serial.println("1 : AUV 1");
        Serial.println("2 : AUV 2");
        Serial.println("3 : Resrt");
      }
      else if (tmp == String(JYRO))
      {
        state = JYRO;
      }
      else if (tmp == String(RESET))
      {
        state = RESET;
        Serial.println("Reset Done.");
        start_menu();
      }
    }
  }


  else if (state == ROV)
  {
    ROV_func(); // TODO : break code
  }
  else if (state == AUV)
  {
    if (auv_state != AUV_1 && auv_state != AUV_2)
    {
      String tmp = Serial.readString();

      if (tmp == "1")
      {
        auv_state = AUV_1;
        Serial.println("Enter the count of points : ");
      }
      else if (tmp == "2")
      {
        auv_state = AUV_2;
      }
      else if (tmp == "3")
      {
        state = RESET;
        auv_state = AUV_STOP;
        start_menu();
      }
    }

    if (auv_state == AUV_1)
    {
      if (auv1_run == 0)
      {
        String tmp = Serial.readString();

        for (int i = 0; i < tmp.toInt(); i++)
        {
          Serial.println("Point " + String(i+1));
          Serial.println("Enter lat" + String(i+1));
          while (1)
          {
            String tmp = Serial.readString();
            if (tmp.toDouble())
            {
              points_lat[i] = tmp.toDouble();
              Serial.println(points_lat[i], 7);
              break;
            }
          }
          Serial.println("Enter lon" + String(i+1));
          while (1)
          {
            String tmp = Serial.readString();
            if (tmp.toDouble())
            {
              points_lon[i] = tmp.toDouble();
              break;
            }
          }
          Serial.println("Enter time" + String(i+1));    
          while (1)
          {
            String tmp = Serial.readString();
            if (tmp.toDouble())
            {
              points_time[i] = tmp.toDouble();
              break;
            }
          }

          if(i == tmp.toInt() -1)
            Serial.println("4 : RUN");
          
          while (1)
          {
            String tmp = Serial.readString();
            if (tmp.toDouble())
            {
              if (tmp == "4")
                auv1_run = 1;
              break;
            }
          }
          
        }
      }
      else if(auv1_run == 1)
      {
        AUV1_func();
      }
        
    }
  }
  else if (state == JYRO)
  {
    
  }
}
