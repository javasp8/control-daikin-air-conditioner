/**
 * AutoStopController.cpp
 *
 * エアコン自動停止制御クラスの実装
 */

#include "AutoStopController.h"

/**
 * コンストラクタ
 */
AutoStopController::AutoStopController(AirConditionerController& ac,
                                       TimeManager& timeMgr,
                                       int stopHour)
  : ac_(ac),
    timeMgr_(timeMgr),
    stopHour_(stopHour),
    enabled_(true),
    stoppedToday_(false),
    lastPrintedHour_(-1) {
}

/**
 * 自動停止チェックを実行
 */
bool AutoStopController::check() {
  // 自動停止機能が無効の場合は何もしない
  if (!enabled_) {
    return false;
  }

  // 現在時刻を取得
  int currentHour = timeMgr_.getCurrentHour();
  int currentMonth = timeMgr_.getCurrentMonth();

  // 時刻が取得できない場合（WiFi未接続など）は何もしない
  if (currentHour == -1 || currentMonth == -1) {
    return false;
  }

  // デバッグ用：1時間に1回、現在時刻を表示
  if (currentHour != lastPrintedHour_) {
    Serial.printf("[AutoStop] 現在時刻: %02d時, 月: %d月\n", currentHour, currentMonth);
    lastPrintedHour_ = currentHour;
  }

  // 7〜9月は自動停止しない
  if (timeMgr_.isSummerSeason()) {
    stoppedToday_ = false;  // 夏季期間はフラグをリセット
    return false;
  }

  // 指定時刻で、まだ今日停止していない場合
  if (currentHour == stopHour_ && !stoppedToday_) {
    Serial.println("[AutoStop] ========================================");
    Serial.printf("[AutoStop] %d時になりました。エアコンを自動停止します（%d月は対象期間）\n",
                  stopHour_, currentMonth);
    Serial.println("[AutoStop] ========================================");

    // エアコンを停止
    ac_.setMode(ACMode::OFF);

    // 今日停止したフラグを立てる（翌日0時までこれ以上停止しない）
    stoppedToday_ = true;

    return true;
  }

  // 0時になったらフラグをリセット（翌日の停止に備える）
  if (currentHour == 0) {
    stoppedToday_ = false;
  }

  return false;
}

/**
 * 自動停止機能の有効/無効を設定
 */
void AutoStopController::setEnabled(bool enabled) {
  enabled_ = enabled;
  Serial.printf("[AutoStop] 自動停止機能: %s\n", enabled ? "有効" : "無効");
}
