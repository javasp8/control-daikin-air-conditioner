/**
 * WiFiManager.h
 *
 * WiFi接続管理クラス
 * WiFiの接続・切断監視・再接続を担当します。
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * WiFi接続管理クラス
 *
 * 主な機能:
 * - WiFiアクセスポイントへの接続
 * - 接続状態の監視
 * - 切断時の自動再接続
 */
class WiFiManager {
public:
  /**
   * コンストラクタ
   * @param ssid WiFiのSSID
   * @param password WiFiのパスワード
   * @param timeoutMs 接続タイムアウト時間（ミリ秒）
   */
  WiFiManager(const char* ssid, const char* password, unsigned long timeoutMs = 10000);

  /**
   * WiFiに接続
   * @return true: 接続成功, false: 接続失敗
   */
  bool connect();

  /**
   * WiFi接続状態を確認し、切断時は再接続を試みる
   * loop関数内で定期的に呼び出してください。
   * @return true: 接続中, false: 切断中
   */
  bool checkConnection();

  /**
   * WiFiが接続中かどうかを確認
   * @return true: 接続中, false: 切断中
   */
  bool isConnected();

  /**
   * 接続情報を表示
   */
  void printConnectionInfo();

private:
  const char* ssid_;              // WiFi SSID
  const char* password_;          // WiFiパスワード
  unsigned long timeoutMs_;       // 接続タイムアウト時間
};

#endif // WIFI_MANAGER_H
