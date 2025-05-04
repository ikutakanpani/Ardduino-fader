/*Arduino IDE ボード設定----------
ボード : ATmega328 (MiniCore)
EEPROM : EEPROM retained
Variant : 328PB
Bootloader : No bootloader
BOO : BOD 2.7V
Clock : External 16 MHz
Baud rate : 初期値
Compiler LTO : LTO enabled
-------------------------------
  ボード
  ・MiniCore
  　https://github.com/MCUdude/MiniCore?tab=readme-ov-file
  ヒューズビット	>> -e -U lock:w:0x3F:m -U efuse:w:0xf5:m -U hfuse:w:0xde:m -U lfuse:w:0xc7:m
*/

#include "CapacitiveSensor.h"
#include "TimerThree.h"
#include "PIDControl.h"

PIDControl PID;
CapacitiveSensor cs = CapacitiveSensor(PIN_PD3, PIN_PD4);

uint8_t spi_buf[4];
uint8_t spi_send_buf[3];
uint8_t readReg;
boolean readonly;

volatile uint16_t fader_val;
volatile boolean centerMode_status;
volatile boolean centerResult_status;

volatile uint16_t send_fader_val;
uint8_t fader_val_cnt;

volatile uint16_t pid_cycle = 8;//タイマー割り込み(n*100uS)サイクル(1cycleの処理時間230～450uS)
volatile uint16_t target_val;
volatile boolean lock_set;
volatile boolean pid_status;
uint16_t old_target_val;
boolean old_pid_status;

void setup() {
  Serial.begin(115200);
  TCCR1B &= B11111000;
  TCCR1B |= B00000001;
  TCCR4B &= B11111000;
  TCCR4B |= B00000001;

  //pinを出力に設定
  pinMode(PIN_PD2 , OUTPUT);
  pinMode(PIN_PB1 , OUTPUT);
  SPI_ini();

  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);

  //PIDパラメータの設定
  PID.set_kp(11);
  PID.set_ki(0.55);
  PID.set_kd(0.3);

  PID.center_warp_call([]() {
    centerResult_status = true;
  });

  delay(100);
}

void SPI_ini() {
  // SPI slave
  DDRB |= 0x10;  // MISO(PB4)をoutputに設定
  // SPCRのビット設定
  // SPI有効:1 | SPI割り込み有効:1 | MSBから送信:0 | スレーブ:0
  // クロック極性:0 | クロック位相:0 | SPR1:0 | SPR0:0
  SPCR = (1 << SPIE) | (1 << SPE);// | (1 << DORD);
}

ISR (SPI_STC_vect) {
  boolean ackBit = false;
  float pid_parameter_val;
  uint16_t send_cash;
  uint8_t receive = SPDR;// 受信データ
  uint8_t receive_flag = receive & 0b11000000;
  noInterrupts();

  if ((receive_flag == 0) && ((receive & 0b00111100) == 0b00111100)) { //-1
    memset(spi_buf, 0, 4);
    spi_buf[3] = receive & 0b00000001;
    SPDR = receive;
    //Serial.println();
    //Serial.println("Parity : " + String(receive, BIN));

  } else if (receive_flag == 0b11000000) { //2
    if (receive != 0b11000000)readonly = false;
    spi_buf[2] = receive;

    if (parity(spi_buf) == spi_buf[3])ackBit = true;

    if (!readonly && ackBit) {
      ackBit = false;
      switch (readReg) { //reg(4bit)
        case 0://← status、fader_val
          target_val = (uint16_t(0b00111111 & spi_buf[2]) << 4) | (0b00001111 & spi_buf[1]);
          ackBit = true;
        case 1://← status
          lock_set = spi_buf[0] & 1;
          centerMode_status = (spi_buf[0] >> 1) & 1;
          pid_status = (spi_buf[1] >> 4) & 1;
          //dummy = (spi_buf[1] >> 5) & 1;
          ackBit = true;
          break;

        case 12://kp
          pid_parameter_val = (uint16_t(0b00111111 & spi_buf[2]) << 6) | (0b00111111 & spi_buf[1]);
          if (pid_parameter_val > 4095)break;
          //Serial.println("PID P CHANGE : " + String(pid_parameter_val / 100));
          PID.set_kp(pid_parameter_val / 100);
          ackBit = true;
          break;

        case 13://ki
          pid_parameter_val = (uint16_t(0b00111111 & spi_buf[2]) << 6) | (0b00111111 & spi_buf[1]);
          if (pid_parameter_val > 4095)break;
          //Serial.println("PID I CHANGE : " + String(pid_parameter_val / 100));
          PID.set_ki(pid_parameter_val / 100);
          ackBit = true;
          break;

        case 14://kd
          pid_parameter_val = (uint16_t(0b00111111 & spi_buf[2]) << 6) | (0b00111111 & spi_buf[1]);
          if (pid_parameter_val > 4095)break;
          //Serial.println("PID D CHANGE : " + String(pid_parameter_val / 100));
          PID.set_kd(pid_parameter_val / 100);
          ackBit = true;
          break;

        case 15://pid cycle
          pid_parameter_val = (uint16_t(0b00111111 & spi_buf[2]) << 6) | (0b00111111 & spi_buf[1]);
          if (pid_parameter_val > 20)break;
          //Serial.println("PID CYCLE CHANGE : " + String(pid_parameter_val));
          pid_cycle = pid_parameter_val;
          ackBit = true;
          break;
      }
    }

    spi_send_buf[2] |= (ackBit ? 0b10 : 0);
    spi_send_buf[2] |= parity(spi_send_buf);
    SPDR = spi_send_buf[2];

    readReg = 0;

  } else if (receive_flag == 0b10000000) { //1
    if (receive != 0b10000000)readonly = false;
    spi_buf[1] = receive;
    SPDR = spi_send_buf[1];
    //Serial.println("spi_send_buf[1] : " + String(spi_send_buf[1], BIN));

  } else if ((receive_flag == 0b01000000) || ((receive_flag == 0) && (receive & 0b00000011))) { //0
    spi_send_buf[0] = 0b01000000;
    spi_send_buf[1] = 0b10000000;
    spi_send_buf[2] = 0b11000000;
    spi_buf[0] = receive;
    readReg = (spi_buf[0] >> 2) & 0b00001111;
    readonly = (receive_flag == 0) ? true : false;

    switch (readReg) {
      case 0://← status、fader_val
        spi_send_buf[0] |= 0b00111111 & fader_val;//下位6/10bit
        spi_send_buf[2] |= 0b00111100 & (fader_val >> 4);//上位4/10bit
        send_fader_val = fader_val;
      case 1://← status
        spi_send_buf[1] |= faderStatus();
        break;

      case 12://← kp
        send_cash = float(PID.get_kp() * 100);
      case 13://← ki
        send_cash = float(PID.get_ki() * 100);
      case 14://← kd
        if (readReg == 14)send_cash = float(PID.get_kd() * 100);
        spi_send_buf[0] |= 0b00111111 & send_cash;//下位6/12bit
        spi_send_buf[1] |= 0b00111111 & (send_cash >> 6);//上位6/12bit
        break;
      case 15://← pid cycle
        spi_send_buf[0] |= 0b00111111 & pid_cycle;
        break;
    }

    SPDR = spi_send_buf[0];
    //Serial.println("spi_send_buf[0] : " + String(spi_send_buf[0], BIN));

  }
  interrupts();
}

uint8_t faderStatus() { //0b00xxxxxx
  uint8_t send_q;
  send_q = PID.get_free() ? 0b00000001 : 0;
  send_q |= PID.get_lockStatus() ? 0b00000010 : 0;
  send_q |= lock_set ? 0b00000100 : 0;
  send_q |= centerResult_status ? 0b0001000 : 0;
  send_q |= centerMode_status ? 0b00010000 : 0;
  send_q |= pid_status ? 0b00100000 : 0;

  if (centerResult_status)centerResult_status = false;

  return send_q;
}

//割り込み処理
void slide_control() {
  PID.set_input(analogRead(A0));

  long i = PID.pidCalculation();  // PIDでモータを制御
  if (abs(i) > 2) {
    if (i > 0) {
      PD2_analogWrite(i);
      PB1_analogWrite(0);
    } else {
      PD2_analogWrite(0);
      PB1_analogWrite(abs(i));
    }
  } else {
    PD2_analogWrite(255);
    PB1_analogWrite(255);
  }
}

void loop() {
  if (old_pid_status != pid_status) {
    old_pid_status = pid_status;

    Serial.println("PID STATUS CHANGE : " + String(pid_status ? "Enabled" : "Disabled"));

    if (pid_status) {
      Timer3.initialize(pid_cycle * 100); //The led will blink in a half second time interval
      Timer3.attachInterrupt(slide_control);
    } else {
      Timer3.stop();
      PD2_analogWrite(0);
      PB1_analogWrite(0);
    }
  }

  if (old_target_val != target_val) {
    old_target_val = target_val;

    Serial.println(target_val);
    PID.set_target(target_val);
  }

  if (cs.capacitiveSensor(50) > 500) {
    PID.set_free(true);

  } else {
    if (send_fader_val == fader_val) {
      if (fader_val_cnt > 40) {
        PID.set_free(false);

      } else {
        fader_val_cnt++;

      }

    } else {
      fader_val_cnt = 0;

    }

  }

  fader_val = PID.get_input();
  PID.set_manual_move_lock(lock_set);
  PID.set_center_mode(centerMode_status);

  delay(1);
}

uint8_t parity(uint8_t *buf) {
  uint32_t val = buf[0];
  val = val << 8;
  val |= buf[1];
  val = val << 8;
  val |= buf[2];

  val ^= val >> 16;
  val ^= val >> 8;
  val ^= val >> 4;
  val ^= val >> 2;
  val ^= val >> 1;

  return val & 0x00000001;
}

void PB1_analogWrite(uint8_t val){
  switch(val){
    case 0:
      TCCR1A &= ~_BV(COM1A1);
      PORTB &= 0b11111101;
      break;
    case 255:
      TCCR1A &= ~_BV(COM1A1);
      PORTB |= 0b00000010;
      break;
    default:
      TCCR1A |= _BV(COM1A1);
      OCR1A = val; // set pwm duty
  }
}

void PD2_analogWrite(uint8_t val){
  switch(val){
    case 0:
      TCCR4A &= ~_BV(COM4B1);
      PORTD &= 0b11111011;
      break;
    case 255:
      TCCR4A &= ~_BV(COM4B1);
      PORTD |= 0b00000100;
      break;
    default:
      PORTD |= _BV(PD2);
      TCCR4A |= _BV(COM4B1);
      OCR4B = val; // set pwm duty
  }
}