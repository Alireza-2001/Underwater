#include <Arduino.h>
#include "Wire.h"
#include <Servo.h>
#include <ArduinoJson.h>
#include <UbxGpsNavPvt.h>
#include <math.h>


#define GPS_BAUDRATE 115200L
#define PC_BAUDRATE 115200L
#define RASPI_BAUDRATE 115200L

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

void motor_attach();
void send_data_to_operator();
void set_motor_pwm();
void check_max_min_motor_pwm();
void check_keys();
void check_gear();
void propulsion_controll(int min_pwm, int max_pwm);
void up_down_controll(int min_pwm, int max_pwm);
void mpu_config();
double haversine_distance(double lat1, double lon1, double lat2, double lon2);
void update_gps();
void AUV1_func();
void AUV2_func();
void start_menu();
void ROV_func();

DynamicJsonDocument joystick(1024);
DynamicJsonDocument setting(1024);

UbxGpsNavPvt<HardwareSerial> gps(Serial2);

Servo top_motor;
Servo bottom_motor;
Servo right_motor;
Servo left_motor;
Servo front_motor;
Servo back_motor;

const int top_motor_pin = 2;
const int bottom_motor_pin = 3;
const int left_motor_pin = 5;
const int right_motor_pin = 4;
const int vertical_motor_front_pin = 7;
const int vertical_motor_back_pin = 6;
const int led_pin = 35;

const double pi = 3.14159265358979323;
const double radius_earth = 6371000.0;

String data[13];

int gear = 1;
int j = 1;
float allowed_yaw_angle = 0;
float allowed_pitch_angle = 0;

int pwm_step = 5;
int yaw_pwm_step = 5;
int pitch_pwm_step = 5;

int key_1 = 0;
int joy_front_back = 128;
int joy_left_right = 128;
int joy_up_down = 128;
int joy = 128;

int right_motor_pwm = 1500;
int left_motor_pwm = 1500;
int top_motor_pwm = 1500;
int bottom_motor_pwm = 1500;
int vertical_motor_front_pwm = 1500;
int vertical_motor_back_pwm = 1500;

float roll = 0;
float pitch = 0;
float yaw = 0;

double lat = 0.0;
double lon = 0.0;
double points_lat [30];
double points_lon [30];
unsigned int points_time [30];
int count_point = 0;
int current_point = 0;

double h1 = 0.0;
double h_t = 0.0;
double delta_h = 0.0;
float dis = 0.0;

int state = RESET;
int rov_state = 0;
int auv_state = 0;
int auv1_run = 0;
int jyro_state = 0;

unsigned long int t1 = 0;
unsigned long int t2 = 0;
unsigned int interval = 1000;


void setup()
{
  Serial.begin(PC_BAUDRATE);
  Serial1.begin(RASPI_BAUDRATE);
  gps.begin(GPS_BAUDRATE);

  pinMode(led_pin, OUTPUT);

  mpu_config();  
  motor_attach();
  start_menu();
}

void motor_attach()
{
  top_motor.attach(top_motor_pin);
  top_motor.writeMicroseconds(1500);

  bottom_motor.attach(bottom_motor_pin);
  bottom_motor.writeMicroseconds(1500);

  right_motor.attach(right_motor_pin);
  right_motor.writeMicroseconds(1500);

  left_motor.attach(left_motor_pin);
  left_motor.writeMicroseconds(1500);

  front_motor.attach(vertical_motor_front_pin);
  front_motor.writeMicroseconds(1500);

  back_motor.attach(vertical_motor_back_pin);
  back_motor.writeMicroseconds(1500);
  
  delay(7000);
}

void set_motor_pwm()
{
  top_motor.writeMicroseconds(top_motor_pwm);
  bottom_motor.writeMicroseconds(bottom_motor_pwm);
  right_motor.writeMicroseconds(right_motor_pwm);
  left_motor.writeMicroseconds(left_motor_pwm);
  front_motor.writeMicroseconds(vertical_motor_front_pwm);
  back_motor.writeMicroseconds(vertical_motor_back_pwm);
}

void mpu_config()
{

}

void send_data_to_operator()
{
  data[0] = String(yaw);
  data[1] = String(roll);
  data[2] = String(pitch);

  data[3] = String(top_motor_pwm);
  data[4] = String(bottom_motor_pwm);
  data[5] = String(right_motor_pwm);
  data[6] = String(left_motor_pwm);
  data[7] = String(vertical_motor_front_pwm);
  data[8] = String(vertical_motor_back_pwm);

  data[9] = String(gear);

  data[10] = String(allowed_yaw_angle);
  data[11] = String(allowed_pitch_angle);
  data[12] = String(j);

  String tmp = data[0] + "," + data[1] + "," + data[2] + "," + data[3] + "," + data[4] + "," + data[5] + "," + data[6] + "," + data[7] + "," + data[8] + "," + data[9] + "," + data[10] + "," + data[11]+ "," + data[12];

  Serial1.println(tmp);
}

void check_gear()
{
  // Dandeh 1: 1400 - 1600 2: 1300 - 1700 3: 1200 - 1800 4: 1100 - 1900
  switch (gear) {
    case 1:
      propulsion_controll(1420, 1580);
      break;
    case 2:
      propulsion_controll(1300, 1700);
      break;
    case 3:
      propulsion_controll(1220, 1780);
      break;
    case 4:
      propulsion_controll(1150, 1900);
      break;
  }
}

void check_keys()
{
  // Dandeh 1: 1400 - 1600 2: 1300 - 1700 3: 1200 - 1800 4: 1100 - 1900
  switch (key_1) {
    case 1:
      gear = 1;
      break;
    case 2:
      gear = 2;
      break;
    case 3:
      gear = 3;
      break;
    case 4:
      gear = 4;
      break;
    case 5:
      break;
    case 7:
      break;
    case 9:
      j = 0;
      break;
    case 10:
      j = 1;
      break;
    case 17:
      digitalWrite(led_pin, HIGH);
      break;
    case 18:
      digitalWrite(led_pin, LOW);
      break;
  }
}

void check_max_min_motor_pwm()
{
  // check max and min motor pwm : 1100 - 1900
  if (right_motor_pwm > 1900)
    right_motor_pwm = 1900;
  if (left_motor_pwm > 1900)
    left_motor_pwm = 1900;

  if (top_motor_pwm > 1900)
    top_motor_pwm = 1900;
  if (bottom_motor_pwm > 1900)
    bottom_motor_pwm = 1900;

  if (vertical_motor_front_pwm > 1900)
    vertical_motor_front_pwm = 1900;
  if (vertical_motor_back_pwm > 1900)
    vertical_motor_back_pwm = 1900;

  if (right_motor_pwm < 1100)
    right_motor_pwm = 1100;
  if (left_motor_pwm < 1100)
    left_motor_pwm = 1100;

  if (top_motor_pwm < 1100)
    top_motor_pwm = 1100;
  if (bottom_motor_pwm < 1100)
    bottom_motor_pwm = 1100;

  if (vertical_motor_front_pwm < 1100)
    vertical_motor_front_pwm = 1100;
  if (vertical_motor_back_pwm < 1100)
    vertical_motor_back_pwm = 1100;
}

void propulsion_controll(int min_pwm, int max_pwm)
{
  top_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
  bottom_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;

  right_motor_pwm = map(255 - joy_left_right, 0, 255, min_pwm, max_pwm) + 1;
  left_motor_pwm = map(joy_left_right, 0, 255, min_pwm, max_pwm) + 1;


  if (joy_left_right < 130 && joy_left_right > 125)
  {
    right_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
    left_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
    if (j == 0)
    {
      if (yaw < allowed_yaw_angle - 1)
      {
        float dif = abs(yaw - allowed_yaw_angle);
        if (dif < 10)
          yaw_pwm_step = pwm_step;
        else if (dif >= 10 && dif < 20)
          yaw_pwm_step = 2*pwm_step;
        else if (dif >= 20 && dif < 30)
          yaw_pwm_step = 3*pwm_step;
        else if (dif >= 30 && dif < 40)
          yaw_pwm_step = 4*pwm_step;
        else if (dif >= 40 && dif < 50)
          yaw_pwm_step = 5*pwm_step;
        else if(dif >= 50)
          yaw_pwm_step = 6*pwm_step;
  
        right_motor_pwm -= yaw_pwm_step;
        left_motor_pwm += yaw_pwm_step;
      }
      else if (yaw > allowed_yaw_angle + 1)
      {
        float dif = abs(yaw - allowed_yaw_angle);
        if (dif < 10)
          yaw_pwm_step = pwm_step;
        else if (dif >= 10 && dif < 20)
          yaw_pwm_step = 2*pwm_step;
        else if (dif >= 20 && dif < 30)
          yaw_pwm_step = 3*pwm_step;
        else if (dif >= 30 && dif < 40)
          yaw_pwm_step = 4*pwm_step;
        else if (dif >= 40 && dif < 50)
          yaw_pwm_step = 5*pwm_step;
        else if (dif >= 50)
          yaw_pwm_step = 6*pwm_step;
        
        right_motor_pwm += yaw_pwm_step;
        left_motor_pwm -= yaw_pwm_step;
      }
    }
  }
  else
  {
    allowed_yaw_angle = yaw;
  }

  up_down_controll(min_pwm, max_pwm);

  check_max_min_motor_pwm();

  set_motor_pwm();
  send_data_to_operator();
}

void up_down_controll(int min_pwm, int max_pwm)
{
  if (joy < 127 || joy > 128)
  {
    vertical_motor_front_pwm = map(255 - joy, 255, 0, min_pwm, max_pwm) + 1;
    allowed_pitch_angle = pitch;
  }

  if (joy < 130 && joy > 125)
  {
    vertical_motor_front_pwm = map(joy_up_down, 0, 255, min_pwm, max_pwm) + 1;
    vertical_motor_back_pwm = map(joy_up_down, 0, 255, min_pwm, max_pwm) + 1;

    if (j == 0)
    {
      if (pitch < allowed_pitch_angle - 1)
      {
        float dif = abs(pitch - allowed_pitch_angle);
        if (dif < 10)
          pitch_pwm_step = pwm_step;
        else if (dif >= 10 && dif < 20)
          pitch_pwm_step = 2*pwm_step;
        else if (dif >= 20 && dif < 30)
          pitch_pwm_step = 3*pwm_step;
        else if (dif >= 30 && dif < 40)
          pitch_pwm_step = 4*pwm_step;
        else if (dif >= 40 && dif < 50)
          pitch_pwm_step = 5*pwm_step;
        else if(dif >= 50)
          pitch_pwm_step = 6*pwm_step;
  
        vertical_motor_front_pwm -= pitch_pwm_step;
        vertical_motor_back_pwm += pitch_pwm_step;
      }
      else if (pitch > allowed_pitch_angle + 1)
      {
        float dif = abs(pitch - allowed_pitch_angle);
        if (dif < 5)
          pitch_pwm_step = pwm_step;
        else if (dif >= 5 && dif < 20)
          pitch_pwm_step = 2*pwm_step;
        else if (dif >= 20 && dif < 30)
          pitch_pwm_step = 3*pwm_step;
        else if (dif >= 30 && dif < 40)
          pitch_pwm_step = 4*pwm_step;
        else if (dif >= 40 && dif < 50)
          pitch_pwm_step = 5*pwm_step;
        else if (dif >= 50)
          pitch_pwm_step = 6*pwm_step;
        
        vertical_motor_front_pwm += pitch_pwm_step;
        vertical_motor_back_pwm -= pitch_pwm_step;
      }
    }
  }

  if(key_1 == 11)
  {
    vertical_motor_front_pwm = 1350;
    vertical_motor_back_pwm = 1350;
  }
  if(key_1 == 12)
  {
    vertical_motor_front_pwm = 1300;
    vertical_motor_back_pwm = 1300;
  }
  if(key_1 == 15)
  {
    vertical_motor_front_pwm = 1100;
    vertical_motor_back_pwm = 1100;
  }
  if(key_1 == 13)
  {
    vertical_motor_front_pwm = 1600;
    vertical_motor_back_pwm = 1600;
  }
  if(key_1 == 14)
  {
    vertical_motor_front_pwm = 1750;
    vertical_motor_back_pwm = 1750;
  }
  if(key_1 == 16)
  {
    vertical_motor_front_pwm = 1900;
    vertical_motor_back_pwm = 1900;
  }
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
  lat1 = radians(lat1);
  lon1 = radians(lon1);
  lat2 = radians(lat2);
  lon2 = radians(lon2);

  double dlon = (lon2 - lon1);
  double dlat = (lat2 - lat1);
  double a = pow(sin((dlat / 2) / 10000000.0), 2) + cos(lat1 / 10000000.0) * cos(lat2 / 10000000.0) * pow(sin((dlon / 2)  / 10000000.0), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  double distance = c * radius_earth;
  return distance;
}

void update_gps()
{
  if (gps.ready())
  {
    lon = gps.lon;
    lat = gps.lat;

    h1 = gps.heading / 100000.0;

    if (h1 > 180)
    {
      h1 = -360 + h1;
    }

    // Serial.println(h1);
  }
}

void start_menu()
{
  Serial.println(String(ROV) + " : ROV");
  Serial.println(String(AUV) + " : AUV");
  Serial.println(String(JYRO) + " : JYRO");
  Serial.println(String(RESET) + " : RESET");
  Serial.println("-----------------------------------------------");
}

void ROV_func()
{
  if (Serial1.available() > 0)
  {
    String tmp = Serial1.readStringUntil('>');
    if (tmp.length() == 59)
    {
      deserializeJson(joystick, tmp);
      joy_front_back = 255 - int(joystick["1"]);
      joy_left_right = 255 - int(joystick["2"]);
      joy_up_down = 255 - int(joystick["3"]);
      joy = 255 - int(joystick["4"]);
      key_1 = int(joystick["5"]);

      check_keys();
      check_gear();
    }
  }
  if(Serial.available() > 0)
  {
    String tmp = Serial.readString();
    if (tmp == "1")
    {
      state = RESET;
      start_menu();
    }
  }
}

void AUV1_func()
{
  if (millis() - t1 > interval)
  {
    t1 = millis();
    
    h_t = (atan2(points_lat[current_point] - lat, points_lon[current_point] - lon) * 180 / pi);
    dis = haversine_distance(lat, lon, points_lat[current_point], points_lon[current_point]);

    if (h_t >= 0)
    {
      if (h_t <= 90)
        h_t = 90 - h_t;
      else
      {
        h_t = h_t - 90;
        h_t = -1 * h_t;
      }
    }
    else
    {
      if (h_t < 0)
        h_t = 90 + (-1 * h_t);
      else
        h_t = (-180 - h_t) + -90;
    }

    if (dis < 1.5)
    {
      if (current_point == count_point - 1)
      {
        Serial.println("Finished.");
        auv_state = AUV_STOP;
        state = RESET;
        start_menu();
      }
      else
      {
        Serial.println("Target " + String(current_point + 1));
        Serial.println("1 : run");
        t2 = millis();
        while (1)
        {
          String tmp = Serial.readString();

          if (tmp == "1")
          {
            Serial.println("Start...");
            break;
          }
          if (millis() - t2 > points_time[current_point] * 1000)
          {
            Serial.println("Start...");
            break;
          }
        }
        
        current_point ++;
      }
    }

    delta_h = h1 - h_t;

    // Serial.print(lat / 10000000.0, 7);
    // Serial.print(", ");
    // Serial.print(lon / 10000000.0, 7);
    // Serial.print(", ");
    Serial.print(h1, 2);
    Serial.print(", ");
    Serial.print(h_t, 2);
    Serial.print(", ");
    Serial.print(delta_h, 2);
    // Serial.print(", ");
    // Serial.print(dis, 2);
    // Serial.print(", ");

    // if (delta_h < -10.0)
    // {
    //   Serial.print("Left");
    // }
    // else if (-10.0 <= delta_h && delta_h <= 10.0)
    // {
    //   Serial.print("Forward");
    // }
    // else if (delta_h > 10.0)
    // {
    //   Serial.print("Right");
    // }
    Serial.println();
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
        Serial.println("1 : Stop");
      }
      else if (tmp == String(AUV))
      {
        state = AUV;

        Serial.println("1 : AUV 1");
        Serial.println("2 : AUV 2");
        Serial.println("3 : Resrt");
        Serial.println("-----------------------------------------------");
      }
      else if (tmp == String(JYRO))
      {
        state = JYRO;
        Serial.println("1 : Enable");
        Serial.println("2 : Disable");
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
    ROV_func();
  }
  else if (state == AUV)
  {
    if (auv_state != AUV_1 && auv_state != AUV_2)
    {
      if(Serial.available() > 0)
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
    }
    else if (auv_state == AUV_1)
    {
      if (auv1_run == 0)
      {
        String tmp = Serial.readString();
        count_point = tmp.toInt();

        for (int i = 0; i < tmp.toInt(); i++)
        {
          Serial.println("Point " + String(i+1));
          Serial.println("Enter lat : " + String(i+1));
          while (1)
          {
            float tmp = Serial.parseFloat();
            if (tmp)
            {
              points_lat[i] = double(tmp);
              break;
            }
          }
          Serial.println("Enter lon : " + String(i+1));
          while (1)
          {
            float tmp = Serial.parseFloat();
            if (tmp)
            {
              points_lon[i] = double(tmp);
              break;
            }
          }
          Serial.println("Enter time : " + String(i+1));    
          while (1)
          {
            float tmp = Serial.parseFloat();
            if (tmp)
            {
              points_time[i] = int(tmp);
              break;
            }
          }

          if(i == count_point - 1)
          {
            Serial.println("4 : RUN");
          
            while (1)
            {
              String tmp = Serial.readString();
              if (tmp)
              {
                if (tmp == "4")
                {
                  auv1_run = 1;
                  break;
                }
              }
            }
          }
          Serial.println("-----------------------------------------------");
        }
      }

      if(auv1_run == 1)
      {
        AUV1_func();
      }
    }
    else if(auv_state == AUV_2)
    {

    }
  }
  else if (state == JYRO)
  {
    if(Serial.available() > 0)
    {
      String tmp = Serial.readString();
      if(tmp == "1")
      {
        jyro_state = JYRO_ENABLE;
        state = RESET;
      }
      else if(tmp == "2")
      {
        jyro_state = JYRO_DISABLE;
        state = RESET;
      }
    }
  }
}


// 318356933    543539085

// 318359200    543542976