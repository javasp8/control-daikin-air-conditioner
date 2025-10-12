# エアコン自動制御システム

ESP32学習用

ESP32を使用したダイキンエアコンの自動制御システムです。季節・時間帯・温湿度に基づいて最適なモードでエアコンを自動制御し、年間を通じて快適な室内環境を維持しつつ、電気代の削減を実現します。

## 主な機能

- 🌡️ **快適温度・湿度帯の維持**: 室温24.5〜26.5度、湿度40〜60%を目標範囲として自動制御
- 🗓️ **季節別制御**: 春・夏・秋・冬の4季節で異なる制御ロジックを実装
- 🌙 **夜間自動停止**: 春・秋・冬季の23:00〜7:00は自動停止（電気代削減）
- ❄️ **極寒日対応**: 最低気温0度以下の予報日は夜間も暖房18度で運転継続
- 📺 **OLEDディスプレイ**: リアルタイムでセンサー情報と天気予報を表示
- 🌐 **WiFi対応**: NTP時刻同期、天気予報API連携
- ☀️ **天気予報連携**: Open-Meteo APIから気温予報を取得し、制御に活用

## ハードウェア構成

### ブレッドボード配置（参考）

![17770105-9143-4C84-A11C-43A7B0C8FE47_1_105_c](https://github.com/user-attachments/assets/6c0e22e0-113f-4012-82fe-5ec6db5d4032)


### 必要な部品

| 部品 | 型番・仕様 | 用途 |
|------|-----------|------|
| マイコン | ESP32-DevKitC | メイン制御 |
| 温湿度センサー | 温湿度センサー モジュール AM2302 | 環境測定 |
| 赤外線LED | 5mm赤外線LED 940nm OSI5LA5113A | エアコン制御 |
| 赤外線受信モジュール | 赤外線リモコン受信モジュールOSRB38C9AA | リモコン学習 |
| OLEDディスプレイ | 0.96インチ 128×64ドット有機ELディスプレイ(OLED) | 情報表示 |

### ピン接続

```
ESP32          デバイス
GPIO32    →    DHT22 (Data)
GPIO5     →    IR LED (送信)
GPIO18    →    IR受信モジュール
GPIO21    →    OLED SDA
GPIO22    →    OLED SCL
3.3V      →    センサー/ディスプレイ電源
GND       →    共通GND
```

## ソフトウェア構成

### プロジェクト構造

```
ControliAirConditioner/
├── include/
│   ├── AirConditionerController.h  # エアコン制御（季節別ロジック）
│   ├── EnvironmentSensor.h         # 温湿度センサー
│   ├── DisplayController.h         # ディスプレイ制御
│   ├── WiFiManager.h               # WiFi接続管理
│   ├── TimeManager.h               # 時刻管理
│   ├── WeatherForecast.h           # 天気予報取得
│   ├── secrets.h.example           # 認証情報テンプレート
│   └── secrets.h                   # WiFi認証情報（.gitignore）
├── src/
│   ├── main.cpp                    # メイン制御
│   ├── AirConditionerController.cpp
│   ├── EnvironmentSensor.cpp
│   ├── DisplayController.cpp
│   ├── WiFiManager.cpp
│   ├── TimeManager.cpp
│   └── WeatherForecast.cpp
└── platformio.ini                  # ビルド設定
```

### クラス設計

#### 🎛️ AirConditionerController
エアコンの赤外線制御を担当
- ダイキンエアコンのIR信号送信
- 季節判定（春・夏・秋・冬）
- 時間帯判定（日中・夜間）
- 極寒日判定（最低気温0度以下）
- 季節・時間帯・温湿度に基づく最適モード決定
- エアコン停止状態の管理（重複送信防止）

#### 🌡️ EnvironmentSensor
温湿度センサーの読み取り
- DHT22センサー制御
- オフセット補正機能
- エラーハンドリング

#### 📺 DisplayController
OLEDディスプレイの制御
- センサーデータ表示
- 天気予報データ表示
- 起動画面表示
- リアルタイム更新

#### 🌐 WiFiManager
WiFi接続の管理
- 自動接続・再接続
- 接続状態監視
- タイムアウト処理

#### ⏰ TimeManager
時刻管理とNTP同期
- NTPサーバーからの時刻取得
- 日本時間（JST）への変換
- 現在時刻の取得機能

#### ☀️ WeatherForecast
天気予報の取得と管理
- Open-Meteo API連携
- 起動時および1時間ごとに天気予報を自動取得
- 最高・最低気温、天気コードを取得
- 天気コードを読みやすい文字列に変換（Clear, Cloudy, Fog, Rain, Snow, Storm）

## セットアップ

### 1. 環境構築

```bash
# PlatformIOのインストール（VS Code拡張機能として推奨）
# または
pip install platformio

# プロジェクトのクローン
git clone <repository-url>
cd ControliAirConditioner
```

### 2. WiFi認証情報の設定

```bash
# テンプレートをコピー
cp include/secrets.h.example include/secrets.h

# secrets.h を編集して実際のWiFi情報を入力
# const char* SSID = "YOUR_WIFI_SSID";
# const char* PASSWORD = "YOUR_WIFI_PASSWORD";
```

⚠️ **重要**: `secrets.h` は `.gitignore` に含まれており、Gitにコミットされません。

### 3. ビルド＆アップロード

```bash
# ビルド
pio run

# ESP32にアップロード
pio run -t upload

# シリアルモニタで動作確認
pio device monitor
```

## 設定のカスタマイズ

`src/main.cpp` の各 namespace で設定を変更できます：

### ハードウェアピン設定
```cpp
namespace HardwareConfig {
  constexpr uint8_t DHT_PIN = 32;        // DHT22ピン
  constexpr uint8_t IR_RECV_PIN = 18;    // IR受信ピン
  constexpr uint8_t IR_SEND_PIN = 5;     // IR送信ピン
}
```

### センサー補正
```cpp
namespace SensorConfig {
  constexpr float TEMP_OFFSET = -2.0f;   // 温度補正（℃）
  constexpr float HUM_OFFSET = 0.0f;     // 湿度補正（%）
}
```

### 時刻設定
```cpp
namespace TimeConfig {
  const char* NTP_SERVER = "ntp.nict.jp";
  const long GMT_OFFSET_SEC = 9 * 3600;     // 日本時間（JST）
  const int DAYLIGHT_OFFSET_SEC = 0;        // サマータイムなし
}
```

### タイミング設定
```cpp
namespace TimingConfig {
  constexpr unsigned long SENSOR_READ_INTERVAL_MS = 2000;   // センサー読取間隔
  constexpr unsigned long CONTROL_INTERVAL_MS = 60000;      // エアコン制御間隔
}
```

### 天気予報設定
```cpp
namespace WeatherConfig {
  constexpr float LATITUDE = 35.653204f;   // 緯度（デフォルト：東京）
  constexpr float LONGITUDE = 139.688272f; // 経度（デフォルト：東京）
}
```

## 制御仕様

### 快適温度・湿度帯
- **目標室温**: 24.5〜26.5度
- **目標湿度**: 40〜60%
- 両方の範囲内であればエアコン停止

### エアコンモード
| モード | 用途 |
|--------|------|
| 暖房23.5度 | 春・秋・冬の日中、室温24.5度未満 |
| 暖房18度 | 冬の極寒日（最低気温0度以下）の夜間 |
| 冷房25度 | 室温26.5度超 |
| 除湿-1.5度 | 室温24.5〜26.5度、湿度61%以上 |
| 停止 | 快適範囲内、または夜間（夏季以外） |

### 季節別制御の概要

#### 春季（3〜5月）・秋季（10〜11月）
- **日中（7:00〜23:00）**: 温度・湿度に応じて制御
- **夜間（23:00〜7:00）**: 停止

#### 夏季（6〜9月）
- **24時間運転**: 熱中症予防のため夜間も運転
- 過冷房防止機能付き（24.5度未満で停止）

#### 冬季（12〜2月）
- **日中（7:00〜23:00）**: 暖房のみ使用
- **夜間（23:00〜7:00）**: 通常は停止、極寒日は暖房18度
- 冷房・除湿は一切使用しない（季節的に不要）

## 動作ログ例

```
========================================
エアコン自動制御システム起動
========================================

[WiFi] WiFi接続を開始します...
[WiFi] SSID: YourWiFi
.....
[WiFi] WiFi接続成功！
[WiFi] IPアドレス: 192.168.1.100
[WiFi] 電波強度 (RSSI): -45 dBm
[System] WiFi接続完了

[Time] NTP時刻同期を開始...
[Time] 時刻同期成功
[Time] 現在時刻: 2025/10/11 22:30:15

[Weather] WeatherForecast初期化完了
[Weather] API URL: http://api.open-meteo.com/v1/forecast?...
[Weather] 初回天気予報データ取得開始
[Weather] APIリクエスト送信: http://api.open-meteo.com/v1/forecast?...
[Weather] APIレスポンス受信成功
[Weather] 天気予報データ更新完了
[Weather]   - 最高気温: 18.5 °C
[Weather]   - 最低気温: 15.4 °C
[Weather]   - 天気コード: 55
[Weather]   - 天気: Rain

[Sensor] 環境センサー初期化完了
[Display] ディスプレイ初期化成功
[AC] エアコンコントローラー初期化完了
[System] システム起動完了
========================================

[Sensor] 温度: 23.8°C, 湿度: 55.0%
[AC] 温度:23.8℃, 湿度:55.0%, 月:10, 時:15
[AC] 季節: 秋季
[AC] 秋季・日中: 室温23.8℃ < 24.5℃ → 暖房23.5度
[AC] 暖房23.5度 送信開始
[AC] 暖房23.5度 送信完了

[Sensor] 温度: 25.2°C, 湿度: 52.0%
[AC] 温度:25.2℃, 湿度:52.0%, 月:10, 時:23
[AC] 季節: 秋季
[AC] 秋季・夜間 → 停止
[AC] エアコン停止 送信開始
[AC] エアコン停止 送信完了
```

## 使用ライブラリ

| ライブラリ | バージョン | 用途 |
|-----------|-----------|------|
| IRremoteESP8266 | ^2.8.6 | 赤外線送受信 |
| DHT sensor library | ^1.4.4 | DHT22制御 |
| Adafruit SSD1306 | ^2.5.7 | OLEDディスプレイ |
| Adafruit GFX Library | ^1.11.3 | グラフィック描画 |
| Adafruit Unified Sensor | ^1.1.14 | センサー統合 |
| ArduinoJson | ^7.2.1 | JSON解析（天気予報API用） |

## 参考資料

- [IRremoteESP8266 Documentation](https://github.com/crankyoldgit/IRremoteESP8266)
- [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [Open-Meteo Weather API](https://open-meteo.com/)
- [PlatformIO Documentation](https://docs.platformio.org/)

## パーツ参考

電子工作用のパーツは秋月電子通商にて購入

```
1.[112612]5mm赤外線LED 940nm OSI5LA5113A グレー:1パック10個入
  価格：￥120 x 数量：1 = 合計：￥120

2.[109406]トランジスター PN2222 30V600mA:1テープ20個入
  価格：￥270 x 数量：1 = 合計：￥270

3.[115673]ESP32-DevKitC-32E ESP32-WROOM-32E開発ボード 4MB:1個
  価格：￥1,800 x 数量：1 = 合計：￥1,800

4.[107002]温湿度センサー モジュール AM2302:1個
  価格：￥1,160 x 数量：1 = 合計：￥1,160

5.[125101]カーボン抵抗(炭素皮膜抵抗) 1/4W100Ω:1袋100本入
  価格：￥150 x 数量：1 = 合計：￥150

6.[100288]ブレッドボード・ジャンパーワイヤ 14種類×10本:1セット
  価格：￥700 x 数量：1 = 合計：￥700

7.[100315]ブレッドボード EIC-801:1個
  価格：￥370 x 数量：2 = 合計：￥740

8.[112031]0.96インチ 128×64ドット有機ELディスプレイ(OLED) 白色:1個
  価格：￥580 x 数量：1 = 合計：￥580

9.[125100]カーボン抵抗(炭素皮膜抵抗) 1/4W10Ω:1袋100本入
  価格：￥170 x 数量：1 = 合計：￥170

10.[125103]カーボン抵抗(炭素皮膜抵抗) 1/4W10kΩ:1袋100本入
  価格：￥100 x 数量：1 = 合計：￥100

11.[104659]赤外線リモコン受信モジュールOSRB38C9AA:1パック2個入
  価格：￥100 x 数量：1 = 合計：￥100
```
