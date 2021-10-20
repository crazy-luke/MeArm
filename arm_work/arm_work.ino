/*
 * arm_work
 *
 *  Created on: Sep 19, 2021
 *      Author: crazy-luke
 */
#include <Servo.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#include <IRremote.h>
#define  IR_RECV_PIN 5
IRrecv irrecv(IR_RECV_PIN);   // 红外遥控初始化
decode_results results;   // 储存接收到的红外遥控信息

/*
  4个 Servo 对象
  baseServo   底盘  引脚11
  rearServo   后臂  引脚10
  frontaServo 前臂  引脚9
  clawServo   钳子  引脚6
*/
Servo baseServo, rearServo, frontServo, clawServo;

//最大活动范围，保护设备
const int PROGMEM baseMin = 0;
const int PROGMEM baseMax = 180;
const int PROGMEM rearMin = 40;
const int PROGMEM rearMax = 170;
const int PROGMEM frontMin = 40;
const int PROGMEM frontMax = 130;
const int PROGMEM clawMin = 0;
const int PROGMEM clawMax = 90;

const int PROGMEM SPEED = 15; //转动的速度
const int PROGMEM MA = 2;   //每次转动的角度

//电机动作
enum Act {Up, Down, Left, Right, Front, Back, Close, Open };

void setup() {
  // put your setup code here, to run once:
  baseServo.attach(11);
  delay(200);
  rearServo.attach(10);
  delay(200);
  frontServo.attach(9);
  delay(200);
  clawServo.attach(6);
  delay(200);

  armInitAngle();
  Serial.begin(9600);
  mySerial.begin(9600);
  irrecv.enableIRIn();
  Serial.println(F("ready"));
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    serialCmd(false);
  }

  if (mySerial.available()) {
    serialCmd(true);
  }

  if (irrecv.decode(&results)) {
    iptvIRCmd(results.value);
    irrecv.resume(); // 恢复接收下一个红外遥控信号
  }

  delay(100);
}

//------------ 解析指令 -----------------------------

/**
   执行串口监视器指令
*/
void serialCmd(bool soft) {
  char cmd = soft ? mySerial.read() : Serial.read();
  if (cmd == 'b' || cmd == 'r' || cmd == 'f' || cmd == 'c' )
  { // b22 base转到22， r22 rear转到22， f22 front转到22， c22 claw转到22
    int to = soft ? mySerial.parseInt() : Serial.parseInt();
    work(cmd, to);
  }
  else if (cmd == 'a' || cmd == 'd' || cmd == 's' || cmd == 'w' || cmd == 'q' || cmd == 'e' || cmd == 'z' || cmd == 'x')
  { // a左, d右, s前, w后, q上, e下, z开, x关
    switch (cmd) {
      case'a':
        workAction(Act::Left);
        break;

      case'd':
        workAction(Act::Right);
        break;

      case's':
        workAction(Act::Back);
        break;

      case'w':
        workAction(Act::Front);
        break;

      case'q':
        workAction(Act::Up);
        break;

      case'e':
        workAction(Act::Down);
        break;

      case'z':
        workAction(Act::Open);
        break;

      case'x':
        workAction(Act::Close);
        break;
    }
  }
  else if (cmd == 'o' || cmd == 'i' || cmd == 'l' || cmd == 'k')
  { // o 状态报告, i 初始化角度,  l 我的组合
    switch (cmd) {
      case 'o':
        reportStatus();
        break;
      case 'i':
        armInitAngle();
        break;
      case 'l':
        lukeWork();
        break;
      case 'k':
        adamWork();
        break;
    }
  }
  else
  { // 清理无效指令
    Serial.print(F("+Warning: Unknown Command! clear"));
    Serial.print(F(" char = ")); Serial.print((char)cmd);
    Serial.print(F(" int = ")); Serial.print((int)cmd);
    Serial.println(F(""));
    if (soft) {
      while (mySerial.available() > 0) mySerial.read(); //清除串口缓存的错误指令
      delay(20);
    } else {
      while (Serial.available() > 0) Serial.read(); //清除串口缓存的错误指令
      delay(20);
    }
    Serial.println(F("clear all cmd "));

  }
}

/**
 * 执行红外遥控器指定
 */
uint32_t lastIRvalue = 0;
//电信IPTV 遥控器
void iptvIRCmd(uint32_t value) {
  Serial.print(F("iptvIRCmd: ")); Serial.println(value, HEX); // results.value为红外遥控信号的具体数值
  if (value == 0xFFFFFFFF) {
    value = lastIRvalue;
  }

  switch (value) {
    case 0x8D72738C:  //确认 键
      armInitAngle();
      break;
    case 0x8D72C13E: //点播 键
      lukeWork();
      break;
    case 0x8D72B34C: //直播 键
      adamWork();
      break;
    case 0x8D72C33C: //信息键
      reportStatus();
      break;
    case 0x8D729966: //左 键
      workAction(Act::Left);
      break;
    case 0x8D72837C: //右 键
      workAction(Act::Right);
      break;
    case 0x8D7253AC: //上 键
      workAction(Act::Front);
      break;
    case 0x8D724BB4: //下 键
      workAction(Act::Back);
      break;
    case 0x8D7211EE: //首页 键
      workAction(Act::Up);
      break;
    case 0x8D721EE1: //互动 键
      workAction(Act::Down);
      break;
    case 0x8D72A956: //播放/暂停 键
      workAction(Act::Open);
      break;
    case 0x8D72A35C: //返回 键
      workAction(Act::Close);
      break;
    case 0x8D728976: //回看 键
      crazydummy();
      break;
    case 0x8D72BB44: //上页  键
      happyGame();
      break;
    case 0x8D720BF4:
      releaseClaw();
      break;

  }
  lastIRvalue = value;
}


//------------ 自定义 -----------------------------

/**
   luke 测试 组合序列
*/
void lukeWork() {
  int action[][2] = {
    //左边抓起
    {'b', 0},
    {'c', 0},
    {'r', 170},
    {'f', 110},
    {'c', 90},

    //回到原位
    {'r', 90},
    {'f', 90},

    //放下
    {'b', 120},
    {'r', 170},
    {'f', 110},
    {'c', 0},

    //回到原位
    {'r', 90},
    {'f', 90},
    {'c', 90},

    //去抓回来
    {'c', 0},
    {'b', 120},
    {'r', 170},
    {'f', 110},
    {'c', 90},

    //回到原位
    {'r', 90},
    {'f', 90},
    {'c', 90},

    //放下
    {'b', 0},
    {'r', 170},
    {'f', 110},
    {'c', 0},

    //回到原位
    {'r', 90},
    {'f', 90},
    {'c', 90},
    {'b', 90},
  };
  workGroup(action, length(action));
}


void adamWork() {
  int action[][2] = {

    {'b', 106},
    {'r', 110},
    {'f', 78},
    {'c', 20},

    {'b', 106},
    {'r', 160},
    {'f', 78},
    {'c', 90},


    {'r', 90},
    {'f', 90},
    {'b', 90},

    {'b', 110},
    {'b', 70},
    {'b', 90},
    {'r', 110},
    {'f', 110},
    {'r', 70},
    {'f', 70},
    {'r', 90},
    {'f', 90},

    {'f', 120},
    {'b', 38},
    {'r', 120},
    {'c', 20},

    {'c', 90},
    {'r', 90},
    {'f', 90},
    {'b', 90},

  };
  workGroup(action, length(action));
}

void releaseClaw() {
  work('c', 0);
  work('c', 90);
}

void crazydummy() {
  for (int a = 0; a <= 20; a++) {
    char cmd;
    int r = random(0, 4);
    switch (r) {
      case 0 :    cmd = 'b';    break;
      case 1 :    cmd = 'r';    break;
      case 2 :    cmd = 'f';    break;
      case 3 :    cmd = 'c';    break;

    }
    delay(10);
    int to = random(20, 160);
    work(cmd, to);
  }
}

void happyGame() {
  // ball 拿起球
  int actionBall[][2] = {
    {'b', 10},
    {'c', 2},
    {'f', 67},
    {'r', 144},

    {'c', 90},
    {'r', 90},
    {'f', 100},
    {'b', 90},
  };
  workGroup(actionBall, length(actionBall));
  delay(500);
  bool gameWin = false;
  while (!gameWin) {
    // 投球
    int b = 0;
    int r = random(0, 3);
    Serial.print(F("r=")); Serial.println(r);
    switch (r) {
      case 0: b = 54; break;
      case 1: b = 90; break;
      case 2: b = 144; break;
    }
    int action[][2] = {
      {'b', b},
      {'f', 10},
      {'r', 148},
    };
    workGroup(action, length(action));
    delay(800);

    //投球成功就结束游戏
    r = random(0, 200);
    Serial.print(F("gameWin=")); Serial.println(r);
    gameWin = r >= 140 ? true : false;

    if (gameWin) {
      int action[][2] = {
        {'f', 75},
        {'c', 0},
      };
      workGroup(action, length(action));
    }
  }

  armInitAngle();
}

/**
   初始化角度
*/
void armInitAngle() {
  Serial.println(F("start init"));
  work('r', 90);
  work('f', 90);
  work('c', 90);
  work('b', 90);
  Serial.println(F("init complete!"));
}

//------ 控制电机 ------

/**
  cmd 指令 ，可以填写 'b' 'f' 'c' 'r'
  to 角度
*/
void work(char cmd, int to) {

  Serial.print(F("Servo-")); Serial.print(cmd); Serial.print(F(" to angle=")); Serial.println(to);

  Servo workServo;

  switch (cmd) {

    case 'b':
      if (to < baseMin || to > baseMax) {
        Serial.println(F("+Warning:Base Servo Out of Limit"));
        return;
      }

      workServo = baseServo;
      break;

    case 'r':
      if (to < rearMin || to > rearMax) {
        Serial.println(F("+Warning:Rear Servo Out of Limit"));
        return;
      }
      workServo = rearServo;
      break;

    case 'f':
      if (to < frontMin || to > frontMax) {
        Serial.println(F("+Warning:Front Servo Out of Limit"));
        return;
      }
      workServo = frontServo;
      break;

    case 'c':
      if (to < clawMin || to > clawMax) {
        Serial.println(F("+Warning:Claw Servo Out of Limit"));
        return;
      }
      workServo = clawServo;
      break;
  }

  int from = workServo.read();
  if (from <= to) {
    for (int i = from; i <= to; i++) {
      workServo.write(i);
      delay(SPEED);
    }
  } else {
    for (int i = from; i >= to; i--) {
      workServo.write(i);
      delay(SPEED);
    }
  }

}

/**
   组合工作
*/
void workGroup(int action[][2], int len) {

  Serial.print(F("start workGroup:")); Serial.println(len);
  for (int i = 0; i < len; i++) {
    work(action[i][0], action[i][1]);
  }
  Serial.println(F("well done workGroup completion!"));
}

/**
   单步骤动作
   Left:  // baseServo 向左
   Right: // baseServo 向右
   Back:  // rearServo 向后
   Front: // rearServo 向前
   Up:    // frontServo 向上
   Down:  // frontServo 向下
   Open:  // clawServo 关闭
   Close: // clawServo 打开
*/
void workAction(Act act) {
  int to = 0;
  switch (act) {
    case Act::Left:
      Serial.println(F("Arm Action: Left"));
      to = baseServo.read() - MA;
      work('b', to);
      break;

    case Act::Right:
      Serial.println(F("Arm Action: Right"));
      to = baseServo.read() + MA;
      work('b', to);
      break;

    case Act::Back:
      Serial.println(F("Arm Action: Back"));
      to = rearServo.read() - MA;
      work('r', to);
      break;

    case Act::Front:
      Serial.println(F("Arm Action: Front"));
      to = rearServo.read() + MA;
      work('r', to);
      break;

    case Act::Up:
      Serial.println(F("Arm Action: Up"));
      to = frontServo.read() + MA;
      work('f', to);
      break;

    case Act::Down:
      Serial.println(F("Arm Action: Down"));
      to = frontServo.read() - MA;
      work('f', to);
      break;

    case Act::Open:
      Serial.println(F("Arm Action: Open"));
      to = clawServo.read() - MA;
      work('c', to);
      break;

    case Act::Close:
      Serial.println(F("Arm Action: Close"));
      to = clawServo.read() + MA;
      work('c', to);
      break;
  }
}

/**
   报告当前状态
*/
void reportStatus() {
  Serial.println(F("++++++ MeArm Status Report +++++"));
  Serial.print(F("clawAngel = ")); Serial.println(clawServo.read());
  Serial.print(F("baseAngle = ")); Serial.println(baseServo.read());
  Serial.print(F("rearAngle = ")); Serial.println(rearServo.read());
  Serial.print(F("frontAngle = ")); Serial.println(frontServo.read());
  Serial.println(F("++++++++++++++++++++++++++++++++++++"));
}

template<class T>
int length(T& arr)
{
  return sizeof(arr) / sizeof(arr[0]);
}
