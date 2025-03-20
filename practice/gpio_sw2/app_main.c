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

//#include <sys/queue.h>
//#include <../kernel/tkernel/timer.h>
#include "gpio/gpio_rp2040.h"

/////////////////////////
/* GPIO interrupt task */
/////////////////////////

/* ① 割込み番号と割込み優先度の定義 */
#define	INTLV_GPIO		6		// 割込みレベル

#define GP_SW			21
#define GP_SW2			22

/* ② 割込みハンドラ定義情報 */
LOCAL T_DINT dint_gpio = {
	.intatr		= TA_HLNG,		// 割込みハンドラ属性
	.inthdr		= NULL,		// 割込みハンドラアドレス
};

/* LED制御タスクの生成情報と関連データ */
LOCAL void task_sw1(INT stacd, void *exinf);	// 実行関数
LOCAL ID	tskid_sw1;			// ID番号
LOCAL T_CTSK ctsk_sw1 = {
	.itskpri	= 10,			// 初期優先度
	.stksz		= 1024,			// スタックサイズ
	.task		= task_sw1,		// 実行関数のポインタ
	.tskatr		= TA_HLNG | TA_RNG3,	// タスク属性
};
LOCAL void task_sw2(INT stacd, void *exinf);	// 実行関数
LOCAL ID	tskid_sw2;			// ID番号
LOCAL T_CTSK ctsk_sw2 = {
	.itskpri	= 10,			// 初期優先度
	.stksz		= 1024,			// スタックサイズ
	.task		= task_sw2,		// 実行関数のポインタ
	.tskatr		= TA_HLNG | TA_RNG3,	// タスク属性
};

/* ③ 割込みのcallback */
LOCAL void gpio_cb(UINT gpio, UW events)
{
	switch(gpio) {
	case GP_SW:
		tk_wup_tsk(tskid_sw1);			// LED制御タスクを起床
		break;
	case GP_SW2:
		tk_wup_tsk(tskid_sw2);			// LED制御タスクを起床
		break;
	default:
		break;
	}
	tm_printf((UB*)"int: gpio:%d, events:%x\n", gpio, events);
}

/* ④ SW制御タスクの実行関数 */
LOCAL void task_sw2(INT stacd, void *exinf)
{
	while(1) {
		tk_slp_tsk(TMO_FEVR);
		tm_printf((UB*)"wake up sw2 task\n");
	}
	tk_ext_tsk();				// ここは実行されない
}
LOCAL void task_sw1(INT stacd, void *exinf)
{
	while(1) {
		tk_slp_tsk(TMO_FEVR);
		tm_printf((UB*)"wake up sw1 task\n");
	}
	tk_ext_tsk();				// ここは実行されない
}

EXPORT INT usermain(void)
{
	tm_printf((UB*)"User program started\n");

	//GPIO設定
	gpio_set_pin(GP_SW, GPIO_MODE_IN);
	gpio_set_pin(GP_SW2, GPIO_MODE_IN);

	/* ⑤ 割込みの設定と許可 */
	gpio_set_hndler(&dint_gpio.inthdr, gpio_cb); // 割り込み関数と、callbackの登録
	tk_def_int( INTNO_IRQ_BANK0, &dint_gpio);	// 割込みハンドラの定義
	gpio_set_irq_enabled(GP_SW, GPIO_EDGE_LOW, 1);	//割り込み条件の設定
	gpio_set_irq_enabled(GP_SW2, GPIO_EDGE_LOW, 1);	//割り込み条件の設定
	EnableInt( INTNO_IRQ_BANK0, INTLV_GPIO);	// 割込み許可 (NVIC)

	tskid_sw1 = tk_cre_tsk(&ctsk_sw1);	// タスクの生成
	tskid_sw2 = tk_cre_tsk(&ctsk_sw2);

	tk_sta_tsk(tskid_sw1, 0);		// タスクの実行
	tk_sta_tsk(tskid_sw2, 0);

	tk_slp_tsk(TMO_FEVR);			// 起床待ち

	return 0;				// ここは実行されない
}
