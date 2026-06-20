#include <AccelStepper.h>
//#include <MultiStepper.h>

#define X_MAX 2200
#define Y_MAX 1400
#define Z_MAX 1500

const int X_STEP_PIN = 54;
const int X_DIR_PIN = 55;
const int X_ENABLE_PIN = 38;

const int Y_STEP_PIN = 60;
const int Y_DIR_PIN = 61;
const int Y_ENABLE_PIN = 56;

const int Z_STEP_PIN = 46;
const int Z_DIR_PIN = 48;
const int Z_ENABLE_PIN = 62;

const int X_MIN_PIN = 3;
const int Y_MIN_PIN = 14;
const int Z_MIN_PIN = 18;

int x_brush_1 = 5*50; int x_brush_2 = 22*50;
int y_brush_1=1*50; int y_brush_2=14*50; int y_brush_3=28*50;
long pos_color_00[2] = {28*50,1}; long pos_color_01[2] = {31*50,1}; //{x,y}
long pos_color_10[2] = {28*50,26*50}; long pos_color_11[2] = {31*50,26*50};
long pos_moboo[2] = {};
long pos_groove[2] = {};

int z_floor=8*50; int z_color=8*50; int z_groove=100; int z_mid=4*50;

AccelStepper stepperX(1, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(1, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(1, Z_STEP_PIN, Z_DIR_PIN);

// MultiStepper steppers;
// MultiStepper stepperXY;

void setup() {

  Serial.begin(9600);
  
  pinMode(X_MIN_PIN, INPUT_PULLUP);
  pinMode(Y_MIN_PIN, INPUT_PULLUP);
  pinMode(Z_MIN_PIN, INPUT_PULLUP);


  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);

  stepperY.setMaxSpeed(200);
  stepperY.setAcceleration(100);
  stepperY.moveTo(200);  // 應該往遠離開關方向走
  while(stepperY.distanceToGo() != 0) stepperY.run();

}



void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(X_MIN_PIN));
  delay(500);
}
