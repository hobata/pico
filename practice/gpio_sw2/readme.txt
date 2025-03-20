スイッチ2個以上に対応した割り込み


・zipファイルを解凍
---今ここの段階
・eclipseを起動する、
・Project Explorerのpico_rp2040のプロジェクトのapp_programを右クリックして Importを選択する。
・File Systemを選択し、Next >をクリック
・Browse..から、解凍済みのgpioフォルダを選択する。
・左側枠内のgpioフォルダを選択する。
・Create top-level folderのチェックボックスにチェック
・Finishを右クリック

サンプル：添付のapp_main.c
このサンプルでは、picoの入力pinは、GP21, GP22に設定されている。

(プルアップ)抵抗 10kオーム or 4.7kオーム

以下のように接続すること。

3V3 --- 抵抗 ---+--- スイッチ --- GND
                |
               GP21

3V3 --- 抵抗 ---+--- スイッチ --- GND
                |
               GP22

ー以上ー