/**
 * AutoStopController.h
 *
 * エアコン自動停止制御クラス
 * 指定時刻にエアコンを自動停止する機能を提供します。
 */

#ifndef AUTO_STOP_CONTROLLER_H
#define AUTO_STOP_CONTROLLER_H

#include <Arduino.h>
#include "AirConditionerController.h"
#include "TimeManager.h"

/**
 * エアコン自動停止制御クラス
 *
 * 主な機能:
 * - 指定時刻（デフォルト23時）にエアコンを自動停止
 * - 7〜9月（夏季）は自動停止をスキップ
 * - 1日1回のみ停止（翌日0時にリセット）
 */
class AutoStopController {
public:
  /**
   * コンストラクタ
   * @param ac エアコンコントローラーの参照
   * @param timeMgr 時刻管理クラスの参照
   * @param stopHour 自動停止する時刻（0-23、デフォルト23時）
   */
  AutoStopController(AirConditionerController& ac, TimeManager& timeMgr, int stopHour = 23);

  /**
   * 自動停止チェックを実行
   * loop関数内で定期的に呼び出してください。
   * @return true: 停止を実行した, false: 停止不要または既に停止済み
   */
  bool check();

  /**
   * 自動停止機能の有効/無効を設定
   * @param enabled true: 有効, false: 無効
   */
  void setEnabled(bool enabled);

  /**
   * 自動停止機能が有効かどうかを取得
   * @return true: 有効, false: 無効
   */
  bool isEnabled() const { return enabled_; }

private:
  AirConditionerController& ac_;   // エアコンコントローラーの参照
  TimeManager& timeMgr_;           // 時刻管理クラスの参照
  int stopHour_;                   // 自動停止する時刻（0-23）
  bool enabled_;                   // 自動停止機能の有効/無効
  bool stoppedToday_;              // 今日既に停止したかのフラグ
  int lastPrintedHour_;            // 最後に時刻を表示した時（デバッグ用）
};

#endif // AUTO_STOP_CONTROLLER_H
