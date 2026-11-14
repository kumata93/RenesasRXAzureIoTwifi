# ckrx65n_wifi_azureiot

Renesas **CK-RX65N**（RX65N マイコン）上で **Azure RTOS (ThreadX / NetX Duo / NetX Secure TLS)** と **SX-ULPGN Wi-Fi モジュール** を用いて、**Azure IoT Hub** へ接続するサンプル／ベースプロジェクトです。

---

## 1. 全体構成（ディレクトリ構成）

```text
ckrx65n_wifi_azureiot/
├── ckrx65n_wifi_azureiot.scfg          # Renesas Smart Configurator 設定ファイル
├── r01an6455ej0103-rx-azure-rtos.pdf   # Renesas Azure RTOS アプリケーションノート
├── src/
│   ├── main.c                          # エントリポイント、NetX/ThreadX 初期化、SNTP/DNS 初期化
│   ├── hardware_setup.c / .h           # CMT タイマによる ThreadX システムティック生成
│   ├── sample_network.c                # Wi-Fi モジュール初期化・AP 接続・IP 設定
│   ├── sample_config.h                 # Azure IoT Hub / DPS 接続設定（認証情報・機能有効/無効）
│   ├── sample_device_identity.c        # X.509 デバイス証明書・秘密鍵定義
│   ├── sample_azure_iot_embedded_sdk.c # Azure IoT Hub クライアント & 各機能スレッド
│   ├── sample_azure_iot_embedded_sdk_connect.c # 指数バックオフ付き再接続モニタ
│   ├── nx_driver_rx_wifi.c / .h        # NetX Duo TCP/IP オフロード用 Wi-Fi ポート層ドライバ
│   ├── nx_azure_iot_cert.c / .h        # Azure IoT 用ルート CA 証明書
│   ├── nx_azure_iot_ciphersuites.c / .h# TLS 暗号スイート設定
│   ├── demo_printf.c / .h              # デバッグログ出力 (SCI5)
│   ├── rtos_config/                    # Azure RTOS 静的オブジェクト定義
│   └── smc_gen/                        # Smart Configurator 生成コード (FIT ドライバ群)
│       ├── r_bsp/                      # RX65N BSP
│       ├── r_cmt_rx/                   # CMT タイマドライバ（システムティック用）
│       ├── r_sci_rx/                   # SCI (UART) ドライバ（SX-ULPGN との通信用）
│       ├── r_byteq/                    # リングバッファ (BYTEQ)
│       ├── r_wifi_sx_ulpgn/            # SX-ULPGN Wi-Fi モジュール FIT ドライバ (AT コマンド/透過モード)
│       └── r_config/                   # 各 FIT モジュールの設定ヘッダ
└── libs/
    ├── threadx/                        # Azure RTOS ThreadX カーネル
    ├── netxduo/                        # Azure RTOS NetX Duo (TCP/IP オフロード有効)
    ├── netxduo_addons/                 # Azure IoT SDK, NX Secure (TLS), MQTT, DNS, SNTP 等
    └── filex/                          # Azure RTOS FileX
```

---

## 2. システムアーキテクチャ

本プロジェクトは **NetX Duo の TCP/IP オフロード機能 (`NX_ENABLE_TCPIP_OFFLOAD`)** を使用しています。
TLS (NetX Secure) や MQTT、DNS、SNTP などの上位プロトコルは MCU 側の NetX Duo 上で動作し、実際の TCP/UDP ソケット送受信はポート層 (`nx_driver_rx_wifi.c`) を介して SX-ULPGN モジュール側の TCP/IP スタックにオフロードされます。

```text
+-------------------------------------------------------------------+
| Application (sample_azure_iot_embedded_sdk.c)                     |
|   Telemetry / C2D / Direct Method / Device Twin / Connection Mon  |
+-------------------------------------------------------------------+
| Middleware (libs/netxduo_addons)                                  |
|   Azure IoT Hub Client -> MQTT -> NetX Secure (TLS) / DNS / SNTP  |
+-------------------------------------------------------------------+
| NetX Duo Core (NX_ENABLE_TCPIP_OFFLOAD)                           |
|   NX_IP / NX_TCP_SOCKET / NX_UDP_SOCKET                           |
+-------------------------------------------------------------------+
| Port Layer (src/nx_driver_rx_wifi.c)                              |
|   _nx_driver_tcpip_handler()  [送信・接続・切断]                   |
|   _nx_driver_thread_entry()   [受信ポーリング]                     |
+-------------------------------------------------------------------+
| Wi-Fi Driver (src/smc_gen/r_wifi_sx_ulpgn)                        |
|   R_WIFI_SX_ULPGN_* API / AT コマンド制御 / 透過モード切替         |
+-------------------------------------------------------------------+
| Serial & Queue (r_sci_rx + r_byteq)                               |
|   SCI0 (データ/コマンド), SCI1 (コマンド), RTS/CTS フロー制御       |
+-------------------------------------------------------------------+
| Hardware: SX-ULPGN Wi-Fi Module                                   |
+-------------------------------------------------------------------+
```

### ソケット管理と受信データの流れ
* **同時利用可能ソケット数**: SX-ULPGN の仕様に合わせて最大 **4 本** (`NX_DRIVER_SOCKETS_MAXIMUM` / `WIFI_CFG_CREATABLE_SOCKETS` = `4`) に制限されています。
* **送信・接続**: NetX の各 API が `nx_interface_tcpip_offload_handler` (`_nx_driver_tcpip_handler`) を呼び出し、`R_WIFI_SX_ULPGN_*` API 経由で UART (SCI) からモジュールへ送信します。
* **受信**:
  1. UART RXI 割り込み内で `cb_sci_hsuart1_for_data()` が受信バイトをソケットごとのリングバッファ (`g_sock_tbl[sock_idx].byteq_hdl`) に格納します。
  2. `Driver Thread` (`_nx_driver_thread_entry`) が定期的に `R_WIFI_SX_ULPGN_ReceiveSocket()` を呼び出し、データを `NX_PACKET` に詰めて `_nx_tcp_socket_driver_packet_receive()` / `_nx_udp_socket_driver_packet_receive()` で NetX へ渡します。

---

## 3. タスク（ThreadX スレッド）と排他制御

システムティックは `TX_TIMER_TICKS_PER_SECOND = 100` (1 tick = 10 ms) で動作します。

| スレッド名 | 優先度 | 生成元 | 役割 |
| :--- | :---: | :--- | :--- |
| **NetX IP Instance 0** | `1` | `main.c` (`nx_ip_create`) | NetX Duo IP インスタンス内部処理 |
| **Driver Thread** | `2` | `nx_driver_rx_wifi.c` | 各ソケットからの受信ポーリング (`NX_DRIVER_THREAD_INTERVAL = 5` ticks) |
| **Demo Thread** | `4` | `main.c` | Wi-Fi 接続、SNTP 時刻同期、Azure IoT 起動、接続監視ループ |
| **Azure IoT Thread** | `NX_AZURE_IOT_THREAD_PRIORITY` | `sample_azure_iot_embedded_sdk.c` | Azure IoT SDK 内部イベント・MQTT 処理 |
| **Sample Telemetry / C2D / Direct Method / Device Twin** | `SAMPLE_THREAD_PRIORITY` | `sample_azure_iot_embedded_sdk.c` | 各 IoT Hub 機能の送受信処理 |

### 排他制御のポイント
* **`ip_ptr->nx_ip_protection`**: NetX Duo の IP インスタンス保護ミューテックスです。パケット受信通知 (`_nx_*_socket_driver_packet_receive`) の間のみ保持し、`R_WIFI_SX_ULPGN_ReceiveSocket()` の待機中には保持しない設計にして他スレッドとの競合を防ぎます。
* **`MUTEX_TX` / `MUTEX_RX` (`r_wifi_sx_ulpgn_api.c`)**:
  * SX-ULPGN モジュールへの送信系 (`MUTEX_TX`) と受信系 (`MUTEX_RX`) の排他制御を行います。
  * ソケット切り替え (`change_socket_index`) 等で両方が必要な場合は、デッドロック防止のため **必ず `MUTEX_TX` → `MUTEX_RX` の順** で取得します。

---

## 4. 起動〜接続シーケンス

1. `main()` (`src/main.c`)
   * `platform_setup()` で CMT タイマを起動し、ThreadX のティック割り込み (`_tx_timer_interrupt`) を開始します。
   * `tx_kernel_enter()` で ThreadX カーネルを起動します。
2. `tx_application_define()` (`src/main.c`)
   * パケットプール (`pool_0`: 1536 byte × 32)、IP インスタンス (`ip_0`)、TCP/UDP、TLS を初期化し、`Demo Thread` (`sample_helper_thread_entry`) を起動します。
3. `sample_helper_thread_entry()` (`src/main.c`)
   * `sample_network_configure()` (`src/sample_network.c`) を呼び出し、`R_WIFI_SX_ULPGN_Open()` → `R_WIFI_SX_ULPGN_Connect()` で Wi-Fi AP へ接続し、取得した IP/Mask/Gateway を NetX へ設定します。
   * DNS クライアント作成後、SNTP クライアントで現在時刻を同期します（TLS 証明書検証に必要）。
   * `sample_entry()` (`src/sample_azure_iot_embedded_sdk.c`) を呼び出します。
4. `sample_entry()` (`src/sample_azure_iot_embedded_sdk.c`)
   * ルート CA 証明書を登録し、Azure IoT Hub クライアントを初期化・接続します。
   * Telemetry / C2D / Direct Method / Device Twin の各スレッドを起動し、メインループで `sample_connection_monitor()` による接続監視・再接続を行います。

---

## 5. 主な設定箇所

* **Wi-Fi SSID / パスワード**: [src/sample_network.c](file:///c:/Users/airpl/Desktop/test01/.metadata/ckrx65n_wifi_azureiot/src/sample_network.c) (`WIFI_SSID`, `WIFI_PASSWORD` — WPA2 対応)
* **Azure IoT Hub / DPS 認証情報**: [src/sample_config.h](file:///c:/Users/airpl/Desktop/test01/.metadata/ckrx65n_wifi_azureiot/src/sample_config.h) (`HOST_NAME`, `DEVICE_ID`, `DEVICE_SYMMETRIC_KEY` または `ENABLE_DPS_SAMPLE`, `USE_DEVICE_CERTIFICATE`)
* **Wi-Fi UART / ピン設定**: [src/smc_gen/r_config/r_wifi_sx_ulpgn_config.h](file:///c:/Users/airpl/Desktop/test01/.metadata/ckrx65n_wifi_azureiot/src/smc_gen/r_config/r_wifi_sx_ulpgn_config.h) (`WIFI_CFG_SCI_CHANNEL`, `WIFI_CFG_SCI_BAUDRATE`, `WIFI_CFG_SCI_USE_FLOW_CONTROL` 等)

---

## 6. ビルド・開発上の注意事項（Usage Notes / Known Issues）

元のサンプルプロジェクトドキュメントに基づく注意事項です（詳細は `r01an6455ej0103-rx-azure-rtos.pdf` セクション 2.6 を参照してください）。

1. **`double` 型のサイズ**: 8 バイトを維持するため、GCC ビルドでは `-m64bit-doubles`（CC-RX では `-dbl_size=8`）が有効になっています。
2. **GCC `-Os` 最適化時のリンカオプション**: 最適化レベルを `-Os` にする場合、リンカの User defined options に `-Wl,--no-gc-sections` を追加してください。
3. **リンカスクリプトの `_end` 配置**: `src/linker_script.ld` の `.bss` セクション末尾に `_end = .;` が配置されていることを確認してください。
4. **Smart Configurator の SCI ピン設定**: 古いバージョンの e2 studio / Smart Configurator では、SCI のピン機能が `SMISOx`/`SMOSIx` になる場合があるため、`RXDx`/`TXDx` になっているか確認してください（e2 studio 2023-04 / Smart Configurator 2.17 以降で修正済み）。

---

## 7. ライセンス・セキュリティ・コントリビューション

- **ライセンス**: Azure RTOS 関連ファイルは [LICENSE.txt](LICENSE.txt) および [LICENSED-HARDWARE.txt](LICENSED-HARDWARE.txt)、Renesas FIT モジュール等は各ソースコードのヘッダ記載ライセンスに従います。
- **変更履歴**: [CHANGELOG.md](CHANGELOG.md)
- **コントリビューション**: [CONTRIBUTING.md](CONTRIBUTING.md)（認証情報をコミットしないための注意点やビルド手順を記載）
- **セキュリティ**: [SECURITY.md](SECURITY.md)

