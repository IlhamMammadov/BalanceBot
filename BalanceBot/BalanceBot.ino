 /*
 * Author: Ilham Mammadov
 * Date: 23.07.2016
 * 
 * BalanceBot is meant for 2 wheel balancing bot which will use MPU6050 for IMU, and 2 dc motors for controlling
 * Requires MPU6050+DMP and I2Cdev libraries
 * 
 */
#include "I2Cdev.h"
//#include "MPU6050.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <DcMotor.h>
#include <Motion.h>
#include <SoftwareSerial.h>


//################################################
//IMU sensor related stuff
//################################################
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 imu;


//variables to hold time
unsigned long t_dur;    //For loop duration

//CONSTANTS
#define INTERRUPT_PIN 2                     //INT0 or D2 on UNO or related boards


// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer


// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//################################################
//Interrupt routine for DMP data ready
//################################################
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}


//################################################
//Motion related stuff
//################################################
DcMotor lm(10,A0,A1);       //Left motor
DcMotor rm(9,A2,A3);        //Right motor
Motion motion(&lm, &rm);    //Motion object using two motors

//################################################
//PID controller stuff
//################################################
int Kp = 100;
int Ki = 0;
int Kd = 300;
float K = 1;
float P;
float I, iError;
float D;
float error, lastError;
int pwm;                      //PID output
float control_angle;          //PID input
float desired_angle = 0.0;    //PID setpoint
float th = 0;                 //Threshold around setpoint, not necessary


//################################################
//Communication stuff
//################################################
SoftwareSerial esp(3,4);      //Serial interface with WIFI module

//################################################
//SETUP stuff
//################################################
void setupDMP(){
  
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  imu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(imu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = imu.dmpInitialize();

  // Chech if it worked (returns 0 if so)
  if (devStatus == 0) {
      // turn on the DMP, now that it's ready
      Serial.println(F("Enabling DMP..."));
      imu.setDMPEnabled(true);

      // enable Arduino interrupt detection
      Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
      attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
      mpuIntStatus = imu.getIntStatus();

      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      Serial.println(F("DMP ready! Waiting for first interrupt..."));
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = imu.dmpGetFIFOPacketSize();
  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      Serial.print(F("DMP Initialization failed (code "));
      Serial.print(devStatus);
      Serial.println(F(")"));
  }
}

void setup() {

  delay(2000);
  //Serial for debugging
  Serial.begin(115200);
  
  //################################################
  //SENSOR
  //################################################
  setupDMP();  
  
  //Callibrate and be ready to get filtered data from IMU
  //IMUgetReady();

  //################################################
  //MOTION
  //################################################
  motion.Setup();
  Serial.println(F("Motion is setup..."));


  //################################################
  //CONTROLLER
  //################################################

  
  //################################################
  //Communication
  //################################################
  esp.begin(38400);
  Serial.println(F("Communication to ESP istablished"));
  //send PID values to webserver in ESP
  Serial.println("p"+String(Kp)+"i"+String(Ki)+"d"+String(Kd));
//  esp.println("p"+String(Kp)+"i"+String(Ki)+"d"+String(Kd));
//  Serial.println(F("PID is sent to webserver"));
}



void getDMPdata(){
  // if programming failed, don't try to do anything
  if (!dmpReady) return;

  //#####################
  //Check if interrupt is received
  if(mpuInterrupt) {
    
    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = imu.getIntStatus();

    // get current FIFO count
    fifoCount = imu.getFIFOCount();
    
    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        imu.resetFIFO();
        Serial.println(F("FIFO overflow!"));
  
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = imu.getFIFOCount();
  
        // read a packet from FIFO
        imu.getFIFOBytes(fifoBuffer, packetSize);

        //clear FIFO to start from zero
//        imu.resetFIFO();
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
  
        imu.dmpGetQuaternion(&q, fifoBuffer);
        imu.dmpGetGravity(&gravity, &q);
        imu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        //convert ypr from radian to degrees;
        ypr[0] *= 180/M_PI;
        ypr[1] *= 180/M_PI;
        ypr[2] *= 180/M_PI;
        
        //PID control angle = pitch
        control_angle = ypr[1];
    }
  }
  //#####################
  
}


void checkCommand(){
  
  if(esp.available()){
  char command = (char)esp.read();
//  Serial.println(command);
  switch(command){
    //motion part
    case 'f':
    motion.Forward();
    break;
    case 'b':
    motion.Backward();
    break;
    case 'l':
    motion.RotateLeft();
    break;
    case 'r':
    motion.RotateRight();
    break;
    case 's':
    motion.Stop();
    break;
    
    //PID part
    case 'p':
    //read all data into string and then parse info:
    String pid_val = String();
    while(esp.available()){
      pid_val += (char)esp.read();
    }
//    Serial.print("pid_val:");
//    Serial.println(pid_val);
    Kp = pid_val.substring(0,pid_val.indexOf("i")).toInt();
    Ki = pid_val.substring(pid_val.indexOf("i")+1,pid_val.indexOf("d")).toInt();
    Kd = pid_val.substring(pid_val.indexOf("d")+1).toInt();
    
//    Serial.println("p"+String(Kp)+"i"+String(Ki)+"d"+String(Kd)); //debugging esp received data
    break;
    }
  //send back stored PID values to webserver, acts kind of Ack.
//  esp.println("p"+String(Kp)+"i"+String(Ki)+"d"+String(Kd));
  }
}

void motionControl(){
  //compute required pwm output
  doPID();
  
  //if close to desired angle dont do anything
  if(abs(control_angle - desired_angle) < th || (pwm == 0)){
    Serial.print("\tStop:\t0.0");
    motion.Stop();
  }
  //if leaning forward correct by forward motion
  else if(pwm > 0){
    pwm = map(pwm,0, 255, 0, 255);
    Serial.print("  \tForward:  ");
    Serial.print(pwm);
    motion.Forward(pwm);
  }
  //if leaning backward correct by backward motion
  else if(pwm < 0){
    pwm = map(pwm,0, -255, 0, -255);
    Serial.print("  \tBackward: ");
    Serial.print(-pwm);
    motion.Backward(-pwm);
  }
}

void checkSerialPid(){
  
  char K_type;
  
  if (Serial.available()){
    
    K_type = (char)Serial.read();
    switch(K_type){
      case 'p':
            Kp = Serial.parseInt();
            break;
      case 'i':
            Ki = Serial.parseInt();
            break;
      case 'd':
            Kd = Serial.parseInt();
            break;
      case 'k':
            K = Serial.parseInt();
            break;
      }
  }
}

void doPID(){
  error = desired_angle - control_angle;
  P = Kp * error;
  iError += error;
  I = Ki * constrain(iError, -10, 10);
  D = Kd * (error - lastError);
  lastError = error;
  pwm = constrain((P + I + D)/K, -255, 255);
}

void loop() {

  t_dur = micros();
  //start reading dmp data from sensor
  
  getDMPdata();
  SerialOutALL();
  checkCommand();
  motionControl();
  checkSerialPid();

//  while((micros() - t_dur)/1000.0 < 10) ;   // to set equal sampling rate of 100 Hz
  Serial.print(F("\tLoop:\t"));
  Serial.println((micros() - t_dur)/1000.0);

}






//############################
//SERIAL OUTPUTS
//############################

void SerialOutALL(){
  
  SerialOutYPR();
  SerialOutK();
}


void SerialOutYPR(){
  Serial.print(F("YPR:\ty:\t"));
  Serial.print(ypr[0]);
  Serial.print(F("\tp:\t"));
  Serial.print(ypr[1]);
  Serial.print(F("\tr:\t"));
  Serial.print(ypr[2]);
}

void SerialOutK(){
  Serial.print(F("\tKp:\t"));
  Serial.print(Kp);
  Serial.print(F("\tKi:\t"));
  Serial.print(Ki);
  Serial.print(F("\tKd:\t"));
  Serial.print(Kd);
  Serial.print(F("\tK:\t"));
  Serial.print(K);
}


