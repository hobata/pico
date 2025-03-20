/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.0 BSP
 *
 *    Copyright (C) 2022-2023 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2023/05.
 *
 *----------------------------------------------------------------------
 */

/*
 *	app_main.c
 *	Application main program for RaspberryPi Pico
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <bsp/libbsp.h>

#include <sys/queue.h>
#include <../kernel/tkernel/timer.h>
#include "gpio/gpio_rp2040.h"

/////////////////////////
/* GPIO interrupt task */
/////////////////////////

/* ① 割込み番号と割込み優先度の定義 */
#define	INTLV_GPIO		6		// 割込みレベル

#define GP_LED			25
#define GP_SW			22
#define FIRST_LED		1		// LED初期値
#define SW_CHA			20		// チャタリング待ち時間(ms)

/* ② 割込みハンドラ定義情報 */
LOCAL void inthdr_sw(UINT intno);
LOCAL T_DINT dint_sw = {
	.intatr		= TA_HLNG,		// 割込みハンドラ属性
	.inthdr		= inthdr_sw,		// 割込みハンドラアドレス
};

/* LED制御タスクの生成情報と関連データ */
LOCAL void task_led(INT stacd, void *exinf);	// 実行関数
LOCAL ID	tskid_led;			// ID番号
LOCAL T_CTSK ctsk_led = {
	.itskpri	= 10,			// 初期優先度
	.stksz		= 1024,			// スタックサイズ
	.task		= task_led,		// 実行関数のポインタ
	.tskatr		= TA_HLNG | TA_RNG3,	// タスク属性
};

/* ③ 割込みハンドラ */
LOCAL void inthdr_sw(UINT intno)
{
	tk_wup_tsk(tskid_led);			// LED制御タスクを起床
	gpio_set_irq_enabled(22, GPIO_EDGE_LOW, 0);	//割り込み条件のクリア
	ClearInt(intno);			// 割込み発生のクリア(NVIC)
	gpio_set_irq_enabled(22, GPIO_EDGE_LOW, 1);	//割り込み条件の設定
}

LOCAL B chkTMO(SYSTIM *pre, SYSTIM *now, int ms)
{
	LSYSTIM p = knl_toLSYSTIM( pre );
	LSYSTIM n = knl_toLSYSTIM( now );
    if (n - p > ms) {
    	return 1; //TMO
    }
    return 0;//TMO未満
}
/* ④ LED制御タスクの実行関数 */
LOCAL UINT val = FIRST_LED;
LOCAL SYSTIM pre = {0, 0};
LOCAL void task_led(INT stacd, void *exinf)
{
	SYSTIM now;
	while(1) {
		tk_slp_tsk(TMO_FEVR);
		tk_get_otm(&now);
		if (chkTMO(&pre, &now, SW_CHA)) {
			val ^= 1;
			gpio_set_val(GP_LED, val & 1);
			pre = now;
		}
	}
	tk_ext_tsk();				// ここは実行されない
}

EXPORT INT usermain(void)
{
	//GPIO設定
	gpio_set_pin(GP_SW, GPIO_MODE_IN);
	gpio_set_pin(GP_LED, GPIO_MODE_OUT);
	gpio_set_val(GP_LED, FIRST_LED);	// LED on

	/* ⑤ 割込みの設定と許可 */
	tk_def_int( INTNO_IRQ_BANK0, &dint_sw);	// ⑤-1 割込みハンドラの定義
	gpio_set_irq_enabled(GP_SW, GPIO_EDGE_LOW, 1);	//割り込み条件の設定
	EnableInt( INTNO_IRQ_BANK0, INTLV_GPIO);	// ⑤-5 割込み許可 (NVIC)

	tskid_led = tk_cre_tsk(&ctsk_led);	// タスクの生成

	tk_sta_tsk(tskid_led, 0);		// タスクの実行

	tk_slp_tsk(TMO_FEVR);			// 起床待ち

	return 0;				// ここは実行されない
}
