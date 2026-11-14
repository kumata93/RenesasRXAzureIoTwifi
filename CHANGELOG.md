# Changelog

## [Unreleased] / 安定化修正

### Fixed
- **Wi-Fi ドライバ (`r_wifi_sx_ulpgn_api.c`)**:
  - `R_WIFI_SX_ULPGN_SendSocket()` と `R_WIFI_SX_ULPGN_ReceiveSocket()` 間の `MUTEX_TX` / `MUTEX_RX` 取得順序の不統一による AB-BA デッドロックを解消。
  - `mutex_take()` およびソケット切替エラーパスでのミューテックスリークを修正。
  - `R_WIFI_SX_ULPGN_SendSocket()` 内の `tx_end_flag` 待機ループにタイムアウト判定を追加し、無限ループを防止。
- **NetX Duo ポート層 (`nx_driver_rx_wifi.c`)**:
  - TCP 接続失敗時のドライバソケットスロットおよびハードウェアソケットのリークを修正。
  - `NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND` におけるソケットインデックス未取得バグおよびハードウェアソケットリークを修正。
  - タイムアウト変換の整数除算による `0` ms 化を修正。
  - `_nx_driver_thread_entry()` で `R_WIFI_SX_ULPGN_ReceiveSocket()` 待機中に `ip_ptr->nx_ip_protection` を保持し続けないよう改善。
  - `_nx_driver_hardware_initialize()` で IP スレッドの優先度を正しく取得するよう修正。
- **割り込み保護 (`r_byteq_config.h`)**:
  - `BYTEQ_CFG_PROTECT_QUEUE` および `BYTEQ_CFG_CRITICAL_SECTION` を `1` に変更。
- **アプリケーション / 互換層 (`sample_network.c`, `main.c`, `sample_azure_iot_embedded_sdk.c`, `demo_printf.c`, `compat.c`)**:
  - `sample_network_configure()` の戻り値欠落・エラー返却値および `main.c` 側の戻り値チェックを修正。
  - Azure IoT 送受信エラー時の `sample_connection_status` 更新とビジーループ防止を追加し、再接続モニタが機能するように修正。
  - `demo_printf()` の `vsnprintf()` 戻り値クランプおよび `R_SCI_Send()` 失敗時のデッドロックを修正。
  - `compat.c` の `az_platform_clock_msec()` / `az_platform_sleep_msec()` を ThreadX API で実装。

### Documentation
- `README.md` に全体構成・アーキテクチャ・スレッド構成・排他制御の説明を追加。
- `CONTRIBUTING.md`, `CHANGELOG.md` を追加。

## Initial Commit
- Renesas CK-RX65N + Azure RTOS + SX-ULPGN Wi-Fi Azure IoT サンプルプロジェクトの初期コードを取り込み。
