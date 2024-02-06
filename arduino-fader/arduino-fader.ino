/*
pwmに使用できるピンは9,10(timer1)だけ
5,6(timer0)はシステムクロックと共有しているためPWM周波数を変更できない
3,11(timer2)はタイマー割り込み処理で使うためPWM周波数を変更できない
*/

#include <MsTimer2.h>
#include "PIDControl.h"
#include "moving_Average.hpp"

uint8_t analog_pin = A0;//モーターフェーダーのアナログ
uint8_t motor_pin[2] = { 9, 10 };

uint8_t vr_pin = A1;//普通のボリューム

PIDControl PID;//PID制御
Average Average_vr(30);//移動平均(30回平均)


void setup() {
  //PWM周波数を高速化設定(9pin,10pin)
  TCCR1B &= B11111000;
  TCCR1B |= B00000001;

//pinを出力に設定
  pinMode(motor_pin[0], OUTPUT);
  pinMode(motor_pin[1], OUTPUT);

//PIDパラメータの設定
  PID.set_kp(12);
  PID.set_ki(0.7);
  PID.set_kd(1);

  //タイマー割り込み5msサイクル
  MsTimer2::set(5, slide_control);
  MsTimer2::start();
  delay(1000);

  PID.set_target(0);
  delay(1000);
  PID.set_target(1000);
  delay(1000);
  PID.set_target(200);
  delay(1000);
  PID.set_target(50);
  delay(1000);
  PID.set_target(500);
  delay(1000);
  PID.set_target(600);
  delay(1000);
  PID.set_target(200);
  delay(1000);
  PID.set_target(800);
  delay(1000);
}

//割り込み処理
void slide_control() {
  PID.set_input(analogRead(analog_pin));
  motor_set(PID.pidCalculation());  // PIDでモータを制御
}

//モーター出力処理
void motor_set(long i) {
  if (i > 5) {
    analogWrite(motor_pin[0], i);
    analogWrite(motor_pin[1], 0);
  } else if (i < -5) {
    analogWrite(motor_pin[0], 0);
    analogWrite(motor_pin[1], abs(i));
  } else {
    analogWrite(motor_pin[0], 255);
    analogWrite(motor_pin[1], 255);
  }
}

void loop() {
  Average_vr.input(analogRead(vr_pin));
  PID.set_target(Average_vr.read_average());

  delay(1);
}
