２．３．５章のソースの導入方法


・zipファイルを解凍
---今ここの段階
・eclipseを起動する、
・Project Explorerのpico_rp2040のプロジェクトのapp_programを右クリックして Importを選択する。
・File Systemを選択し、Next >をクリック
・Browse..から、解凍済みのgpioフォルダを選択する。
・左側枠内のgpioフォルダを選択する。
・Create top-level folderのチェックボックスにチェック
・Finishを右クリック

サンプル：app_main.c
前提条件：入力pinは、GP22

ー以上ー