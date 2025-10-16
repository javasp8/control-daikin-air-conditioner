#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "EnvironmentSensor.h"
#include "WeatherForecast.h"
#include "AirConditionerController.h"

// ディスプレイコントローラークラス
class DisplayController {
public:
  DisplayController(uint8_t width, uint8_t height, TwoWire* wire, int8_t resetPin, uint8_t address);

  // 初期化
  bool begin();

  // スタートアップ画面を表示
  void showStartupScreen();

  // センサーデータを表示
  void showSensorData(const SensorData& data, const char* datetime);

  // センサーデータと天気予報を表示
  void showSensorDataWithWeather(const SensorData& data, const char* datetime, const WeatherData& weather);

  // センサーデータと天気予報とエアコン状態を表示
  void showSensorDataWithWeatherAndAC(const SensorData& data, const char* datetime, const WeatherData& weather, ACMode acMode);

  // エラー画面を表示
  void showError(const char* message);

private:
  Adafruit_SSD1306 display_;
  uint8_t width_;
  uint8_t height_;
};

#endif // DISPLAY_CONTROLLER_H
