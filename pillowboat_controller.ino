#include <ESP32Servo.h>
#include "ESC.h"
// #include <Ps3Controller.h>
#include <PS4Controller.h>


#define SERVO_PIN 12
#define ESC_PILLOW_PIN 13
#define ESC_MOVE_PIN 16

#define PILLOW_SPEED_MIN 0
#define PILLOW_SPEED_MAX 1000

#define MOVE_SPEED_MIN 0
#define SERVO_ZERO_POSITION 90

#define JOYSTICK_LEFT_MIN_VALUE -5
#define JOYSTICK_RIGHT_MIN_VALUE 5


#define BT_ADDR "70:66:55:6c:3c:f4"

ESC esc_pillow;
ESC esc_move;
Servo dir_servo;

int x = 0, y = 0;
int mapped_servo_position = 0, mapped_esc_speed = 0;

void setup() {

  Serial.begin(115200);

  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.begin(BT_ADDR);

  dir_servo.setPeriodHertz(50);
  dir_servo.attach(SERVO_PIN, 100, 1700);

  while (!PS4.isConnected()) {
    Serial.println("Joystick not connected");
    delay(500);
  }

  //  ESP32PWM::allocateTimer(0);
  // ESP32PWM::allocateTimer(1);
  // ESP32PWM::allocateTimer(2);
  // ESP32PWM::allocateTimer(3);

  esc_pillow.attach(ESC_PILLOW_PIN);
  esc_pillow.setSpeed(0);

  esc_move.attach(ESC_MOVE_PIN);
  esc_move.setSpeed(0);


//  Serial.println("Ready.");

//
//  delay(1000);
//  esc_pillow.setSpeed(1000);
//  esc_move.setSpeed(1000);
//
//  delay(1000);
//
//  esc_pillow.setSpeed(0);
//  esc_move.setSpeed(0);
}

void loop() {
  //  if (!Ps3.isConnected()) {
  //    esc_pillow.setSpeed(0);
  //    esc_move.setSpeed(0);
  //  }
}

void onConnect() {
//  Serial.println("Connected to joystick.");
  x = 0;
  y = 0;
}

bool pillow_enabled = false;

void notify() {
  //  Serial.println("Got joystick event");
  if (PS4.data.button.touchpad) {
//    Serial.print("Started pressing the touchpad button, pillow ");
//    Serial.println(pillow_enabled);
    if (pillow_enabled) {
      // выключим подушку и мотор
      esc_pillow.setSpeed(PILLOW_SPEED_MIN);
      esc_move.setSpeed(MOVE_SPEED_MIN);
    } else {
      // включим
      esc_pillow.setSpeed(PILLOW_SPEED_MAX);
    }
    pillow_enabled = !pillow_enabled;
  }


  // int dx = Ps3.event.analog_changed.stick.lx;
  // int dy = Ps3.event.analog_changed.stick.ly;
  // if ((abs(dx) + abs(dy)) > 2) {
  //   Serial.print("Moved the left stick:");
  //   Serial.print(" dx="); Serial.print(dx, DEC);
  //   Serial.print(" dy="); Serial.print(dy, DEC);
  //   Serial.println();
  //   x = (x + dx) % 128;
  //   y = (y + dy) % ;
  //   Serial.print("Left stick position:");
  //   Serial.print(" x="); Serial.print(x, DEC);
  //   Serial.print(" y="); Serial.print(y, DEC);
  //   Serial.println();
  // }
  y = PS4.data.analog.stick.ly;

//  Serial.print("Left stick position:");
//  Serial.print(" y=");
//  Serial.print(y, DEC);
//  Serial.println();

  if (y < JOYSTICK_LEFT_MIN_VALUE) {  // левый джойстик вперед дает отрицательные значения, в дефайне -5
    mapped_esc_speed = map(
      -y,
      0, 128,
      300, 1000);
//    Serial.print("Mapped raw left joystick y=");
//    Serial.print(y);
//    Serial.print(" to ");
//    Serial.print(mapped_esc_speed);
//    Serial.println(" move esc speed");
    esc_move.setSpeed(mapped_esc_speed);
  } else {
    esc_move.setSpeed(MOVE_SPEED_MIN);
  }


  // direction servo control

  x = PS4.data.analog.stick.rx;

//  Serial.print("Right stick position:");
//  Serial.print(" x=");
//  Serial.print(x, DEC);
//  Serial.println();

  if (abs(x) > JOYSTICK_RIGHT_MIN_VALUE) {
    mapped_servo_position = map(
      128 + x,
      0, 255,
      0, 180);
//    Serial.print("Mapped raw right joystick x=");
//    Serial.print(x);
//    Serial.print(" to ");
//    Serial.print(mapped_servo_position);
//    Serial.println("degree servo position");
    dir_servo.write(mapped_servo_position);
  } else {
    dir_servo.write(SERVO_ZERO_POSITION);
  }
}
