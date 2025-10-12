#ifndef WEATHER_FORECAST_H
#define WEATHER_FORECAST_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 前方宣言
class TimeManager;

// 天気予報データ構造体
struct WeatherData {
  bool isValid;              // データの有効性
  float tempMax;             // 最高気温 (°C)
  float tempMin;             // 最低気温 (°C)
  int weatherCode;           // 天気コード
  String weatherString;      // 天気の文字列表現
  unsigned long lastUpdate;  // 最終更新時刻 (millis)
};

// 天気予報管理クラス
class WeatherForecast {
public:
  // コンストラクタ
  WeatherForecast(float latitude, float longitude);

  // 初期化（起動時の天気予報取得）
  bool begin();

  // 定期更新チェック（毎時0分に更新）
  void update(TimeManager& timeMgr);

  // 最新の天気予報データを取得
  WeatherData getData() const;

private:
  // API設定
  String apiUrl_;

  // 更新管理
  int lastUpdateHour_;  // 最後に更新した時（0-23）

  // 天気データ
  WeatherData weatherData_;

  // 内部処理関数
  bool fetchWeatherData();
  String weatherCodeToString(int code) const;
};

#endif // WEATHER_FORECAST_H
