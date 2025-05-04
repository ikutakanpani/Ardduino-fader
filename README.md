# モーターフェーダーユニットの仕様
- 電源電圧：6.5～7.5V
- 信号電圧：3.3V
- SPI推奨周波数：400kHz
## ハードについて
- 基板
  - 開発ソフト：PCBE
  - JLCPCBで発注 ガーバーデータ配布あり
  ```
  ベース素材： FR-4
  層： 2
  寸法： 18 mm* 98.3 mm
  PCB 数量： 10
  製品タイプ 産業用/民生用電子機器
  異なるデザイン： 1
  配信形式： 面付けなし
  PCB の厚さ： 1.6mm
  スタックアップの指定: no
  PCBの色： Black
  シルク印刷： White
  材料の種類： FR4-Standard TG 135-140
  ビア処理: レジストカバー
  表面処理： HASL(with lead)
  バリ取り/エッジ丸め： No
  外面被覆銅重量: 1 oz
  金端子: いいえ
  電気テスト: フライング・プローブ・フルテスト
  鋳抜き穴 no
  エッジメッキ: No
  PCB上のマーク: マーク除去
  4-Wire Kelvin Test: No
  Paper between PCBs: No
  Appearance Quality: IPC Class 2 Standard
  Confirm Production file: No
  Silkscreen Technology: Ink-jet/Screen Printing Silkscreen
  Package Box: With JLCPCB logo
  Board Outline Tolerance: ±0.2mm(Regular)
  ```
## パーツリスト
| PCBシルク  | 概要                                          | 部品  | 数量 | 型番               | 販社     | 販社品番 |
| ---------- | --------------------------------------------- | ----- | ---- | ------------------ | -------- | -------- |
| C1-4       | MLCC 0.1μF50V B 2012                          | -     | 4    | GRM40-034B104K50   | 秋月電子 | 111384   |
| C5         | MLCC 4.7μF10V F 2012                          | -     | 1    | GRM21BF11A475ZA01  | 秋月電子 | 106135   |
| C6-7       | MLCC 10μF25V X7R 3216                         | -     | 2    | GRM31CR71E106KA12  | 秋月電子 | 114741   |
| C8-9       | OS-CON 100μF16V105℃                           | -     | 2    | 16SEPC100M         | 秋月電子 | 108290   |
| R1         | チップ抵抗 1/10W10kΩ                          | 1608m | 1    | RC0603J10K         | 秋月電子 | 115029   |
| R1(代替)   | カーボン抵抗(炭素皮膜抵抗) 1/6W10kΩ           | -     | 1    | RD16S 10K          | 秋月電子 | 116103   |
| R2         | チップ抵抗 1/10W1MΩ                           | 1608m | 1    |                    |          |          |
| R2(代替)   | カーボン抵抗(炭素皮膜抵抗) 1/6W1MΩ            | -     | 1    | CF16J1MB           | 秋月電子 | 116105   |
| L          | チップインダクター 10μH1.1A                   | -     | 1    | DFE322512F-100M    | 秋月電子 | 114977   |
| Xt         | チップセラロック コンデンサー内蔵タイプ 16MHz | -     | 1    | CSTNE16M0V530000R0 | 秋月電子 | 114562   |
| U1         | AVRマイコン ATMEGA328PB-AU                    | -     | 1    | ATMEGA328PB-AU     | 秋月電子 | 118148   |
| U2         | モータードライバー AM1096                     | -     | 1    | AM1096             | 秋月電子 | 118299   |
| U3         | 三端子レギュレーター 5V1.5A HTC製 LM7805RS    | -     | 1    | LM7805RS           | 秋月電子 | 118082   |
| motorFader | モータ駆動マスタタイプ（モータNフェーダ）     | -     | 1    | RSA0N11M9A0J       |          |          |
| -          | ピンヘッダー 1.27mm 2×5(10P)                  | -     | 1    | PH11-2x5SAG        | 秋月電子 | 113800   |

※モーターフェーダーについて2025/5/4時点で取扱ショップ見つけられず、以前はサウンドハウスでヤマハミキサーの予備パーツ(VAU01100 モーターフェーダー)として扱われていた。なおAliExpressで売っている同等品はおすすめしない(質が悪く本家と比べフェーダーのなめらかさがいまいち)
## ピン配置
| PIN | 用途                       |
| --- | -------------------------- |
| 1   | リセット(10kΩプルアップ有) |
| 2   | 通信GND                    |
| 3   | SPI_SCK                    |
| 4   | SPI_MOSI                   |
| 5   | Vin                        |
| 6   | Vin                        |
| 7   | SPI_MISO                   |
| 8   | SPI_SS                     |
| 9   | GND                        |
| 10  | GND                        |

※ピン配置についてはフラットケーブルを逆さまに接続しても故障しないように設定しました

# モーターフェーダーユニットの開発
## Arduino 環境の構築
- IDEバージョン
  - 2.x.x
https://www.arduino.cc/en/software/
- ボードマネージャー
  - MiniCore v3.1.1
  https://github.com/MCUdude/MiniCore
    - Boards Manager URLs : 
      https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json

### Arduino ファイルファイルの追加
- my_Programmers をダウンロード
- フォルダ構成
  - C:\Users\{ユーザ名}\AppData\Local\Arduino15\packages\MiniCore\hardware\avr\3.1.1
    - \my_Programmers\ ←ダウンロードしたフォルダをコピー
      - avrdude2.conf
      - avrdube.exe

### Arduino ファイルの書き換え
- フォルダパス
  - C:\Users\{ユーザ名}\AppData\Local\Arduino15\packages\MiniCore\hardware\avr\3.1.1\
- platform.txt に追記
```
# MY AVR Uploader/Programmers tools
# ------------------------------

tools.my_avrdude.path={runtime.tools.avrdude.path}
tools.my_avrdude.cmd.path={runtime.platform.path}/my_Programmers/avrdude
tools.my_avrdude.config.path={runtime.platform.path}/my_Programmers/avrdude.conf

tools.my_avrdude.upload.params.verbose=-v
tools.my_avrdude.upload.params.quiet=-q -q
# tools.my_avrdude.upload.verify is needed for backwards compatibility with my_avrdude 6.3.0 and IDE 1.6.8 or older, IDE 1.6.9 or newer overrides this value
tools.my_avrdude.upload.verify=
tools.my_avrdude.upload.params.noverify=-V
tools.my_avrdude.upload.pattern="{cmd.path}" "-C{config.path}" {upload.verbose} {upload.verify} -p{build.mcu} -c{protocol} {program.extra_params} "-Ueeprom:w:{build.path}/{build.project_name}.eep:i" "-Uflash:w:{build.path}/{build.project_name}.hex:i"

tools.my_avrdude.program.params.verbose=-v
tools.my_avrdude.program.params.quiet=-q -q
# tools.my_avrdude.program.verify is needed for backwards compatibility with my_avrdude 6.3.0 and IDE 1.6.8 or older, IDE 1.6.9 or newer overrides this value
tools.my_avrdude.program.verify=
tools.my_avrdude.program.params.noverify=-V
tools.my_avrdude.program.pattern="{cmd.path}" "-C{config.path}" {program.verbose} {program.verify} -p{build.mcu} -c{protocol} {program.extra_params} "-Ueeprom:w:{build.path}/{build.project_name}.eep:i" "-Uflash:w:{build.path}/{build.project_name}.hex:i"

tools.my_avrdude.erase.params.verbose=-v
tools.my_avrdude.erase.params.quiet=-q -q
tools.my_avrdude.erase.pattern="{cmd.path}" "-C{config.path}" {bootloader.verbose} -p{build.mcu} -c{protocol} {program.extra_params} -e -Ulock:w:{bootloader.unlock_bits}:m -Uefuse:w:{bootloader.extended_fuses}:m -Uhfuse:w:{bootloader.high_fuses}:m -Ulfuse:w:{bootloader.low_fuses}:m

tools.my_avrdude.bootloader.params.verbose=-v
tools.my_avrdude.bootloader.params.quiet=-q -q
tools.my_avrdude.bootloader.pattern="{cmd.path}" "-C{config.path}" {bootloader.verbose} -p{build.mcu} -c{protocol} {program.extra_params} "-Uflash:w:{runtime.platform.path}/bootloaders/{bootloader.file}:i" -Ulock:w:{bootloader.lock_bits}:m

tools.my_avrdude_remote.upload.pattern=/usr/bin/run-my_avrdude /tmp/sketch.hex {upload.verbose} -p{build.mcu}
```
- programmers.txt に追記
```
net_isp.name=My network ISP
net_isp.protocol=arduino
net_isp.program.protocol=arduino
net_isp.program.extra_params= -P net:esp-avrisp-test.local:328
net_isp.program.tool=my_avrdude
```
## ヒューズビットの設定
- コマンドプロンプトで下記フォルダに移動
  - C:\Users\{ユーザ名}\AppData\Local\Arduino15\packages\MiniCore\hardware\avr\3.1.1\my_Programmers\
- 書きコマンド実行で書き込み
~~~
avrdude.exe -Cavrdude.conf -v -carduino -patmega328pb -P net:esp-avrisp-test.local:328 -e -U lock:w:0x3F:m -U efuse:w:0xf5:m -U hfuse:w:0xde:m -U lfuse:w:0xc7:m
~~~

## 基板へ書き込み
プログラムの書き込みには一般的なICSPライタかモーターフェーダーコントロール基板から行う、この手順はモーターフェーダーコントロール基板を想定した物になる
- ヒューズビットの設定
- プログラムの書き込み
  - arduino-fader_r2_v2 をダウンロード
  - ArduinoIDEからarduino-fader_r2_v2.ino を開く
  - ツールからボードの選択
  - 書き込み装置をMy network ISP に設定
 
