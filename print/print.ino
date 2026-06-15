#include <AccelStepper.h>
#include <MultiStepper.h>

#define X_MAX 1950
#define Y_MAX 1200
#define Z_MAX 1200

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

int x_brush_1 = 2*50; int x_brush_2 = 18*50;
int y_brush_1=1*50; int y_brush_2=9*50; int y_brush_3=21*50;
long pos_color_00[2] = {21.5*50,3*50}; long pos_color_01[2] = {24.5*50,3*50}; //{x,y}
long pos_color_10[2] = {21.5*50,19*50}; long pos_color_11[2] = {24.5*50,19*50};
long pos_moboo[2] = {28*50,13*50};
long pos_groove[2] = {40*50,13*50};

int z_floor=900; int z_color=900; int z_groove=1100; int z_mid=0;

AccelStepper stepperX(1, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(1, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(1, Z_STEP_PIN, Z_DIR_PIN);

MultiStepper steppers;
MultiStepper stepperXY;

void setup() {
  
  pinMode(X_MIN_PIN, INPUT_PULLUP);
  pinMode(Y_MIN_PIN, INPUT_PULLUP);
  pinMode(Z_MIN_PIN, INPUT_PULLUP);


  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);

  homeAxis(stepperX, X_MIN_PIN, -1);
  homeAxis(stepperY, Y_MIN_PIN, -1);
  homeAxis(stepperZ, Z_MIN_PIN, -1);

  stepperX.setMaxSpeed(400);
  stepperY.setMaxSpeed(400);
  stepperZ.setMaxSpeed(400);

  stepperXY.addStepper(stepperX);
  stepperXY.addStepper(stepperY);

  steppers.addStepper(stepperX);
  steppers.addStepper(stepperY);
  steppers.addStepper(stepperZ);

}

void loop() {
  Paint(pos_color_11,pos_color_01,pos_color_10);
  while(true){};
}

void Paint(long* color1, long* color21, long* color22) {

  //沾顏料（一）
  Color(color1);

  //上色（一）
  Draw();

  //水槽
  groove();

  //moboo
  moboo();

  //沾顏料（二）
  Color(color21);
  Color(color22);

  //上色（二）
  Draw();

}

void Color(long* color) {
  moveXY(color);
  moveZ(z_color);
  // int k=50;
  // for(int i=0;i<2;i++) {
  //   moveX(color[0]+k);
  //   moveX(color[0]-k);
  // }
  moveZ(0);
}

void Draw() {
  moveXY(x_brush_1, y_brush_1);
  moveZ(z_floor);
  moveXY(x_brush_2, y_brush_1);
  moveZ(z_mid);
  moveXY(x_brush_2, y_brush_2);
  moveZ(z_floor);
  moveXY(x_brush_1, y_brush_2);
  moveZ(z_mid);
  moveXY(x_brush_1, y_brush_3);
  moveZ(z_floor);
  moveXY(x_brush_2, y_brush_3);
  moveZ(z_mid);
}

void moboo() {
  moveXY(pos_moboo);
  moveZ(z_floor);
  moveZ(z_mid);
}

void groove() {
  moveXY(pos_groove);
  moveZ(z_groove);
  moveZ(z_mid);
}

void homeAxis(AccelStepper &stepper, int homingPin, int direction) {
  
  
  stepper.setMaxSpeed(400);      
  stepper.setAcceleration(300);
  
  
  stepper.moveTo(100000 * direction); 

  
  while (digitalRead(homingPin) == LOW) {
    stepper.run();
  }

  stepper.stop();
  stepper.setCurrentPosition(0);
  delay(200); 

  
  // stepper.setMaxSpeed(100); 
  
  
  // stepper.moveTo(200 * -direction);
  // while (stepper.distanceToGo() != 0) {
  //   stepper.run();
  // }
  
  // // 重新慢速壓向開關
  // stepper.moveTo(100000 * direction);
  // while (digitalRead(homingPin) == LOW) {
  //   stepper.run();
  // }
  
  // stepper.stop();
  // stepper.setCurrentPosition(0); // 將此處定義為絕對座標 0
  // delay(200);
}

void test1(AccelStepper& stepper) {
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  stepper.moveTo(800);
  while(stepper.distanceToGo() != 0) {
    stepper.run();
  }

  stepper.moveTo(0);
  while(stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void moveSafe(int targetX, int targetY, int targetZ) {
  int safeX = constrain(targetX, 0, X_MAX);
  int safeY = constrain(targetY, 0, Y_MAX);
  int safeZ = constrain(targetZ, 0, Z_MAX);

  long position[3];
  position[0] = safeX;
  position[1] = safeY;
  position[2] = safeZ;

  steppers.moveTo(position);
  steppers.runSpeedToPosition();
}

void moveZ(int targetZ) {
  int safeZ = constrain(targetZ, 0, Z_MAX);
  stepperZ.setAcceleration(300);

  stepperZ.moveTo(safeZ);
  while(stepperZ.distanceToGo() != 0) {
    stepperZ.run();
  }
}
void moveY(int targetY) {
  int safeY = constrain(targetY, 0, Y_MAX);
  stepperY.setAcceleration(300);
  
  stepperY.moveTo(safeY);
  while(stepperY.distanceToGo() != 0) {
    stepperY.run();
  }
}
void moveX(int targetX) {
  int safeX = constrain(targetX, 0, X_MAX);
  stepperX.setAcceleration(300);
  
  stepperX.moveTo(safeX);
  while(stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
}

void moveXY(int targetX, int targetY) {
  int safeX = constrain(targetX, 0, X_MAX);
  int safeY = constrain(targetY, 0, Y_MAX);

  long position[2];
  position[0] = safeX;
  position[1] = safeY;

  stepperXY.moveTo(position);
  stepperXY.runSpeedToPosition();
}

void moveXY(long* position) {
  position[0] = constrain(position[0], 0, X_MAX);
  position[1] = constrain(position[1], 0, Y_MAX);

  stepperXY.moveTo(position);
  stepperXY.runSpeedToPosition();
}


