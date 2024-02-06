#include "PIDControl.h"

//PIDパラメータ比例の設定
void PIDControl::set_kp(float Kp) {
  _Kp = Kp;
}

//PIDパラメータ積分の設定
void PIDControl::set_ki(float Ki) {
  _Ki = Ki;
}

//PIDパラメータ微分の設定
void PIDControl::set_kd(float Kd) {
  _Kd = Kd;
}

//フェーダーのanalogRead値を設定
//input = 0 ~ 1023
void PIDControl::set_input(uint16_t input) {
  PIDControl::input(input);
  _input = input;
}

//フェーダーをフリー状態にし、手動で変更した値にtargetを書き換える
//例)フェーダーのタッチセンサに触れているときはTrueを設定
//   離したらFalse
//set = true: フリー  false: 自動
void PIDControl::set_free(boolean set) {
  _free_set = set;
}

//フェーダーをTargetの位置で固定を強制する
//手動でフェーダーを動かしても戻ろうとする
//lock = true: 有効  false: 解除
void PIDControl::set_manual_move_lock(boolean lock) {
  _manual_move_lock = lock;
}

//フェーダーの移動を指示
// t = : 0 ~ 1023
void PIDControl::set_target(uint16_t Target) {
  if (Target > 1021) Target = 1021;
  if (Target < 2) Target = 2;

  if (_Target != Target) {
    _Target = Target;
    _total_num = 0;
    _old_free_time = 0;
  }
}

//フェーダーのロック状態を取得
// True : フェーダーの位置がTargetで指定した位置で制止している状態
// False: ↑の対義
boolean PIDControl::get_lockStatus() {
  return free;
}

//フェーダーの位置を返す　移動平均で異常値をならす
// 0 ~ 1023
uint16_t PIDControl::get_input() {
  uint16_t _input_raw = PIDControl::read_average();

  if (_input_raw >= 1021) _input_raw = 1023;
  if (_input_raw <= 2) _input_raw = 0;

  return _input_raw;
}

//手動フェーダーロックの状態を取得
boolean PIDControl::get_manual_move_lock() {
  return _manual_move_lock;
}

//現在のTargetを取得
uint16_t PIDControl::get_target() {
  return _Target;
}

//絶対値計算(adsでいいかも)
double PIDControl::absolute_val(double a) {
  return (a > 0) ? a : a * -1;
}

//フェーダーをフリーにするか判定
//フェーダーのタッチセンサを使用する際に重要
boolean PIDControl::free_check() {
  if (_old_lock != _free_set) {
    _old_lock = _free_set;
    if (!_free_set) _old_free_time = millis() + _lock_delay;
  }

  if (_free_set) _lock_out = true;
  else if (millis() > _old_free_time) _lock_out = false;

  if (_manual_move_lock) return _free_set;
  return _lock_out;
}

//PID計算メイン処理
//PIDの計算しモーターへの値を出力
//戻り値：-255 ~ 0 ~ 255
long PIDControl::pidCalculation() {
  if (PIDControl::absolute_val(_Target - _input) < _target_lock_ref) _total_num = (_total_num < (_lockval + _total_num_limit)) ? _total_num + 1 : _total_num;
  else _total_num = (_total_num > 0) ? _total_num - 1 : 0;

  boolean free_enable = PIDControl::free_check();
  if (_total_num > _lockval) _free = true;
  else if (!free_enable) _free = false;

  if (free_enable) {
    _free = false;
    if (!_manual_move_lock) {
      _free = true;
      _Target = _input;
    }
  }

  double err = _Target - _input;  // 偏差を計算

  double Kp = _Kp * err;          // 偏差の比例を計算

  double Kd = _Kd * (err - _last_err);  // 偏差の微分を計算

  _integral += (PIDControl::absolute_val(_integral + err) > 300) ? 0 : err;  // 偏差の積分を計算
  double Ki = _Ki * _integral;

  _last_err = err;  // 現在の偏差を保存

  long out_val = _free ? 0 : Kp + Ki + Kd;//合算

  if (_manual_move_lock && free_enable) {
    if (PIDControl::absolute_val(out_val) > 100) {
      out_val = (out_val > 0) ? 100 : -100;
    }
  }

  if (PIDControl::absolute_val(out_val) > 240) {
    out_val = (out_val > 0) ? 240 : -240;
  }

  return out_val;
}
