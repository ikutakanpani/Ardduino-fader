#ifndef PIDControl_h
#define PIDControl_h
#include "Arduino.h"
#include "moving_Average.hpp"

class PIDControl: Average {
public:
  PIDControl(){
    set_total(20);//移動平均20回分
  }

  void set_kp(float Kp);
  void set_ki(float Ki);
  void set_kd(float Kd);

  void set_input(uint16_t input);
  void set_free(boolean set);
  void set_manual_move_lock(boolean lock);
  void set_target(uint16_t Target);

  uint16_t get_input();
  uint16_t get_target();
  boolean get_lockStatus();
  boolean get_manual_move_lock();

  long pidCalculation();

private:
  float _Kp = 1;                 // 比例ゲインKp
  float _Ki = 0;                 // 比例ゲインKi
  float _Kd = 0;                 // 比例ゲインKd

  uint16_t _target_lock_ref = 10;     //PIDロック判定誤差
  uint16_t _lockval = 30;             //PIDロック完了カウント判定値
  uint16_t _total_num_limit = 20;     //ヒステリシス
  uint16_t _lock_delay = 200;         //free⇒autoの復帰遅延 ms

  uint16_t _Target = 1024 / 2;     //スライダー中心値
  uint16_t _input = 0;
  double _integral = 0;
  double _last_err = 0;
  uint32_t _total_num = 0;

  boolean _free;
  boolean _free_set = false;
  uint32_t _old_free_time = 0;

  boolean _manual_move_lock = false;
  boolean _old_lock = false;
  boolean _lock_out = false;

  double absolute_val(double a);
  boolean free_check();
};

#endif
