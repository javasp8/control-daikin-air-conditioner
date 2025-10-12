/**
 * WiFiManager.cpp
 *
 * WiFi接続管理クラスの実装
 */

#include "WiFiManager.h"

/**
 * コンストラクタ
 * WiFi接続に必要な情報を初期化します。
 */
WiFiManager::WiFiManager(const char* ssid, const char* password, unsigned long timeoutMs)
  : ssid_(ssid),
    password_(password),
    timeoutMs_(timeoutMs) {
}

/**
 * WiFiに接続
 */
bool WiFiManager::connect() {
  Serial.println("\n[WiFi] WiFi接続を開始します...");
  Serial.printf("[WiFi] SSID: %s\n", ssid_);

  // WiFiモードをステーションモード（クライアント）に設定
  WiFi.mode(WIFI_STA);

  // WiFi接続を開始（SSID、パスワードを指定）
  WiFi.begin(ssid_, password_);

  // 接続試行の開始時刻を記録
  unsigned long startTime = millis();

  // 接続完了またはタイムアウトまで待機
  // WiFi.status()がWL_CONNECTEDになるまでループ
  while (WiFi.status() != WL_CONNECTED) {
    // タイムアウトチェック
    if (millis() - startTime > timeoutMs_) {
      Serial.println("\n[WiFi] 接続タイムアウト");
      return false;  // 接続失敗
    }

    // 進捗表示（500msごとにドットを表示）
    delay(500);
    Serial.print(".");
  }

  // 接続成功
  Serial.println("\n[WiFi] WiFi接続成功！");
  printConnectionInfo();

  return true;
}

/**
 * WiFi接続状態を確認し、切断時は再接続を試みる
 */
bool WiFiManager::checkConnection() {
  // WiFi.status()で現在の接続状態を確認
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] WiFi切断を検出、再接続を試みます...");
    return connect();  // 再接続を試みる
  }
  return true;  // 接続中
}

/**
 * WiFiが接続中かどうかを確認
 */
bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

/**
 * 接続情報を表示
 */
void WiFiManager::printConnectionInfo() {
  Serial.print("[WiFi] IPアドレス: ");
  Serial.println(WiFi.localIP());  // 取得したIPアドレスを表示
  Serial.print("[WiFi] 電波強度 (RSSI): ");
  Serial.print(WiFi.RSSI());       // 電波強度を表示（dBm）
  Serial.println(" dBm");
}
