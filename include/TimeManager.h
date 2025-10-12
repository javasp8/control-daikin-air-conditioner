/**
 * TimeManager.h
 *
 * 時刻管理クラス
 * NTP時刻同期と時刻情報の取得を担当します。
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

/**
 * 時刻管理クラス
 *
 * 主な機能:
 * - NTPサーバーからの時刻同期
 * - 日本時間（JST）への自動変換
 * - 現在時刻の取得
 */
class TimeManager {
public:
  /**
   * コンストラクタ
   * @param ntpServer NTPサーバーのアドレス
   * @param gmtOffsetSec GMTオフセット（秒）
   * @param daylightOffsetSec サマータイムオフセット（秒）
   */
  TimeManager(const char* ntpServer, long gmtOffsetSec, int daylightOffsetSec);

  /**
   * NTPサーバーから時刻を同期
   * @return true: 同期成功, false: 同期失敗
   */
  bool syncTime();

  /**
   * 現在の時刻情報を取得
   * @param timeinfo 時刻情報を格納する構造体（出力）
   * @return true: 取得成功, false: 取得失敗
   */
  bool getCurrentTime(struct tm& timeinfo);

  /**
   * 現在の時（0-23）を取得
   * @return 時（0-23）、取得失敗時は -1
   */
  int getCurrentHour();

  /**
   * 現在の月（1-12）を取得
   * @return 月（1-12）、取得失敗時は -1
   */
  int getCurrentMonth();

  /**
   * 7〜9月（夏季）かどうかを判定
   * @return true: 7〜9月, false: それ以外
   */
  bool isSummerSeason();

  /**
   * 現在の日時をシリアル出力
   */
  void printCurrentTime();

  /**
   * フォーマットされた日時文字列を取得
   * @param format フォーマット文字列（strftime形式、例: "%Y-%m-%d %H:%M"）
   * @return フォーマットされた日時文字列、取得失敗時は空文字列
   */
  String getFormattedTime(const char* format);

  /**
   * フォーマットされた日時を指定バッファに書き込む（組み込み環境推奨）
   * @param format フォーマット文字列（strftime形式、例: "%Y-%m-%d %H:%M"）
   * @param buffer 出力先バッファ
   * @param bufferSize バッファサイズ
   * @return true: 成功, false: 失敗
   */
  bool getFormattedTime(const char* format, char* buffer, size_t bufferSize);

  /**
   * よく使うフォーマット定義
   */
  static constexpr const char* FORMAT_DATETIME = "%Y-%m-%d %H:%M";
  static constexpr const char* FORMAT_DATE_ONLY = "%Y-%m-%d";
  static constexpr const char* FORMAT_TIME_ONLY = "%H:%M:%S";

private:
  const char* ntpServer_;         // NTPサーバーアドレス
  long gmtOffsetSec_;             // GMTオフセット（秒）
  int daylightOffsetSec_;         // サマータイムオフセット（秒）
};

#endif // TIME_MANAGER_H
