# Contributing

このリポジトリへの Issue 報告や Pull Request を歓迎します。

## 開発・ビルド環境

- **IDE**: Renesas e2 studio
- **コンパイラ**: GCC for Renesas RX (`rx-elf-gcc`)
- **ターゲットボード**: Renesas CK-RX65N + SX-ULPGN Wi-Fi モジュール

## コントリビューション手順

1. リポジトリを Fork し、作業用ブランチを作成してください（例: `fix/wifi-reconnect`）。
2. 変更を加えたあと、e2 studio または `HardwareDebug/compile_commands.json` 相当のビルド設定でコンパイルエラー・警告が出ないことを確認してください。
3. 変更内容が分かる簡潔なコミットメッセージでコミットし、Pull Request を作成してください。

## 注意事項

- **機密情報をコミットしない**: `src/sample_network.c` の `WIFI_SSID` / `WIFI_PASSWORD` や、`src/sample_config.h` の `HOST_NAME` / `DEVICE_ID` / `DEVICE_SYMMETRIC_KEY`、`src/sample_device_identity.c` の証明書・秘密鍵など、実環境の認証情報は絶対にコミットしないでください（ビルド時のプリプロセッサ定義等で上書きすることを推奨します）。
- **Smart Configurator による再生成**: `ckrx65n_wifi_azureiot.scfg` からコード再生成を行うと、`src/smc_gen/` 配下（`r_wifi_sx_ulpgn_api.c` や `r_byteq_config.h` への手動修正を含む）が上書きされる場合があります。再生成後は必ず `git diff` で差分を確認してください。
- **ライセンス**: Azure RTOS コンポーネントは [LICENSE.txt](LICENSE.txt) および [LICENSED-HARDWARE.txt](LICENSED-HARDWARE.txt)、Renesas FIT モジュールは各ファイルヘッダのライセンス条項に従います。
