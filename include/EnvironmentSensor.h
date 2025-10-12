#ifndef ENVIRONMENT_SENSOR_H
#define ENVIRONMENT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

// センサーデータ構造体
struct SensorData {
  float temperature;
  float humidity;
  float discomfortIndex;  // 不快指数（DI）
  bool isValid;

  SensorData() : temperature(0.0f), humidity(0.0f), discomfortIndex(0.0f), isValid(false) {}
  SensorData(float temp, float hum, bool valid)
    : temperature(temp), humidity(hum), discomfortIndex(0.0f), isValid(valid) {}
  SensorData(float temp, float hum, float di, bool valid)
    : temperature(temp), humidity(hum), discomfortIndex(di), isValid(valid) {}
};

// 環境センサークラス
class EnvironmentSensor {
public:
  EnvironmentSensor(uint8_t pin, uint8_t type, float tempOffset = 0.0f, float humOffset = 0.0f);

  // 初期化
  void begin();

  // センサーデータを読み取る
  SensorData read();

  // オフセットを設定
  void setTemperatureOffset(float offset) { temperatureOffset_ = offset; }
  void setHumidityOffset(float offset) { humidityOffset_ = offset; }

  // 不快指数（DI）を計算
  static float calculateDiscomfortIndex(float temperature, float humidity);

private:
  DHT dht_;
  float temperatureOffset_;
  float humidityOffset_;
};

#endif // ENVIRONMENT_SENSOR_H
