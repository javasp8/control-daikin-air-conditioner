#include "DisplayController.h"

DisplayController::DisplayController(uint8_t width, uint8_t height, TwoWire* wire, int8_t resetPin, uint8_t address)
  : display_(width, height, wire, resetPin), width_(width), height_(height) {
}

bool DisplayController::begin() {
  if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[Display] 初期化失敗");
    return false;
  }
  Serial.println("[Display] ディスプレイ初期化完了");
  return true;
}

void DisplayController::showStartupScreen() {
  display_.clearDisplay();

  // シンプルなテキストベースのスプラッシュ画面
  display_.setTextSize(2);
  display_.setTextColor(SSD1306_WHITE);

  // タイトル
  display_.setCursor(25, 10);
  display_.println("ERNEST");

  // サブタイトル
  display_.setTextSize(1);
  display_.setCursor(15, 35);
  display_.println("Air Conditioner");
  display_.setCursor(30, 48);
  display_.println("Controller");

  display_.display();
}

void DisplayController::showSensorData(const SensorData& data, const char* datetime) {
  if (!data.isValid) {
    showError("Sensor Error");
    return;
  }

  display_.clearDisplay();

  // 温度表示（大きめ）
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.println("Temp");

  display_.setTextSize(2);
  display_.setCursor(5, 12);
  display_.print(data.temperature, 1);
  display_.setTextSize(1);
  display_.setCursor(62, 18);
  display_.println("C");

  // 湿度表示（大きめ）
  display_.setTextSize(1);
  display_.setCursor(78, 0);
  display_.println("Hum");

  display_.setTextSize(2);
  display_.setCursor(75, 12);
  display_.print(data.humidity, 0);
  display_.setTextSize(1);
  display_.setCursor(110, 18);
  display_.println("%");

  // 区切り線
  display_.drawLine(0, 30, width_, 30, SSD1306_WHITE);

  // 現在日付
  display_.setTextSize(1);
  display_.setCursor(0, 36);
  display_.print(datetime);

  // 不快指数（DI）表示（小さめ）
  display_.setTextSize(1);
  display_.setCursor(0, 46);
  display_.print("DI: ");
  display_.print(data.discomfortIndex, 1);

  // DI値のステータス表示
  display_.setCursor(50, 46);
  if (data.discomfortIndex >= 77.0f) {
    display_.print("(Hot)");
  } else if (data.discomfortIndex >= 75.0f) {
    display_.print("(Warm)");
  } else if (data.discomfortIndex >= 70.0f) {
    display_.print("(Comfy)");
  } else {
    display_.print("(Cool)");
  }

  // 表示実行
  display_.display();
}

void DisplayController::showSensorDataWithWeather(const SensorData& data, const char* datetime, const WeatherData& weather) {
  if (!data.isValid) {
    showError("Sensor Error");
    return;
  }

  display_.clearDisplay();

  // 現在日時（最上部）
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.print(datetime);

  // 区切り線
  display_.drawLine(0, 10, width_, 10, SSD1306_WHITE);

  // 温度表示（やや小さめに調整）
  display_.setTextSize(1);
  display_.setCursor(0, 14);
  display_.println("Temp");

  display_.setTextSize(2);
  display_.setCursor(5, 24);
  display_.print(data.temperature, 1);
  display_.setTextSize(1);
  display_.setCursor(55, 28);
  display_.println("C");

  // 湿度表示（やや小さめに調整）
  display_.setTextSize(1);
  display_.setCursor(70, 14);
  display_.println("Hum");

  display_.setTextSize(2);
  display_.setCursor(70, 24);
  display_.print(data.humidity, 0);
  display_.setTextSize(1);
  display_.setCursor(105, 28);
  display_.println("%");

  // 不快指数（DI）表示
  display_.setTextSize(1);
  display_.setCursor(0, 44);
  display_.print("DI:");
  display_.print(data.discomfortIndex, 1);

  // DI値のステータス表示
  display_.setCursor(48, 44);
  if (data.discomfortIndex >= 77.0f) {
    display_.print("(Hot)");
  } else if (data.discomfortIndex >= 75.0f) {
    display_.print("(Warm)");
  } else if (data.discomfortIndex >= 70.0f) {
    display_.print("(Comfy)");
  } else {
    display_.print("(Cool)");
  }

  // 天気予報表示（画面最下部に配置を最適化）
  display_.setTextSize(1);
  if (weather.isValid) {
    // 天気文字列（左側）
    display_.setCursor(0, 56);
    display_.print(weather.weatherString);

    // 最高・最低気温（右側）
    display_.setCursor(60, 56);
    display_.print(weather.tempMin, 1);
    display_.print("/");
    display_.print(weather.tempMax, 1);
    display_.print("C");
  } else {
    display_.setCursor(0, 56);
    display_.print("Weather: N/A");
  }

  // 表示実行
  display_.display();
}

void DisplayController::showError(const char* message) {
  display_.clearDisplay();
  display_.setTextSize(2);
  display_.setCursor(20, 25);
  display_.println(message);
  display_.display();
}
