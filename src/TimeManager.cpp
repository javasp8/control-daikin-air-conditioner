/**
 * TimeManager.cpp
 *
 * 時刻管理クラスの実装
 */

#include "TimeManager.h"

/**
 * コンストラクタ
 * NTPサーバーとタイムゾーン設定を初期化します。
 */
TimeManager::TimeManager(const char* ntpServer, long gmtOffsetSec, int daylightOffsetSec)
  : ntpServer_(ntpServer),
    gmtOffsetSec_(gmtOffsetSec),
    daylightOffsetSec_(daylightOffsetSec) {
}

/**
 * NTPサーバーから時刻を同期
 * WiFi接続後に呼び出してください。
 */
bool TimeManager::syncTime() {
  Serial.println("[Time] NTP時刻同期を開始...");

  // NTPサーバーと接続して時刻を設定
  // configTime(GMTオフセット秒, サマータイムオフセット秒, NTPサーバー)
  configTime(gmtOffsetSec_, daylightOffsetSec_, ntpServer_);

  // 時刻同期が完了するまで待機（最大10秒）
  int retryCount = 0;
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo) && retryCount < 10) {
    Serial.print(".");
    delay(1000);
    retryCount++;
  }

  if (retryCount >= 10) {
    Serial.println("\n[Time] 時刻同期失敗");
    return false;
  }

  // 同期成功：現在の日時を表示
  Serial.println("\n[Time] 時刻同期成功");
  printCurrentTime();

  return true;
}

/**
 * 現在の時刻情報を取得
 */
bool TimeManager::getCurrentTime(struct tm& timeinfo) {
  return getLocalTime(&timeinfo);
}

/**
 * 現在の時（0-23）を取得
 */
int TimeManager::getCurrentHour() {
  struct tm timeinfo;
  if (!getCurrentTime(timeinfo)) {
    return -1;  // 取得失敗
  }
  return timeinfo.tm_hour;
}

/**
 * 現在の月（1-12）を取得
 */
int TimeManager::getCurrentMonth() {
  struct tm timeinfo;
  if (!getCurrentTime(timeinfo)) {
    return -1;  // 取得失敗
  }
  return timeinfo.tm_mon + 1;  // tm_monは0-11なので+1する
}

/**
 * 7〜9月（夏季）かどうかを判定
 */
bool TimeManager::isSummerSeason() {
  int month = getCurrentMonth();
  if (month == -1) {
    return false;  // 時刻取得失敗時はfalse
  }
  return (month >= 7 && month <= 9);
}

/**
 * 現在の日時をシリアル出力
 */
void TimeManager::printCurrentTime() {
  struct tm timeinfo;
  if (!getCurrentTime(timeinfo)) {
    Serial.println("[Time] 時刻取得失敗");
    return;
  }

  Serial.printf("[Time] 現在時刻: %04d/%02d/%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900,  // 年（1900年からの経過年数）
                timeinfo.tm_mon + 1,       // 月（0-11なので+1）
                timeinfo.tm_mday,          // 日
                timeinfo.tm_hour,          // 時
                timeinfo.tm_min,           // 分
                timeinfo.tm_sec);          // 秒
}

/**
 * フォーマットされた日時文字列を取得
 */
String TimeManager::getFormattedTime(const char* format) {
  struct tm timeinfo;
  if (!getCurrentTime(timeinfo)) {
    return "";  // 時刻取得失敗時は空文字列を返す
  }

  char buffer[64];
  strftime(buffer, sizeof(buffer), format, &timeinfo);
  return String(buffer);
}

/**
 * フォーマットされた日時を指定バッファに書き込む（組み込み環境推奨）
 */
bool TimeManager::getFormattedTime(const char* format, char* buffer, size_t bufferSize) {
  struct tm timeinfo;
  if (!getCurrentTime(timeinfo)) {
    buffer[0] = '\0';  // 空文字列にする
    return false;
  }

  size_t written = strftime(buffer, bufferSize, format, &timeinfo);
  return (written > 0);
}
