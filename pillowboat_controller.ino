#include <ESP32Servo.h>
#include "ESC.h"
#include <PS4Controller.h>

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

#define SERVO_PIN 12
#define ESC_PILLOW_PIN 13
#define ESC_MOVE_PIN 16

#define PILLOW_SPEED_MIN 0
#define PILLOW_SPEED_MAX 1000

#define MOVE_SPEED_MIN 0
#define SERVO_ZERO_POSITION 90

#define JOYSTICK_LEFT_MIN_VALUE -5
#define JOYSTICK_RIGHT_MIN_VALUE 5

#define LED_BUILTIN 4
#define BT_CLEAR_BTN 15

#define BT_ADDR "24:d7:eb:10:1a:52"

ESC esc_pillow;
ESC esc_move;
Servo dir_servo;

int x = 0, y = 0;
int mapped_servo_position = 0;
int mapped_esc_speed = 0;
bool pillow_enabled = false;

// ----- bt_remove_paired_devices.ino parts -----
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove

#define PAIR_MAX_DEVICES 20
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];

bool initBluetooth()
{
  if(!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }
 
  if(esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }
 
  if(esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  return true;
}

char *bda2str(const uint8_t* bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}

// ----- bt_remove_paired_devices.ino parts end -----


void setup() {

  Serial.begin(115200);

  pinMode(BT_CLEAR_BTN, INPUT_PULLUP);
  if (digitalRead(BT_CLEAR_BTN) == 0) {
    pinMode(LED_BUILTIN, OUTPUT);

// ----- bt_remove_paired_devices.ino logic -----
// source: https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/bt_remove_paired_devices/bt_remove_paired_devices.ino

    initBluetooth();
    Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
    // Get the numbers of bonded/paired devices in the BT module
    int count = esp_bt_gap_get_bond_device_num();
    if (!count) {
      Serial.println("No bonded device found.");
    } else {
      Serial.print("Bonded device count: "); Serial.println(count);
      if (PAIR_MAX_DEVICES < count) {
        count = PAIR_MAX_DEVICES;
        Serial.print("Reset bonded device count: "); Serial.println(count);
      }
      esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
      if (ESP_OK == tError) {
        for (int i = 0; i < count; i++) {
          Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
          Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
          if (REMOVE_BONDED_DEVICES) {
            esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
            if (ESP_OK == tError) {
              Serial.print("Removed bonded device # ");
            } else {
              Serial.print("Failed to remove bonded device # ");
            }
            Serial.println(i);
          }
        }
      }
    }
   
// ----- bt_remove_paired_devices.ino logic end -----

    digitalWrite(LED_BUILTIN, 1);
    while (true) {}
  }

  PS4.attach(notify);
  //  PS4.attachOnConnect(onConnect);
  //  PS4.attachOnDisconnect(onDConnect);
  PS4.begin(BT_ADDR);

  dir_servo.setPeriodHertz(50);
  dir_servo.attach(SERVO_PIN, 100, 1700);

  while (!PS4.isConnected()) {
    Serial.println("Joystick not connected");
    delay(500);
  }

  esc_pillow.attach(ESC_PILLOW_PIN);
  esc_pillow.setSpeed(0);

  esc_move.attach(ESC_MOVE_PIN);
  esc_move.setSpeed(0);


  //  Serial.println("Ready.");

}

void loop() {
  if (!PS4.isConnected()) {
    mapped_esc_speed = MOVE_SPEED_MIN;
    mapped_servo_position = SERVO_ZERO_POSITION;
    pillow_enabled = false;
  }
  esc_move.setSpeed(mapped_esc_speed);
  dir_servo.write(mapped_servo_position);
  if (pillow_enabled) {
    // включим подушку и мотор
    esc_pillow.setSpeed(PILLOW_SPEED_MAX);
  } else {
    // выключим
    esc_pillow.setSpeed(PILLOW_SPEED_MIN);
  }
  delay(15);
}


//
//void onConnect() {
//  Serial.println("CONN");
//}
//
//void onDConnect() {
//  Serial.println("DCONN");
//}


void notify() {
  //    Serial.println("Got joystick event");
  if (PS4.data.button.touchpad) {
    //    Serial.print("Started pressing the touchpad button, pillow ");
    //    Serial.println(pillow_enabled);
    //    if (pillow_enabled) {
    //      // выключим подушку и мотор
    //      esc_pillow.setSpeed(PILLOW_SPEED_MIN);
    //      esc_move.setSpeed(MOVE_SPEED_MIN);
    //    } else {
    //      // включим
    //      esc_pillow.setSpeed(PILLOW_SPEED_MAX);
    //    }
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
    //    esc_move.setSpeed(mapped_esc_speed);
  } else {
    //    esc_move.setSpeed(MOVE_SPEED_MIN);
    mapped_esc_speed = MOVE_SPEED_MIN;
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
    //    dir_servo.write(mapped_servo_position);
  } else {
    //    dir_servo.write(SERVO_ZERO_POSITION);
    mapped_servo_position = SERVO_ZERO_POSITION;
  }
}
