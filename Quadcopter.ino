#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>


Servo ESC1;
Servo ESC2;
Servo ESC3;
Servo ESC4;

Adafruit_MPU6050 mpu;

//=====KHAI BÁO CÁC BIẾN CHO ĐIỀU KHIỂN PID=====
float t_1, t_2, t_3, t_4;
int input_throttle, input_roll, input_pitch, input_yaw;//Các biến input lấy từ server
int16_t gyro_x, gyro_y, gyro_z, acc_x, acc_y, acc_z, temperature, acc_total_vector;//Thông số đọc từ cảm biến gia tốc
float angle_pitch, angle_roll, angle_yaw;//Các góc tính từ gyroscope của MPU
float angle_roll_acc, angle_pitch_acc;// các góc tính từ accelemeter của MPU
long gyro_x_cal, gyro_y_cal, gyro_z_cal;//Các giá trị tốc độ góc offset sau khi calibrate
float roll_error, pitch_error, yaw_error;//Error
float roll_previous_error = 0;
float pitch_previous_error = 0;
//Ở đây tôi sử dụng bộ điều khiển PID cho độ cao,roll pitch và bộ điều khiển PD cho yaw
float roll_pid_p, roll_pid_i, roll_pid_d;
float pitch_pid_p, pitch_pid_i, pitch_pid_d;
float yaw_pid_p, yaw_pid_i;
float roll_desired, pitch_desired, yaw_desired;
float roll_PID, pitch_PID, yaw_PID;

//Các hệ số PID;
float twoX_kp = 5;
float twoX_ki = 0.003;
float twoX_kd = 2;
float yaw_kp = 3;
float yaw_ki = 0.002;
long Time, PreviousTime, tsamp;

int ESCout_1 , ESCout_2 , ESCout_3 , ESCout_4; //Giá trị microsecond điều khiển BLDC
//Set ID Wifi và password cho Wifi


#include "setupWifi.h"
void setup() {
  delay(1000);
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
//  Serial.print("Accelerometer range set to: ");
//  switch (mpu.getAccelerometerRange()) {
//    case MPU6050_RANGE_2_G:
//      Serial.println("+-2G");
//      break;
//    case MPU6050_RANGE_4_G:
//      Serial.println("+-4G");
//      break;
//    case MPU6050_RANGE_8_G:
//      Serial.println("+-8G");
//      break;
//    case MPU6050_RANGE_16_G:
//      Serial.println("+-16G");
//      break;
//  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
//  switch (mpu.getGyroRange()) {
//    case MPU6050_RANGE_250_DEG:
//      Serial.println("+- 250 deg/s");
//      break;
//    case MPU6050_RANGE_500_DEG:
//      Serial.println("+- 500 deg/s");
//      break;
//    case MPU6050_RANGE_1000_DEG:
//      Serial.println("+- 1000 deg/s");
//      break;
//    case MPU6050_RANGE_2000_DEG:
//      Serial.println("+- 2000 deg/s");
//      break;
//  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
//  Serial.print("Filter bandwidth set to: ");
//  switch (mpu.getFilterBandwidth()) {
//    case MPU6050_BAND_260_HZ:
//      Serial.println("260 Hz");
//      break;
//    case MPU6050_BAND_184_HZ:
//      Serial.println("184 Hz");
//      break;
//    case MPU6050_BAND_94_HZ:
//      Serial.println("94 Hz");
//      break;
//    case MPU6050_BAND_44_HZ:
//      Serial.println("44 Hz");
//      break;
//    case MPU6050_BAND_21_HZ:
//      Serial.println("21 Hz");
//      break;
//    case MPU6050_BAND_10_HZ:
//      Serial.println("10 Hz");
//      break;
//    case MPU6050_BAND_5_HZ:
//      Serial.println("5 Hz");
//      break;
//  }
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++) {
    if (cal_int % 125 == 0)Serial.print(".");
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    gyro_x = g.gyro.x;
    gyro_y = g.gyro.y;
    gyro_z = g.gyro.z;
    gyro_x_cal += gyro_x;
    gyro_y_cal += gyro_y;
    gyro_z_cal += gyro_z;
    delayMicroseconds(1000);
  }
  gyro_x_cal /= 2000;
  gyro_y_cal /= 2000;
  gyro_z_cal /= 2000;
  Serial.println("Calibrated successfully");
  //===================SETTING UP SERVER=================================
  setupServer();
  //======================SETTING UP PIN=============================
  ESC1.attach(D5); ESC2.attach(D6); ESC3.attach(D7); ESC4.attach(D8);

  ESC1.writeMicroseconds(2000);
  ESC2.writeMicroseconds(2000);
  ESC3.writeMicroseconds(2000);
  ESC4.writeMicroseconds(2000);
  delay(2000);
  Time = micros();

}
void PID() {
  //Đọc giá trị từ cảm biến MPU: gyro_x, gyro_y, gyro_z, acc_x, acc_y, acc_z, temperature
  gyro_x -= gyro_x_cal;
  gyro_y -= gyro_y_cal;
  gyro_z -= gyro_z_cal;
  angle_pitch -= gyro_x * tsamp * 57.296;
  angle_roll += gyro_y * tsamp * 57.296;
  angle_yaw += gyro_z * tsamp;

  acc_total_vector = sqrt((acc_x * acc_x) + (acc_y * acc_y) + (acc_z * acc_z));
  angle_pitch_acc = asin((float)acc_y / acc_total_vector) * -57.296;
  angle_roll_acc = asin((float)acc_x / acc_total_vector) * -57.296;

  //Lọc nhiễu bù
  angle_pitch = angle_pitch * 0.96 + angle_pitch_acc * 0.04;
  angle_roll = angle_roll * 0.96 + angle_roll_acc * 0.04;

  //Tính error:
  //roll_error, roll_last_error, pitch_error, pitch_last_error, yaw_error, yaw_last_error
  input_roll = 0;
  input_pitch = 0;
  input_yaw = 0;
  roll_error =  -angle_roll + input_roll;
  pitch_error = -angle_pitch + input_pitch;
  yaw_error = -angle_yaw + input_yaw;
  //  Serial.print("error pitch: ");
  //  Serial.println(pitch_error);
  roll_pid_p = twoX_kp * roll_error;
  pitch_pid_p = twoX_kp * pitch_error;
  yaw_pid_p = yaw_kp * yaw_error;


  if (-3 < roll_error < 3) {
    roll_pid_i = roll_pid_i + (twoX_ki * roll_error);
  }
  if (-3 < pitch_error < 3) {
    pitch_pid_i = pitch_pid_i + (twoX_ki * pitch_error);
  }
  if (-3 < yaw_error < 3) {
    yaw_pid_i = yaw_pid_i + (yaw_ki * yaw_error);
  }

  roll_pid_d = twoX_kd * (roll_error - roll_previous_error);
  pitch_pid_d = twoX_kd * (pitch_error - pitch_previous_error);

  roll_PID = roll_pid_p + roll_pid_i + roll_pid_d;
  pitch_PID = pitch_pid_p + pitch_pid_i + pitch_pid_d;
  yaw_PID = yaw_pid_p + yaw_pid_i;
  roll_PID = constrain(roll_PID, -400, 400);
  pitch_PID  = constrain(pitch_PID, -400, 400);
  yaw_PID = constrain(yaw_PID, -400, 400);
  if (roll_PID < -400) {
    roll_PID = -400;
  }
  else if (roll_PID > 400) {
    roll_PID = 400;
  }
  if (pitch_PID < -400) {
    pitch_PID = -400;
  }
  else if (pitch_PID > 400) {
    pitch_PID = 400;
  }
  if (yaw_PID < -400) {
    yaw_PID = -400;
  }
  else if (yaw_PID > 400) {
    yaw_PID = 400;
  }
  ESCout_1 = input_throttle - (int)roll_PID + (int)pitch_PID + (int)yaw_PID;
  ESCout_2 = input_throttle + (int)roll_PID - (int)pitch_PID + (int)yaw_PID;
  ESCout_3 = input_throttle + (int)roll_PID + (int)pitch_PID - (int)yaw_PID;
  ESCout_4 = input_throttle - (int)roll_PID - (int)pitch_PID - (int)yaw_PID;
  if (abs(roll_error) < 10 && abs(pitch_error) < 10) {
    ESCout_1 = constrain(ESCout_1, 0, 2000);
    ESCout_2 = constrain(ESCout_2, 0, 2000);
    ESCout_3 = constrain(ESCout_3, 0, 2000);
    ESCout_4 = constrain(ESCout_4, 0, 2000);
  }
  else if (abs(roll_error) > 10 || abs(pitch_error) > 10) {
    ESCout_1 = constrain(ESCout_1, 1100, 2000);
    ESCout_2 = constrain(ESCout_2, 1100, 2000);
    ESCout_3 = constrain(ESCout_3, 1100, 2000);
    ESCout_4 = constrain(ESCout_4, 1100, 2000);
  }
  //  Serial.print("ESCout 1: ");
  //  Serial.print(ESCout_1);
  //  Serial.print(", ESCout 2: ");
  //  Serial.print(ESCout_2);
  //  Serial.print(", ESCout 3: ");
  //  Serial.print(ESCout_3);
  //  Serial.print(", ESCout 4: ");
  //  Serial.println(ESCout_4);
  roll_previous_error = roll_error;
  pitch_previous_error = pitch_error;

  ESC1.writeMicroseconds(ESCout_1);
  ESC2.writeMicroseconds(ESCout_2);
  ESC3.writeMicroseconds(ESCout_3);
  ESC4.writeMicroseconds(ESCout_4);

}
void loop() {
  server.handleClient();
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  //Đọc gia tốc và tốc độ góc 3 trục
  acc_x = a.acceleration.x;
  acc_y = a.acceleration.y;
  acc_z = a.acceleration.z;
  gyro_x = g.gyro.x;
  gyro_y = g.gyro.y;
  gyro_z = g.gyro.z;

  PreviousTime = Time;
  Time = micros();
  tsamp = (float)(Time - PreviousTime) / 1000000;
  PID();

}
