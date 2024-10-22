#include <Arduino.h>
#include <MPU6050_tockn.h>
#include "Wire.h"
#include <Servo.h>
#include <ArduinoJson.h>
#include <UbxGpsNavPvt.h>
#include <math.h>


#define GPS_BAUDRATE 115200L
#define PC_BAUDRATE 115200L
#define RASPI_BAUDRATE 250000L

#define ROV 1
#define ROV_1 5
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
void mpu_update();
void motor_off();
void read_voltage();
void jyro_stable_yaw();

DynamicJsonDocument joystick(1024);
DynamicJsonDocument setting(1024);

UbxGpsNavPvt<HardwareSerial> gps(Serial2);
MPU6050 mpu6050(Wire);

Servo top_motor;
Servo bottom_motor;
Servo right_motor;
Servo left_motor;
Servo front_motor;
Servo back_motor;

const int top_motor_pin = 2;
const int bottom_motor_pin = 3;
const int left_motor_pin = 4;
const int right_motor_pin = 5;
const int vertical_motor_front_pin = 7;
const int vertical_motor_back_pin = 6;
const int front_led_pin = 12;

const float Vref = 4.8; 
const float R1 = 36088; 
const float R2 = 4700; 

const float tolerance_jyro = 5.0;
const float tolerance_gps = 10.0;

const int auv_base_pwm = 1750;

const int first_forward_time = 5;

const double pi = 3.14159265358979323;
const double radius_earth = 6371000.0;

const int turn_pwm = 100;

int gear = 1;
float allowed_yaw_angle = 0;
float allowed_pitch_angle = 0;
float dif_yaw_angle = 0.0;
float dif_pitch_angle = 0.0;

int pwm_step = 40;
int yaw_pwm_step = 0;
int pitch_pwm_step = 0;

int joy_front_back = 128;
int joy_left_right = 128;
int joy_up_down_ver = 128;
int joy_up_down_hor = 128;

int right_motor_pwm = 1500;
int left_motor_pwm = 1500;
int top_motor_pwm = 1500;
int bottom_motor_pwm = 1500;
int vertical_motor_front_pwm = 1500;
int vertical_motor_back_pwm = 1500;

float roll = 0;
float pitch = 0;
float yaw = 0;

float k = 6;
int a = 5;
double lat_filter[5];
double lon_filter[5];
double sum_lat = 0.0;
double sum_lon = 0.0;

double lat = 0.0;
double lon = 0.0;
int satellite_count = 0;
float speed = 0.0;
double points_lat [30];
double points_lon [30];
unsigned int points_time [30];
int count_point = 0;
int current_point = 0;

double rov_1_lat = 0.0;
double rov_1_lon = 0.0;

float distance;
double h1 = 0.0;
double h_t = 0.0;
double delta_h = 0.0;
float battery_voltage = 0.0;

int state = RESET;
int rov_state = 0;
int auv_state = 0;
int auv1_run = 0;
int rov_1_run = 0;
int jyro_state = JYRO_DISABLE;

unsigned long int t1 = 0;
unsigned long int t2 = 0;
unsigned long int t3 = 0;
unsigned long int auv_start_time = 0;
unsigned int interval = 1000;


void setup()
{
  Serial.begin(PC_BAUDRATE);
  Serial1.begin(RASPI_BAUDRATE);
  gps.begin(GPS_BAUDRATE);

  pinMode(front_led_pin, OUTPUT);
  pinMode(A0,INPUT);

  motor_attach();
  mpu_config();
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
  delay(4000);
}

void set_motor_pwm()
{
  check_max_min_motor_pwm();
//  top_motor.writeMicroseconds(top_motor_pwm);
  bottom_motor.writeMicroseconds(bottom_motor_pwm);
  right_motor.writeMicroseconds(right_motor_pwm);
  left_motor.writeMicroseconds(left_motor_pwm);
  front_motor.writeMicroseconds(vertical_motor_front_pwm);
  back_motor.writeMicroseconds(vertical_motor_back_pwm);
}

void mpu_config()
{
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.println();
}

void send_data_to_operator()
{
  if (millis() - t3 > 500)
  {
    t3 = millis();
    String tmp = String(top_motor_pwm) + "," + String(bottom_motor_pwm) + "," + String(right_motor_pwm) + "," + String(left_motor_pwm) + "," + String(vertical_motor_front_pwm) + "," + String(vertical_motor_back_pwm) + "," + String(roll) + "," + String(pitch) + "," + String(yaw) + "," + String(jyro_state) + "," + String(lat) + "," + String(lon) + "," + String(satellite_count) + "," + String(speed) + "," + String(distance) + "," + String(delta_h) + "," + String(gear) + "," + String(battery_voltage);
    Serial1.println(tmp);
  }
}

void check_gear()
{
  switch (gear) {
    case 1:
      propulsion_controll(1400, 1600);
      break;
    case 2:
      propulsion_controll(1300, 1700);
      break;
    case 3:
      propulsion_controll(1200, 1800);
      break;
    case 4:
      propulsion_controll(1100, 1900);
      break;
  }
}

void check_max_min_motor_pwm()
{
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

void jyro_stable_yaw()
{
  if (jyro_state == JYRO_ENABLE)// jyro yaw angle
  {
    if (yaw < allowed_yaw_angle - tolerance_jyro)
    {
      dif_yaw_angle = abs(yaw - allowed_yaw_angle);
      if (dif_yaw_angle < 10)
        yaw_pwm_step = pwm_step;
      else if (dif_yaw_angle >= 10 && dif_yaw_angle < 30)
        yaw_pwm_step = 2*pwm_step;
      else if (dif_yaw_angle >= 30)
        yaw_pwm_step = 3*pwm_step;

      left_motor_pwm += yaw_pwm_step;
      right_motor_pwm -= yaw_pwm_step;
    }
    else if (yaw > allowed_yaw_angle + tolerance_jyro)
    {
      dif_yaw_angle = abs(yaw - allowed_yaw_angle);
      if (dif_yaw_angle < 10)
        yaw_pwm_step = pwm_step;
      else if (dif_yaw_angle >= 10 && dif_yaw_angle < 30)
        yaw_pwm_step = 2*pwm_step;
      else if (dif_yaw_angle >= 30)
        yaw_pwm_step = 3*pwm_step;
 
      right_motor_pwm += yaw_pwm_step;
      left_motor_pwm -= yaw_pwm_step;
    }
  }
}

void propulsion_controll(int min_pwm, int max_pwm)
{
  top_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
  bottom_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;

  if (joy_left_right < 130 && joy_left_right > 125) // forward move
  {
    right_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
    left_motor_pwm = map(joy_front_back, 0, 255, min_pwm, max_pwm) + 1;
  }
  else
  { // right and left
    right_motor_pwm = map(255 - joy_left_right, 0, 255, min_pwm, max_pwm) + 1;
    left_motor_pwm = map(joy_left_right, 0, 255, min_pwm, max_pwm) + 1;

    allowed_yaw_angle = yaw;
  }


  if (int(joystick["6"]) == 1) //   forward
  {
    top_motor_pwm = max_pwm;
    bottom_motor_pwm = max_pwm;
    right_motor_pwm = max_pwm;
    left_motor_pwm = max_pwm;
  }
  else if (int(joystick["6"]) == 2) //    right
  {
    top_motor_pwm = max_pwm;
    bottom_motor_pwm = max_pwm;
    right_motor_pwm = max_pwm + turn_pwm;
    left_motor_pwm = max_pwm - turn_pwm;
    allowed_yaw_angle = yaw; 
  }
  else if (int(joystick["6"]) == 3) //    back
  {
    top_motor_pwm = min_pwm;
    bottom_motor_pwm = min_pwm;
    right_motor_pwm = min_pwm;
    left_motor_pwm = min_pwm;    
  }
  else if (int(joystick["6"]) == 4) //    left
  {
    top_motor_pwm = max_pwm;
    bottom_motor_pwm = max_pwm;
    right_motor_pwm = max_pwm - turn_pwm;
    left_motor_pwm = max_pwm + turn_pwm;
    allowed_yaw_angle = yaw;
  }

  jyro_stable_yaw();

  up_down_controll(1100, 1900);

  set_motor_pwm();
}

void up_down_controll(int min_pwm, int max_pwm)
{
  if (joy_up_down_hor < 127 || joy_up_down_hor > 128)
  {
    vertical_motor_front_pwm = map(255 - joy_up_down_hor, 255, 0, min_pwm, max_pwm) + 1;
    vertical_motor_back_pwm = map(joy_up_down_hor, 255, 0, min_pwm, max_pwm) + 1;
    allowed_pitch_angle = pitch;
  }

  if (joy_up_down_hor < 130 && joy_up_down_hor > 125)
  {
    vertical_motor_front_pwm = map(joy_up_down_ver, 0, 255, min_pwm, max_pwm) + 1;
    vertical_motor_back_pwm = map(joy_up_down_ver, 0, 255, min_pwm, max_pwm) + 1;

    switch (int(joystick["8"]))
    {
    case 1:
      vertical_motor_front_pwm = 1600;
      vertical_motor_back_pwm = 1600;
      break;
    case 2:
      vertical_motor_front_pwm = 1300;
      vertical_motor_back_pwm = 1300;
      break;
    case 3:
      vertical_motor_front_pwm = 1750;
      vertical_motor_back_pwm = 1750;
      break;
    case 4:
      vertical_motor_front_pwm = 1200;
      vertical_motor_back_pwm = 1200;
      break;
    case 5:
      vertical_motor_front_pwm = 1100;
      vertical_motor_back_pwm = 1100;
      break;
    case 6:
      vertical_motor_front_pwm = 1900;
      vertical_motor_back_pwm = 1900;
      break;
    }

    if (jyro_state == JYRO_ENABLE)
    {
      if (pitch < allowed_pitch_angle - tolerance_jyro)
      {
        dif_pitch_angle = abs(pitch - allowed_pitch_angle);
        if (dif_pitch_angle < 10)
          pitch_pwm_step = pwm_step;
        else if (dif_pitch_angle >= 10 && dif_pitch_angle < 30)
          pitch_pwm_step = 2*pwm_step;
        else if (dif_pitch_angle >= 30)
          pitch_pwm_step = 3*pwm_step;

        vertical_motor_back_pwm += pitch_pwm_step;
        vertical_motor_front_pwm -= pitch_pwm_step;
      }
      else if (pitch > allowed_pitch_angle + tolerance_jyro)
      {
        dif_pitch_angle = abs(pitch - allowed_pitch_angle);
        if (dif_pitch_angle < 10)
          pitch_pwm_step = 2*pwm_step;
        else if (dif_pitch_angle >= 5 && dif_pitch_angle < 30)
          pitch_pwm_step = 3*pwm_step;
        else if (dif_pitch_angle >= 30)
          pitch_pwm_step = 4*pwm_step;

        if(vertical_motor_front_pwm < 1850)
          vertical_motor_front_pwm += pitch_pwm_step;
        else
          vertical_motor_back_pwm -= pitch_pwm_step;
        vertical_motor_back_pwm -= pitch_pwm_step;
      }
    }
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

void mpu_update()
{
  mpu6050.update();
  yaw = mpu6050.getAngleZ();
  pitch = mpu6050.getAngleY();
  roll = mpu6050.getAngleX();
}

void start_menu()
{
  Serial.println("-----------------------------------------------");
  Serial.println(String(ROV) + " : ROV");
  Serial.println(String(AUV) + " : AUV");
  Serial.println(String(JYRO) + " : JYRO");
  Serial.println(String(RESET) + " : RESET");
  Serial.println(String(ROV_1) + " : ROV 1");
  Serial.println("-----------------------------------------------");
}

void read_voltage()
{
  battery_voltage = ((analogRead(0) * Vref) / 1024.0) / (R2 / (R1 + R2)); 
}

void ROV_func()
{
  if (Serial1.available() > 0)
  {
    String tmp = Serial1.readStringUntil('>');

    if (tmp.length() == 113)
    {
      deserializeJson(joystick, tmp);
      joy_front_back = 255 - int(joystick["2"]);
      joy_left_right = 255 - int(joystick["1"]);
      joy_up_down_ver = 255 - int(joystick["4"]);
      joy_up_down_hor = 255 - int(joystick["3"]);

      if (int(joystick["5"]) != 0)
      {
        gear = int(joystick["5"]);
      }

      switch (int(joystick["7"]))
      {
        case 1:
          jyro_state = JYRO_ENABLE;
          allowed_pitch_angle = pitch;
          allowed_yaw_angle = yaw;
          break;
        case 2:
          jyro_state = JYRO_DISABLE;
          break;
        case 3:
          break;
        case 4:
          break;      
      }
      analogWrite(front_led_pin, int(joystick["9"]));
    }
    check_gear();
  }
  else
  {
    top_motor.writeMicroseconds(1500);
    bottom_motor.writeMicroseconds(1500);
    right_motor.writeMicroseconds(1500);
    left_motor.writeMicroseconds(1500);
    front_motor.writeMicroseconds(1500);
    back_motor.writeMicroseconds(1500);
  }
  
  if(Serial.available() > 0)
  {
    String tmp = Serial.readString();
    if (tmp == "1")
    {
      state = RESET;
      start_menu();
      gear = 1;
      top_motor_pwm = 1500;
      bottom_motor_pwm = 1500;
      left_motor_pwm = 1500;
      right_motor_pwm = 1500;
      vertical_motor_back_pwm = 1500;
      vertical_motor_front_pwm = 1500;

      top_motor.writeMicroseconds(1500);
      bottom_motor.writeMicroseconds(1500);
      right_motor.writeMicroseconds(1500);
      left_motor.writeMicroseconds(1500);
      front_motor.writeMicroseconds(1500);
      back_motor.writeMicroseconds(1500);
    }
  }
}

void update_gps()
{
  if (gps.ready())
  {
    for (int i = 0; i < a - 1; i++)
    {
      lat_filter[i] = lat_filter[i + 1];
      lat_filter[a - 1] = gps.lat;
    }
    sum_lat = 0.0;
    for (int i = 0; i < a; i++)
    {
      sum_lat = sum_lat + lat_filter[i];
    }
    lat = sum_lat / a;

    for (int i = 0; i < a - 1; i++)
    {
      lon_filter[i] = lon_filter[i + 1];
      lon_filter[a - 1] = gps.lon;
    }
    sum_lon = 0.0;
    for (int i = 0; i < a; i++)
    {
      sum_lon = sum_lon + lon_filter[i];
    }
    lon = sum_lon / a;

    h1 = gps.heading / 100000.0;

    speed = gps.gSpeed * 0.0036;
    satellite_count = gps.numSV;

    if (h1 > 180)
    {
      h1 = -360 + h1;
    }

    if (h1 < -90)
    {
      h1 = -1 * (h1 + 270);
    }
    else
    {
      h1 = 90 - h1;
    }

    // Serial.print(lat, 7);
    // Serial.print(",");
    // Serial.print(lon, 7);
    // Serial.println();
  }
}

void AUV1_func()
{
  if (millis() - t1 > interval)
  {
    t1 = millis();
    
    h_t = atan2(points_lat[current_point] - lat, points_lon[current_point] - lon) * 180 / pi;
    distance = haversine_distance(lat, lon, points_lat[current_point], points_lon[current_point]);

    if (distance < 2.0)
    {
      if (current_point == count_point - 1)
      {
        motor_off();
        Serial.println("Finished.");
        auv_state = AUV_STOP;
        state = RESET;
        auv1_run = 0;
        start_menu();
      }
      else
      {
        motor_off();
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

        t2 = millis();
        right_motor_pwm = auv_base_pwm + 150;
        left_motor_pwm = auv_base_pwm + 150;
        bottom_motor_pwm = auv_base_pwm + 150;
        while (1)
        {
          if (millis() - t2 > first_forward_time * 1000)
          {
            break;
          }
        }
        
        current_point ++;
      }
    }
    else
    {
      delta_h = h1 - h_t;

      if (delta_h > 180)
      {
        delta_h = 360 - delta_h;
      }
      else if (delta_h < -180)
      {
        delta_h = 360 - abs(delta_h);        
      }

      Serial.print(delta_h, 2);
      Serial.print(", ");
      Serial.print(distance, 2);
      Serial.print(", ");

      if (delta_h < -1*tolerance_gps)
      {
        float dif = abs(delta_h + tolerance_gps);

        int coefficient_pwm = int(dif * k);

        right_motor_pwm = auv_base_pwm - coefficient_pwm;
        left_motor_pwm = auv_base_pwm + coefficient_pwm;
        bottom_motor_pwm = auv_base_pwm + 150;
        Serial.print("Left");

        Serial.print(", ");
        Serial.print(left_motor_pwm);
        Serial.print(", ");
        Serial.print(right_motor_pwm);
      }
      else if (-1*tolerance_gps <= delta_h && delta_h <= tolerance_gps)
      {
        Serial.print("Forward");
        right_motor_pwm = auv_base_pwm + 150;
        left_motor_pwm = auv_base_pwm + 150;
        bottom_motor_pwm = auv_base_pwm;

        Serial.print(", ");
        Serial.print(left_motor_pwm);
        Serial.print(", ");
        Serial.print(right_motor_pwm);
      }
      else if (delta_h > tolerance_gps)
      {
        float dif = abs(delta_h - tolerance_gps);

        int coefficient_pwm = int(dif * k);

        right_motor_pwm = auv_base_pwm + coefficient_pwm;
        left_motor_pwm = auv_base_pwm - coefficient_pwm;
        bottom_motor_pwm = auv_base_pwm + 150;
        Serial.print("Right");

        Serial.print(", ");
        Serial.print(left_motor_pwm);
        Serial.print(", ");
        Serial.print(right_motor_pwm);
      }
      Serial.println();
    }
    set_motor_pwm();
  }

  if (Serial.available() > 0)
  {
    String tmp = Serial.readString();

    if (tmp == "85")
    {
      state = RESET;
      auv1_run = 0;
      auv_state = AUV_STOP;
      start_menu();
    }
  }
}

void motor_off()
{
  right_motor_pwm = 1500;
  left_motor_pwm = 1500;
  top_motor_pwm = 1500;
  bottom_motor_pwm = 1500;
  vertical_motor_back_pwm = 1500;
  vertical_motor_front_pwm = 1500;
  set_motor_pwm();
}

void loop()
{
  update_gps();
  mpu_update();

  if (state == RESET)
  {
    motor_off();

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
        Serial.println("-----------------------------------------------");
        Serial.println("1 : AUV 1");
        Serial.println("2 : AUV 2");
        Serial.println("3 : Resrt");
        Serial.println("-----------------------------------------------");
      }
      else if (tmp == String(JYRO))
      {
        state = JYRO;
        Serial.println("-----------------------------------------------");
        Serial.println("1 : Enable");
        Serial.println("2 : Disable");
        Serial.println("-----------------------------------------------");
      }
      else if (tmp == String(RESET))
      {
        state = RESET;
        Serial.println("Reset Done.");
        start_menu();
      }
      else if (tmp == String(ROV_1))
      {
        state = ROV_1;
        rov_1_run = 0;
        Serial.println("-----------------------------------------------");
        Serial.println("point :");
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
          Serial.println("-----------------------------------------------");
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
                  auv_start_time = millis();
                  break;
                }
              }
            }
          }
          Serial.println("-----------------------------------------------");
        }
      }
      else if(auv1_run == 1)
      {
        if (millis() - auv_start_time < first_forward_time * 1000)
        {
          right_motor_pwm = auv_base_pwm;
          left_motor_pwm = auv_base_pwm;
          set_motor_pwm();
        }
        else
        {
          AUV1_func();
        }
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
        allowed_pitch_angle = pitch;
        allowed_yaw_angle = yaw;
        state = RESET;
        start_menu();
      }
      else if(tmp == "2")
      {
        jyro_state = JYRO_DISABLE;
        state = RESET;
        start_menu();
      }
    }
  }
  else if (state == ROV_1)
  {
    if (rov_1_run == 0)
    {
      Serial.println("Enter lat : ");
      while (1)
      {
        float tmp = Serial.parseFloat();
        if (tmp)
        {
          rov_1_lat = double(tmp);
          break;
        }
      }
      Serial.println("Enter lon : ");
      while (1)
      {
        float tmp = Serial.parseFloat();
        if (tmp)
        {
          rov_1_lon = double(tmp);
          break;
        }
      }

      rov_1_run = 1;
      Serial.println("ROV 1 started...");
      Serial.println("1 : Stop");
    }
    else
    {
      ROV_func();

      h_t = atan2(rov_1_lat - lat, rov_1_lon - lon) * 180 / pi;
      distance = haversine_distance(lat, lon, rov_1_lat, rov_1_lon);

      Serial.print(lat / 10000000.0, 7);
      Serial.print(", ");
      Serial.print(lon / 10000000.0, 7);
      Serial.print(", ");
      Serial.print(delta_h, 2);
      Serial.print(", ");
      Serial.print(distance, 2);
    }
  }
  
  read_voltage();
  // send_data_to_operator();
}