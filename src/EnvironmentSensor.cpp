#include "EnvironmentSensor.h"

EnvironmentSensor::EnvironmentSensor(uint8_t pin, uint8_t type, float tempOffset, float humOffset)
  : dht_(pin, type), temperatureOffset_(tempOffset), humidityOffset_(humOffset) {
}

void EnvironmentSensor::begin() {
  dht_.begin();
  Serial.println("[Sensor] 環境センサー初期化完了");
}

SensorData EnvironmentSensor::read() {
  float humidity = dht_.readHumidity();
  float temperature = dht_.readTemperature();

  // 読み取りエラーチェック
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("[Sensor] 読み取りエラー");
    return SensorData(0.0f, 0.0f, 0.0f, false);
  }

  // オフセット適用
  temperature += temperatureOffset_;
  humidity += humidityOffset_;

  // 不快指数（DI）を計算
  float di = calculateDiscomfortIndex(temperature, humidity);

  Serial.printf("[Sensor] 温度: %.1f°C, 湿度: %.1f%%, DI: %.1f\n", temperature, humidity, di);

  return SensorData(temperature, humidity, di, true);
}

/**
 * 不快指数（Discomfort Index: DI）を計算
 * @param temperature 温度（摂氏）
 * @param humidity    湿度（%）
 * @return 不快指数（DI値）
 *
 * 計算式: DI = 0.81T + 0.01H(0.99T - 14.3) + 46.3
 * DI値の目安:
 *   〜55: 寒い
 *   55〜60: 肌寒い
 *   60〜65: 何も感じない
 *   65〜70: 快い
 *   70〜75: 暑くない
 *   75〜80: やや暑い
 *   80〜85: 暑くて汗が出る
 *   85〜  : 暑くてたまらない
 */
float EnvironmentSensor::calculateDiscomfortIndex(float temperature, float humidity) {
  float di = 0.81f * temperature + 0.01f * humidity * (0.99f * temperature - 14.3f) + 46.3f;
  return di;
}
