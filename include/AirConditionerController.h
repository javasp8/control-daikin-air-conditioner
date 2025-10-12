#ifndef AIR_CONDITIONER_CONTROLLER_H
#define AIR_CONDITIONER_CONTROLLER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <ir_Daikin.h>
#include "TimeManager.h"
#include "WeatherForecast.h"

// 季節の定義
enum class Season {
  SPRING,  // 春季（3〜5月）
  SUMMER,  // 夏季（6〜9月）
  AUTUMN,  // 秋季（10〜11月）
  WINTER   // 冬季（12〜2月）
};

// 時間帯の定義
enum class TimeOfDay {
  DAYTIME,  // 日中（7:00〜23:00）
  NIGHT     // 夜間（23:00〜翌7:00）
};

// エアコンの動作モード
enum class ACMode {
  NONE,
  OFF,               // エアコン停止（電源オフ）
  HEATING_23_5,      // 暖房23.5度
  HEATING_18,        // 暖房18度（極寒日の夜間用）
  COOLING_25,        // 冷房25度
  DEHUMID_MINUS_1_5  // 除湿-1.5度
};

// エアコン制御クラス
class AirConditionerController {
public:
  AirConditionerController(uint8_t sendPin, uint8_t recvPin);

  // 初期化
  void begin();

  // 指定されたモードでエアコンを制御
  void setMode(ACMode mode);

  // 現在のモードを取得
  ACMode getCurrentMode() const { return currentMode_; }

  // エアコンが停止状態かどうかを確認
  bool isOff() const { return currentMode_ == ACMode::OFF; }

  // 温度・湿度・時刻・天気予報に基づいて最適なモードを決定
  ACMode determineOptimalMode(float temperature, float humidity, TimeManager& timeMgr, const WeatherData& weather);

  // 赤外線信号の受信処理
  void handleIRReceive();

private:
  IRDaikinESP daikinAC_;
  IRrecv irRecv_;
  ACMode currentMode_;

  // ヘルパー関数
  Season getCurrentSeason(int month) const;
  TimeOfDay getTimeOfDay(int hour) const;
  bool isExtremeColdDay(const WeatherData& weather) const;

  // 季節別制御関数
  ACMode determineSpringMode(float temperature, float humidity, TimeOfDay timeOfDay);
  ACMode determineSummerMode(float temperature, float humidity);
  ACMode determineAutumnMode(float temperature, float humidity, TimeOfDay timeOfDay);
  ACMode determineWinterMode(float temperature, float humidity, TimeOfDay timeOfDay, bool isExtremeCold);

  // 各モードの送信関数
  void sendOff();                // エアコン停止（電源オフ）
  void sendHeating23_5();        // 暖房23.5度
  void sendHeating18();          // 暖房18度
  void sendCooling25();          // 冷房25度
  void sendDehumidMinus1_5();    // 除湿-1.5度
};

#endif // AIR_CONDITIONER_CONTROLLER_H
