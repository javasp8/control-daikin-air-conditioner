/**
 * AirConditionerController.cpp
 *
 * ダイキンエアコンを赤外線で制御するクラスの実装ファイル
 *
 * 主な機能:
 * - 季節・時間帯・温湿度に基づいた最適なエアコン制御
 * - 夜間停止機能（春・秋・冬季の23:00〜7:00）
 * - 極寒日の特別対応（最低気温0度以下の場合は夜間も暖房18度で運転）
 * - 快適温度帯（24.5〜26.5度）と湿度帯（40〜60%）の維持
 */

#include "AirConditionerController.h"
#include <IRutils.h>

// 温度・湿度の閾値設定
namespace Threshold {
  // 温度範囲
  constexpr float TEMP_LOWER = 24.2f;   // 目標室温下限
  constexpr float TEMP_UPPER = 26.5f;   // 目標室温上限

  // ヒステリシス（不感帯）設定
  constexpr float TEMP_HYSTERESIS = 0.3f;   // 温度ヒステリシス幅（℃）
  constexpr float TEMP_LOWER_OFF = TEMP_LOWER + TEMP_HYSTERESIS;  // 暖房停止温度: 24.5℃
  constexpr float TEMP_UPPER_OFF = TEMP_UPPER - TEMP_HYSTERESIS;  // 冷房停止温度: 26.2℃

  // 湿度範囲
  constexpr float HUMIDITY_LOWER = 40.0f;   // 目標湿度下限
  constexpr float HUMIDITY_UPPER = 62.0f;   // 目標湿度上限
  constexpr float HUMIDITY_HIGH = 65.0f;    // 高湿度の閾値
  constexpr float HUMIDITY_VERY_HIGH = 70.0f; // 非常に高湿度の閾値
}

/**
 * コンストラクタ
 */
AirConditionerController::AirConditionerController(uint8_t sendPin, uint8_t recvPin)
  : daikinAC_(sendPin), irRecv_(recvPin), currentMode_(ACMode::NONE) {
}

/**
 * 初期化処理
 */
void AirConditionerController::begin() {
  daikinAC_.begin();
  irRecv_.enableIRIn();
  Serial.println("[AC] エアコンコントローラー初期化完了");
}

/**
 * エアコンの動作モードを設定
 */
void AirConditionerController::setMode(ACMode mode) {
  // 既に同じモードの場合はスキップ
  if (mode == currentMode_) {
    Serial.println("[AC] モード変更なし（すでに同じモード）");
    return;
  }

  // 停止命令の場合、既に停止状態ならスキップ（受信音防止）
  if (mode == ACMode::OFF && currentMode_ == ACMode::OFF) {
    Serial.println("[AC] すでに停止状態のため、停止信号を送信しません");
    return;
  }

  switch (mode) {
    case ACMode::OFF:
      sendOff();
      break;
    case ACMode::HEATING_23_5:
      sendHeating23_5();
      break;
    case ACMode::HEATING_18:
      sendHeating18();
      break;
    case ACMode::COOLING_25:
      sendCooling25();
      break;
    case ACMode::DEHUMID_MINUS_1_5:
      sendDehumidMinus1_5();
      break;
    default:
      Serial.println("[AC] 無効なモード");
      return;
  }

  currentMode_ = mode;
}

/**
 * 現在の季節を判定
 */
Season AirConditionerController::getCurrentSeason(int month) const {
  if (month >= 3 && month <= 5) {
    return Season::SPRING;
  } else if (month >= 6 && month <= 9) {
    return Season::SUMMER;
  } else if (month >= 10 && month <= 11) {
    return Season::AUTUMN;
  } else {
    return Season::WINTER;
  }
}

/**
 * 現在の時間帯を判定
 */
TimeOfDay AirConditionerController::getTimeOfDay(int hour) const {
  if (hour >= 7 && hour < 23) {
    return TimeOfDay::DAYTIME;
  } else {
    return TimeOfDay::NIGHT;
  }
}

/**
 * 極寒日かどうかを判定（最低気温0度以下）
 */
bool AirConditionerController::isExtremeColdDay(const WeatherData& weather) const {
  if (!weather.isValid) {
    return false;
  }
  return weather.tempMin <= 0.0f;
}

/**
 * 温度・湿度・時刻・天気予報に基づいて最適なモードを決定
 */
ACMode AirConditionerController::determineOptimalMode(float temperature, float humidity,
                                                      TimeManager& timeMgr, const WeatherData& weather) {
  // 現在の時刻を取得
  struct tm timeinfo;
  if (!timeMgr.getCurrentTime(timeinfo)) {
    Serial.println("[AC] 時刻取得失敗、デフォルトモード");
    return ACMode::OFF;
  }

  int month = timeinfo.tm_mon + 1;  // tm_monは0-11なので+1
  int hour = timeinfo.tm_hour;

  Season season = getCurrentSeason(month);
  TimeOfDay timeOfDay = getTimeOfDay(hour);
  bool isExtremeCold = isExtremeColdDay(weather);

  Serial.printf("[AC] 温度:%.1f℃, 湿度:%.1f%%, 月:%d, 時:%d\n", temperature, humidity, month, hour);

  // 季節別に制御ロジックを実行
  ACMode mode;
  switch (season) {
    case Season::SPRING:
      Serial.println("[AC] 季節: 春季");
      mode = determineSpringMode(temperature, humidity, timeOfDay);
      break;
    case Season::SUMMER:
      Serial.println("[AC] 季節: 夏季");
      mode = determineSummerMode(temperature, humidity);
      break;
    case Season::AUTUMN:
      Serial.println("[AC] 季節: 秋季");
      mode = determineAutumnMode(temperature, humidity, timeOfDay);
      break;
    case Season::WINTER:
      Serial.println("[AC] 季節: 冬季");
      mode = determineWinterMode(temperature, humidity, timeOfDay, isExtremeCold);
      break;
    default:
      mode = ACMode::OFF;
  }

  return mode;
}

/**
 * 春季（3〜5月）の制御ロジック
 */
ACMode AirConditionerController::determineSpringMode(float temperature, float humidity, TimeOfDay timeOfDay) {
  // 夜間は停止
  if (timeOfDay == TimeOfDay::NIGHT) {
    Serial.println("[AC] 春季・夜間 → 停止");
    return ACMode::OFF;
  }

  // 日中の制御（ヒステリシス付き）
  if (temperature < Threshold::TEMP_LOWER) {
    // 24.2度未満 → 暖房23.5度
    Serial.printf("[AC] 春季・日中: 室温%.1f℃ < %.1f℃ → 暖房23.5度\n", temperature, Threshold::TEMP_LOWER);
    return ACMode::HEATING_23_5;
  } else if (currentMode_ == ACMode::HEATING_23_5 && temperature < Threshold::TEMP_LOWER_OFF) {
    // 暖房中で24.5度未満 → 暖房継続（ヒステリシス）
    Serial.printf("[AC] 春季・日中: 暖房中（室温%.1f℃ < %.1f℃）→ 暖房継続\n", temperature, Threshold::TEMP_LOWER_OFF);
    return ACMode::HEATING_23_5;
  } else if (temperature > Threshold::TEMP_UPPER) {
    // 26.5度超 → 冷房25度
    Serial.printf("[AC] 春季・日中: 室温%.1f℃ > %.1f℃ → 冷房25度\n", temperature, Threshold::TEMP_UPPER);
    return ACMode::COOLING_25;
  } else if (currentMode_ == ACMode::COOLING_25 && temperature > Threshold::TEMP_UPPER_OFF) {
    // 冷房中で26.2度超 → 冷房継続（ヒステリシス）
    Serial.printf("[AC] 春季・日中: 冷房中（室温%.1f℃ > %.1f℃）→ 冷房継続\n", temperature, Threshold::TEMP_UPPER_OFF);
    return ACMode::COOLING_25;
  } else {
    // 快適範囲内 → 停止（春季は除湿を行わない）
    Serial.printf("[AC] 春季・日中: 快適範囲内（温度%.1f℃, 湿度%.1f%%）→ 停止\n", temperature, humidity);
    return ACMode::OFF;
  }
}

/**
 * 夏季（6〜9月）の制御ロジック（24時間運転）
 */
ACMode AirConditionerController::determineSummerMode(float temperature, float humidity) {
  // 過冷房防止（24.2度未満で停止）
  if (temperature < Threshold::TEMP_LOWER) {
    Serial.printf("[AC] 夏季: 室温%.1f℃ < %.1f℃ → 過冷房防止のため停止\n", temperature, Threshold::TEMP_LOWER);
    return ACMode::OFF;
  }

  // 冷房運転中のヒステリシス判定
  if (currentMode_ == ACMode::COOLING_25 && temperature > Threshold::TEMP_UPPER_OFF) {
    // 冷房中で26.2度超 → 冷房継続（ヒステリシス）
    Serial.printf("[AC] 夏季: 冷房中（室温%.1f℃ > %.1f℃）→ 冷房継続\n", temperature, Threshold::TEMP_UPPER_OFF);
    return ACMode::COOLING_25;
  }

  // 除湿運転中のヒステリシス判定
  if (currentMode_ == ACMode::DEHUMID_MINUS_1_5) {
    if (temperature > Threshold::TEMP_UPPER_OFF && humidity > Threshold::HUMIDITY_UPPER) {
      // 除湿中で条件継続 → 除湿継続（ヒステリシス）
      Serial.printf("[AC] 夏季: 除湿中（室温%.1f℃ > %.1f℃, 湿度%.1f%% > %.1f%%）→ 除湿継続\n",
                    temperature, Threshold::TEMP_UPPER_OFF, humidity, Threshold::HUMIDITY_UPPER);
      return ACMode::DEHUMID_MINUS_1_5;
    }
  }

  // 新規起動判定
  if (temperature > Threshold::TEMP_UPPER) {
    // 26.5度超 → 冷房25度
    Serial.printf("[AC] 夏季: 室温%.1f℃ > %.1f℃ → 冷房25度\n", temperature, Threshold::TEMP_UPPER);
    return ACMode::COOLING_25;
  } else if (temperature >= Threshold::TEMP_LOWER && temperature <= Threshold::TEMP_UPPER) {
    // 快適温度範囲内
    if (humidity > Threshold::HUMIDITY_UPPER) {
      // 湿度62%超 → 除湿
      Serial.printf("[AC] 夏季: 湿度%.1f%% > %.1f%% → 除湿-1.5度\n", humidity, Threshold::HUMIDITY_UPPER);
      return ACMode::DEHUMID_MINUS_1_5;
    } else {
      // 湿度も快適範囲内 → 停止
      Serial.printf("[AC] 夏季: 快適範囲内（温度%.1f℃, 湿度%.1f%%）→ 停止\n", temperature, humidity);
      return ACMode::OFF;
    }
  }

  return ACMode::OFF;
}

/**
 * 秋季（10〜11月）の制御ロジック
 */
ACMode AirConditionerController::determineAutumnMode(float temperature, float humidity, TimeOfDay timeOfDay) {
  // 夜間は停止
  if (timeOfDay == TimeOfDay::NIGHT) {
    Serial.println("[AC] 秋季・夜間 → 停止");
    return ACMode::OFF;
  }

  // 日中の制御（春季と同じロジック、ヒステリシス付き）
  if (temperature < Threshold::TEMP_LOWER) {
    // 24.2度未満 → 暖房23.5度
    Serial.printf("[AC] 秋季・日中: 室温%.1f℃ < %.1f℃ → 暖房23.5度\n", temperature, Threshold::TEMP_LOWER);
    return ACMode::HEATING_23_5;
  } else if (currentMode_ == ACMode::HEATING_23_5 && temperature < Threshold::TEMP_LOWER_OFF) {
    // 暖房中で24.5度未満 → 暖房継続（ヒステリシス）
    Serial.printf("[AC] 秋季・日中: 暖房中（室温%.1f℃ < %.1f℃）→ 暖房継続\n", temperature, Threshold::TEMP_LOWER_OFF);
    return ACMode::HEATING_23_5;
  } else if (temperature > Threshold::TEMP_UPPER) {
    // 26.5度超 → 冷房25度
    Serial.printf("[AC] 秋季・日中: 室温%.1f℃ > %.1f℃ → 冷房25度\n", temperature, Threshold::TEMP_UPPER);
    return ACMode::COOLING_25;
  } else if (currentMode_ == ACMode::COOLING_25 && temperature > Threshold::TEMP_UPPER_OFF) {
    // 冷房中で26.2度超 → 冷房継続（ヒステリシス）
    Serial.printf("[AC] 秋季・日中: 冷房中（室温%.1f℃ > %.1f℃）→ 冷房継続\n", temperature, Threshold::TEMP_UPPER_OFF);
    return ACMode::COOLING_25;
  } else {
    // 快適範囲内 → 停止（秋季は除湿を行わない）
    Serial.printf("[AC] 秋季・日中: 快適範囲内（温度%.1f℃, 湿度%.1f%%）→ 停止\n", temperature, humidity);
    return ACMode::OFF;
  }
}

/**
 * 冬季（12〜2月）の制御ロジック
 */
ACMode AirConditionerController::determineWinterMode(float temperature, float humidity,
                                                     TimeOfDay timeOfDay, bool isExtremeCold) {
  // 極寒日の夜間は暖房18度で継続運転
  if (timeOfDay == TimeOfDay::NIGHT && isExtremeCold) {
    Serial.println("[AC] 冬季・夜間・極寒日（最低気温0度以下）→ 暖房18度");
    return ACMode::HEATING_18;
  }

  // 通常の夜間は停止
  if (timeOfDay == TimeOfDay::NIGHT) {
    Serial.println("[AC] 冬季・夜間 → 停止（コスト削減優先）");
    return ACMode::OFF;
  }

  // 日中の制御（ヒステリシス付き）
  if (temperature < Threshold::TEMP_LOWER) {
    // 24.2度未満 → 暖房23.5度
    Serial.printf("[AC] 冬季・日中: 室温%.1f℃ < %.1f℃ → 暖房23.5度\n", temperature, Threshold::TEMP_LOWER);
    return ACMode::HEATING_23_5;
  } else if (currentMode_ == ACMode::HEATING_23_5 && temperature < Threshold::TEMP_LOWER_OFF) {
    // 暖房中で24.5度未満 → 暖房継続（ヒステリシス）
    Serial.printf("[AC] 冬季・日中: 暖房中（室温%.1f℃ < %.1f℃）→ 暖房継続\n", temperature, Threshold::TEMP_LOWER_OFF);
    return ACMode::HEATING_23_5;
  } else if (temperature >= Threshold::TEMP_LOWER_OFF && temperature <= Threshold::TEMP_UPPER) {
    // 快適範囲内 → 停止
    Serial.printf("[AC] 冬季・日中: 快適範囲内（温度%.1f℃）→ 停止\n", temperature);
    return ACMode::OFF;
  } else {
    // 26.5度超 → 自然冷却待ち（冷房・除湿は使用しない）
    Serial.printf("[AC] 冬季・日中: 室温%.1f℃ > %.1f℃ → 自然冷却待ち（停止）\n", temperature, Threshold::TEMP_UPPER);
    return ACMode::OFF;
  }
}

/**
 * 赤外線リモコン信号を受信して内容を表示（デバッグ用）
 */
void AirConditionerController::handleIRReceive() {
  decode_results results;

  if (irRecv_.decode(&results)) {
    Serial.println("====================================");
    Serial.print("[IR] 受信コード: ");
    serialPrintUint64(results.value, HEX);
    Serial.println();
    Serial.print("[IR] プロトコル: ");
    Serial.println(typeToString(results.decode_type));
    Serial.print("[IR] ビット数: ");
    Serial.println(results.bits);

    Serial.print("uint16_t rawData[");
    Serial.print(results.rawlen - 1);
    Serial.println("] = {");
    Serial.print("  ");
    for (uint16_t i = 1; i < results.rawlen; i++) {
      Serial.print(results.rawbuf[i] * kRawTick);
      if (i < results.rawlen - 1) Serial.print(", ");
      if (i % 10 == 0) Serial.print("\n  ");
    }
    Serial.println("\n};");
    Serial.println("====================================");

    irRecv_.resume();
  }
}

/**
 * エアコンを停止（電源オフ）
 */
void AirConditionerController::sendOff() {
  Serial.println("[AC] エアコン停止 送信開始");

  irRecv_.disableIRIn();
  daikinAC_.off();
  daikinAC_.send();

  Serial.println("[AC] エアコン停止 送信完了");

  delay(200);
  irRecv_.enableIRIn();
}

/**
 * 暖房23.5度の信号を送信
 */
void AirConditionerController::sendHeating23_5() {
  Serial.println("[AC] 暖房23.5度 送信開始");

  irRecv_.disableIRIn();

  daikinAC_.on();
  daikinAC_.setMode(kDaikinHeat);
  daikinAC_.setTemp(23.5);
  daikinAC_.setFan(kDaikinFanAuto);
  daikinAC_.setSwingVertical(false);
  daikinAC_.setSwingHorizontal(false);

  daikinAC_.send();

  Serial.println("[AC] 暖房23.5度 送信完了");

  delay(200);
  irRecv_.enableIRIn();
}

/**
 * 暖房18度の信号を送信（極寒日の夜間用）
 */
void AirConditionerController::sendHeating18() {
  Serial.println("[AC] 暖房18度 送信開始");

  irRecv_.disableIRIn();

  daikinAC_.on();
  daikinAC_.setMode(kDaikinHeat);
  daikinAC_.setTemp(18);
  daikinAC_.setFan(kDaikinFanAuto);
  daikinAC_.setSwingVertical(false);
  daikinAC_.setSwingHorizontal(false);

  daikinAC_.send();

  Serial.println("[AC] 暖房18度 送信完了");

  delay(200);
  irRecv_.enableIRIn();
}

/**
 * 冷房25度の信号を送信
 */
void AirConditionerController::sendCooling25() {
  Serial.println("[AC] 冷房25度 送信開始");

  irRecv_.disableIRIn();

  daikinAC_.on();
  daikinAC_.setMode(kDaikinCool);
  daikinAC_.setTemp(25);
  daikinAC_.setFan(kDaikinFanAuto);
  daikinAC_.setSwingVertical(false);
  daikinAC_.setSwingHorizontal(false);

  daikinAC_.send();

  Serial.println("[AC] 冷房25度 送信完了");

  delay(200);
  irRecv_.enableIRIn();
}

/**
 * 除湿-1.5度の信号を送信
 */
void AirConditionerController::sendDehumidMinus1_5() {
  Serial.println("[AC] 除湿-1.5度 送信開始");

  irRecv_.disableIRIn();

  daikinAC_.on();
  daikinAC_.setMode(kDaikinDry);
  daikinAC_.setTemp(24.5);  // 26度 - 1.5度 = 24.5度
  daikinAC_.setFan(kDaikinFanAuto);
  daikinAC_.setSwingVertical(false);
  daikinAC_.setSwingHorizontal(false);

  daikinAC_.send();

  Serial.println("[AC] 除湿-1.5度 送信完了");

  delay(200);
  irRecv_.enableIRIn();
}
