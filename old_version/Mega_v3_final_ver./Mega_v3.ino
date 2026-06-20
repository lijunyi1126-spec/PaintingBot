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

int x_brush_1 = 0*50; int x_brush_2 = 17*50;
int y_brush_1=4.5*50; int y_brush_2=13.5*50; int y_brush_3=22.5*50;
long pos_color_00[2] = {20.1*50,3.5*50}; long pos_color_01[2] = {24.8*50,3.5*50}; //{x,y}
long pos_color_10[2] = {20.1*50,19.5*50}; long pos_color_11[2] = {24.8*50,19.5*50};
long pos_moboo_1[2] = {29*50,6*50}; long pos_moboo_2[2] = {29*50,19*50};
long pos_groove_2[2] = {40*50,3.5*50}; long pos_groove_1[2] = {40*50,19.5*50};

int z_floor=950; int z_color=850; int z_groove=1100; int z_mid=0;

AccelStepper stepperX(1, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(1, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(1, Z_STEP_PIN, Z_DIR_PIN);

MultiStepper steppers;
MultiStepper stepperXY;

long* selectedColor1  = pos_color_11;  // 預設
long* selectedColor2a = pos_color_00;
long* selectedColor2b = pos_color_01;
bool waitingToStart = true;
int currentStep = 0;
bool emergencyStop = false;
bool isBrushing = false;

void setup() {
  Serial2.begin(9600);  // 和 ESP32 溝通
  pinMode(X_MIN_PIN, INPUT_PULLUP);
  pinMode(Y_MIN_PIN, INPUT_PULLUP);
  pinMode(Z_MIN_PIN, INPUT_PULLUP);


  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);
  
  homeAxis(stepperZ, Z_MIN_PIN, -1);
  homeAxis(stepperX, X_MIN_PIN, -1);
  homeAxis(stepperY, Y_MIN_PIN, -1);
  

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
  if (Serial2.available()) {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("COLOR1:"))  selectedColor1 = getColorPos(cmd.substring(7));
    if (cmd.startsWith("COLOR2:")) selectedColor2a = getColorPos(cmd.substring(7));
    if (cmd.startsWith("COLOR3:")) selectedColor2b = getColorPos(cmd.substring(7));

    if (cmd == "START" && waitingToStart && !emergencyStop) {
      digitalWrite(X_ENABLE_PIN, LOW);
      digitalWrite(Y_ENABLE_PIN, LOW);
      digitalWrite(Z_ENABLE_PIN, LOW);
      delay(100);
      homeAxis(stepperZ, Z_MIN_PIN, -1);
      homeAxis(stepperX, X_MIN_PIN, -1);
      homeAxis(stepperY, Y_MIN_PIN, -1);
      waitingToStart = false;
      runPaint();
    }
    if (cmd == "STOP" || emergencyStop ) {
      digitalWrite(X_ENABLE_PIN, HIGH);
      digitalWrite(Y_ENABLE_PIN, HIGH);
      digitalWrite(Z_ENABLE_PIN, HIGH);
      waitingToStart = true;
      emergencyStop == false;
    }
  }
}

long* getColorPos(String name) {
  if (name == "COLOR_00") return pos_color_00;
  if (name == "COLOR_01") return pos_color_01;
  if (name == "COLOR_10") return pos_color_10;
  return pos_color_11;
}

void reportStep(int step, String msg) {
  currentStep = step;
  Serial2.println("STEP:" + String(step));
  Serial2.println("STATUS:" + msg);
}

void runPaint() {
  emergencyStop = false;
  
  reportStep(1, "第一次沾顏料");
  Color(selectedColor1);
  if (emergencyStop) return;

  //moboo4();

  reportStep(2, "第一次繪製");
  Draw();
  if (emergencyStop) return;
  Color(selectedColor1);
  if (emergencyStop) return;
  DrawCounter();
  if (emergencyStop) return;

  reportStep(3, "清洗筆刷");
  groove(pos_groove_1);
  if (emergencyStop) return;

  reportStep(4, "吸去殘餘水分");
  moboo();
  if (emergencyStop) return;
  moboo();
  if (emergencyStop) return;

  

  reportStep(5, "第二次沾顏料");
  Color(selectedColor2a);  // Color() 保持原樣，呼叫兩次
  
  if (emergencyStop) return;

  Wet(pos_groove_2);
  if (emergencyStop) return;
  moboo2();
  if (emergencyStop) return;

  reportStep(6, "第二次繪製");
  Draw2();
  groove(pos_groove_2);
  moboo();
  moboo();

  reportStep(7, "第三次繪製");
  Color(selectedColor2b);
  Wet(pos_groove_2);
  moboo3();
  Draw3();
  if (emergencyStop) return;
  
  // Color(selectedColor2b);
  // //Color(selectedColor2a);
  // if (emergencyStop) return;
  // DrawCounter();

  reportStep(8, "完成！");
  waitingToStart = true;
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
void Color(long* color) {
  moveXY(color);
  moveSafe(color[0], color[1], z_color);
  int k=100;
  moveSafe(color[0]+k,color[1],z_color-k*3);
  delay(100);
  moveSafe(color[0], color[1], z_color);
  delay(100);
  moveSafe(color[0]-k, color[1], z_color-k*3);
  delay(100);
  moveSafe(color[0], color[1], z_color);
  delay(100);
  moveSafe(color[0]+k,color[1],z_color-k*3);
  delay(100);


  moveSafe(color[0], color[1], z_mid);

//左右
  // int b = 300;
  // moveSafe(color[0],color[1],z_color);
  // moveSafe(color[0], color[1]+b, z_color+100);
  // delay(100);
  // moveSafe(color[0], color[1]-b, z_color+100);
  // delay(100);
  // moveSafe(color[0], color[1]+b, z_color+100);
  // delay(100);
  // moveSafe(color[0], color[1]-b, z_color+100);
  // delay(100);
  // moveSafe(color[0], color[1], z_mid);
}
void Color1_2(long* color) {
  moveXY(color);
  moveSafe(color[0], color[1], z_color);
  int k=100; int a = 50;
  moveSafe(color[0]+k+a,color[1],z_color-k*3);
  delay(100);
  moveSafe(color[0]+a, color[1], z_color);
  delay(100);
  moveSafe(color[0]-k+a, color[1], z_color-k*3);
  delay(100);
  moveSafe(color[0]+a, color[1], z_color);
  delay(100);
  moveSafe(color[0]+k+a,color[1],z_color-k*3);
  delay(100);

  // int b = 200;
  // moveSafe(color[0]+a,color[1],z_color);
  // moveSafe(color[0]+a, color[1]+b, z_color+100);
  // delay(100);
  // moveSafe(color[0]+a, color[1]-b, z_color+100);
  // delay(100);
  // moveSafe(color[0]+a, color[1]+b, z_color+100);
  // delay(100);
  // moveSafe(color[0]+a, color[1]-b, z_color+100);
  // delay(100);
  // moveSafe(color[0]+a, color[1], z_mid);

  moveSafe(color[0], color[1], z_mid);
}

void Color2(long* color) {
  int k=70;
  moveSafe(color[0]+k, color[1], 0);
  moveSafe(color[0]+k, color[1], z_color);
  moveSafe(color[0]+k, color[1], z_mid);
}

void Draw() {
  int k = 100;
  moveXY(x_brush_1, y_brush_1);
  moveSafe(x_brush_1, y_brush_1,z_floor-k-60);
  moveSafe(x_brush_2+50, y_brush_1,z_floor-80);
  moveSafe(x_brush_2+50, y_brush_1,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_floor-k-60+40-10);
  moveSafe(x_brush_2+50, y_brush_2, z_floor-40+40-10);
  moveSafe(x_brush_2+50, y_brush_2, z_mid);
  moveSafe(x_brush_1, y_brush_3,z_mid);
  moveSafe(x_brush_1, y_brush_3,z_floor-k-60+60+20-20);
  moveSafe(x_brush_2+50, y_brush_3,z_floor-40+60+20-20);
  moveSafe(x_brush_2+50, y_brush_3, 0);
}

void DrawCounter() {
  int k = 100;
  moveXY(x_brush_2, y_brush_3);
  moveSafe(x_brush_2, y_brush_3,z_floor-k-60+60);
  moveSafe(x_brush_1, y_brush_3,z_floor-40+60);
  moveSafe(x_brush_1, y_brush_3,z_mid);
  moveSafe(x_brush_2, y_brush_2,z_mid);
  moveSafe(x_brush_2, y_brush_2,z_floor-k-60+40-20);
  moveSafe(x_brush_1, y_brush_2, z_floor-40-20);
  moveSafe(x_brush_1, y_brush_2, z_mid);
  moveSafe(x_brush_2, y_brush_1,z_mid);
  moveSafe(x_brush_2, y_brush_1,z_floor-k-60+40-20);
  moveSafe(x_brush_1, y_brush_1,z_floor-40+40-20);
  moveSafe(x_brush_1, y_brush_1, 0);

  // moveSafe(x_brush_2, y_brush_3,z_floor-k-60);
  // moveSafe(x_brush_1, y_brush_3,z_floor-80+40);
  // moveSafe(x_brush_1, y_brush_3,z_mid);
  // moveSafe(x_brush_2-50, y_brush_2,z_mid);
  // moveSafe(x_brush_2-50, y_brush_2,z_floor-k-60+40);
  // moveSafe(x_brush_1, y_brush_3, z_floor-40+40);
  // moveSafe(x_brush_1, y_brush_3, z_mid);
  // moveSafe(x_brush_2-50, y_brush_1,z_mid);
  // moveSafe(x_brush_2-50, y_brush_1,z_floor-k-60+60);
  // moveSafe(x_brush_1, y_brush_2,z_floor-40+60);
  // moveSafe(x_brush_1, y_brush_2, 0);
}
void Draw2() {
  int k = 100; int a = 25; int b = 50;
  moveXY(x_brush_1, y_brush_1+a);
  moveSafe(x_brush_1, y_brush_1+a,z_floor-k-60+40+20+60-20-20-20);
  moveSafe(x_brush_2+50, y_brush_1+a+b,z_floor-80+40+40+20+60-20-20-20);
  moveSafe(x_brush_2+50, y_brush_1+a+b,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_floor-k-60+40+40+20+60-20-20-20);
  moveSafe(x_brush_2+50, y_brush_2+b, z_floor-40+40+40+60+20-20-20-20);
  moveSafe(x_brush_2+50, y_brush_2+b, z_mid);
  moveSafe(x_brush_1, y_brush_3-a,z_mid);
  moveSafe(x_brush_1, y_brush_3-a,z_floor-k-60+40+40+20+60-20-20-20);
  moveSafe(x_brush_2+50, y_brush_3-a+b,z_floor-40+40+40+20+60-20-20-20);
  moveSafe(x_brush_2+50, y_brush_3-a+b, 0);
}

void Draw3() {
  int k = 100; int a = 25; int b = 20; int c = 50;
  moveXY(x_brush_1, y_brush_3+a+b);
  moveSafe(x_brush_1, y_brush_3+a+b,z_floor-k-60+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_3+a+b-c,z_floor-80+40+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_3+a+b-c,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_mid);
  moveSafe(x_brush_1, y_brush_2,z_floor-k-60+40+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_2-c, z_floor-40+40+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_2-c, z_mid);
  moveSafe(x_brush_1, y_brush_1-a-b,z_mid);
  moveSafe(x_brush_1, y_brush_1-a-b,z_floor-k-60+40+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_1-a-b-c,z_floor-40+40+40+80-40-20);
  moveSafe(x_brush_2+50, y_brush_1-a-b-c, 0);
}

void Draw2_1() {
  moveXY(x_brush_1+50, y_brush_1);
  moveSafe(x_brush_1+50, y_brush_1,z_floor-140);
  moveSafe(x_brush_2+100, y_brush_1,z_floor+80);
  moveSafe(x_brush_2+100, y_brush_1,z_mid);
  moveSafe(x_brush_1+50, y_brush_2,z_mid);
  moveSafe(x_brush_1+50, y_brush_2,z_floor-50);
  moveSafe(x_brush_2+100, y_brush_2, z_floor+80);
  moveSafe(x_brush_2+100, y_brush_2, z_mid);
}

void Draw2_2() {

  moveXY(x_brush_1+50, y_brush_2);
  moveSafe(x_brush_1+50, y_brush_2,z_floor-140);
  moveSafe(x_brush_2+100, y_brush_2,z_floor+80);
  moveSafe(x_brush_2+100, y_brush_2,z_mid);
  moveSafe(x_brush_1+50, y_brush_3,z_mid);
  moveSafe(x_brush_1+50, y_brush_3,z_floor-50);
  moveSafe(x_brush_2+100, y_brush_3, z_floor+80);
  moveSafe(x_brush_2+100, y_brush_3, z_mid);
}
void Draw2_3() {

  moveXY(x_brush_1+50, y_brush_3);
  moveSafe(x_brush_1+50, y_brush_3,z_floor-140);
  moveSafe(x_brush_2+100, y_brush_3,z_floor+80);
  moveSafe(x_brush_2+100, y_brush_3,z_mid);
  moveSafe(x_brush_1+50, y_brush_3,z_mid);
  moveSafe(x_brush_1+50, y_brush_3,z_floor-50);
  moveSafe(x_brush_2+100, y_brush_3, z_floor+80);
  moveSafe(x_brush_2+100, y_brush_3, z_mid);
}

void moboo() {
  moveXY(pos_moboo_1);
  moveSafe(pos_moboo_1[0], pos_moboo_1[1], z_floor);
  int k=150; int a = 100;
  
  moveSafe(pos_moboo_1[0],pos_moboo_1[1],z_floor-k);
  moveSafe(pos_moboo_1[0],pos_moboo_1[1],z_floor);
  moveSafe(pos_moboo_1[0],pos_moboo_1[1],z_mid);
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor);
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor-k);
  moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_floor);
  moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_mid);

  // moveSafe(pos_moboo_1[0]+k,pos_moboo_1[1], z_mid);
  // moveSafe(pos_moboo_1[0]+k,pos_moboo_1[1], z_floor-k);
  // moveSafe(pos_moboo_1[0]-k,pos_moboo_1[1],z_floor);
  // moveSafe(pos_moboo_1[0]-k,pos_moboo_1[1],z_floor-k);
  // moveSafe(pos_moboo_1[0]-k,pos_moboo_1[1],z_floor+a);


  // moveSafe(pos_moboo_2[0]+k,pos_moboo_2[1],z_mid);
  // moveSafe(pos_moboo_2[0]+k,pos_moboo_2[1],z_floor-k);
  // moveSafe(pos_moboo_2[0]-k,pos_moboo_2[1],z_floor);
  // moveSafe(pos_moboo_2[0]-k,pos_moboo_2[1],z_floor-k);
  // moveSafe(pos_moboo_2[0]-k, pos_moboo_2[1], z_floor+a);


  //moveSafe(pos_moboo_2[0]-k, pos_moboo_2[1], z_mid);
}

void moboo2() {
  moveXY(pos_moboo_1);
  moveSafe(pos_moboo_1[0], pos_moboo_1[1], z_floor);
  int k=150; int a = 100;
  moveSafe(pos_moboo_1[0],pos_moboo_1[1],z_floor-k);
  moveSafe(pos_moboo_1[0], pos_moboo_1[1], z_floor);
  moveSafe(pos_moboo_1[0],pos_moboo_1[1],z_mid);
  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);
  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor);
  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);

  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);
  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor);
  // moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor-k);
  // moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_floor);
  // moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_mid);
}
void moboo3() {
  moveXY(pos_moboo_2);
  moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_floor);
  int k=150; int a = 100;
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_floor-k);
  moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_floor);
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);
}
void moboo4() {
  moveXY(pos_moboo_2);
  moveSafe(pos_moboo_2[0], pos_moboo_2[1], z_floor);
  moveSafe(pos_moboo_2[0],pos_moboo_2[1],z_mid);
}

void groove(long* pos_groove_1) {
  moveXY(pos_groove_1);
  moveSafe(pos_groove_1[0], pos_groove_1[1], z_groove);
  int k=300;

  moveXY(pos_groove_1[0]+k,pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]-k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]+k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]-k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]+k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]-k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]+k, pos_groove_1[1]);
  delay(200);
  moveXY(pos_groove_1[0]-k, pos_groove_1[1]);

  moveSafe(pos_groove_1[0]-k, pos_groove_1[1], z_mid);
}

void Wet(long* pos_groove) {
  moveXY(pos_groove);
  moveSafe(pos_groove[0], pos_groove[1], z_floor-20);
  
  moveSafe(pos_groove[0], pos_groove[1], z_mid);
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

  reportPosition();
}

void moveZ(int targetZ) {
  int safeZ = constrain(targetZ, 0, Z_MAX);
  stepperZ.setAcceleration(300);

  stepperZ.moveTo(safeZ);
  while(stepperZ.distanceToGo() != 0) {
    if (emergencyStop) return;
    checkSerial();
    stepperZ.run();
  }

  reportPosition();
}
void moveY(int targetY) {
  int safeY = constrain(targetY, 0, Y_MAX);
  stepperY.setAcceleration(300);
  
  stepperY.moveTo(safeY);
  while(stepperY.distanceToGo() != 0) {
    if (emergencyStop) return;
    checkSerial();
    stepperY.run();
  }

  reportPosition();
}
void moveX(int targetX) {
  int safeX = constrain(targetX, 0, X_MAX);
  stepperX.setAcceleration(300);
  
  stepperX.moveTo(safeX);
  while(stepperX.distanceToGo() != 0) {
    if (emergencyStop) return;
    checkSerial();
    stepperX.run();
  }

  reportPosition();
}

void moveXY(int targetX, int targetY) {
  int safeX = constrain(targetX, 0, X_MAX);
  int safeY = constrain(targetY, 0, Y_MAX);

  long position[2];
  position[0] = safeX;
  position[1] = safeY;

  stepperXY.moveTo(position);
  stepperXY.runSpeedToPosition();
  if (emergencyStop) return;
  checkSerial();

  reportPosition();
}

void moveXY(long* position) {
  position[0] = constrain(position[0], 0, X_MAX);
  position[1] = constrain(position[1], 0, Y_MAX);

  stepperXY.moveTo(position);
  stepperXY.runSpeedToPosition();
  if (emergencyStop) return;
  checkSerial();

  reportPosition();
}

void checkSerial() {
  if (Serial2.available()) {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim();
    if (cmd == "STOP") {
      emergencyStop = true;
      digitalWrite(X_ENABLE_PIN, HIGH);
      digitalWrite(Y_ENABLE_PIN, HIGH);
      digitalWrite(Z_ENABLE_PIN, HIGH);
    }
  }
}

void reportPosition() {
  Serial2.println("POS:" + String(stepperX.currentPosition()) + "," + String(stepperY.currentPosition()));
}