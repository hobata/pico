#include <tk/tkernel.h>
#include <tk/device.h>		// デバイスドライバ定義ファイル
#include <tm/tmonitor.h>
#include <bsp/libbsp.h>

#include <stdint.h>
#include "ssd1306_ini.h"
#include "font.h"

#define S_ADR	0x3c	// ① I/OデバイスのI2Cアドレス定義
#define CNTLB 0x80 // continuous bit
#define D_DATA   0x40  // DATA bit
void *memset(void *s, int c, size_t n);

static ID	dd_i2c;
static ER	err;
static SZ	asz;

void i2c_init(void)
{
	//HW　I2C0 GP8, GP9 kernel/sysdepend/pico_rp2040/hw_setting.h を参照
	dd_i2c = tk_opn_dev((UB*)"iica", TD_UPDATE);		// デバイスのオープン
	tk_slp_tsk(1); // これが無い場合：「開発環境からは正常起動するが、USB接続のみ時はI2CでSlaveからNACKが帰る」。

	UB	height=64, width = 128;
	//初期化 data sheet Figure 2 : Software Initialization Flow Chart
	//https://analogicintelligence.blogspot.com/2019/03/mycropythonoled.html
	//https://analogicintelligence.blogspot.com/2019/04/mycropythonoled.html
	UB	snd_data[] = { // 初期化送信データ
	        SET_DISP, // set  display off
	        // timing and driving scheme
	        SET_DISP_CLK_DIV,
	        0x80, // reset
	        SET_MUX_RATIO,
	        height - 1, // COM0 to 63
	        SET_DISP_OFFSET,
	        0x00, //mapping of the display start line to one of COM0~COM63
	        // resolution and layout
	        SET_DISP_START_LINE, //start line is COM0
	        // charge pump
	        SET_CHARGE_PUMP, // 0x14で有効(必須)
	        0 ? 0x10:0x14,					//external vcc
	        SET_SEG_REMAP | 0x1,        // column addr 127 mapped to SEG0
			SET_COM_OUT_DIR | 0x8,			// remapped mode. Scan from	COM[N-1] to COM0
	        SET_COM_PIN_CFG,
	        width>2*height?0x02:0x12,
	        // display
	        SET_CONTRAST,
	        0x7f,
			SET_PRECHARGE,
	        0 ? 0x22:0xF1,					//external vcc
	        SET_VCOM_DESEL,
	        0x40,                           //0x30 or 0x40?
			SET_ENTIRE_ON,                  // output follows RAM contents
	        SET_NORM_INV,                   // set normal display not inverted			NOC,
			SET_DISP | 0x01, // display on
	        // address setting
			SET_MEM_ADDR,
	        0x00,  // horizontal
	};
	UB w_data[sizeof(snd_data)*2];
    int cnt = 0;
	for (int i=0; i < sizeof(snd_data); i++){
    	w_data[cnt++] = CNTLB;
    	w_data[cnt++] = snd_data[i];
    }
	err = tk_swri_dev(dd_i2c, S_ADR, w_data, sizeof(w_data), &asz);
}

void clear(UB ptn)
{
	//画面 data
	UB d_data[64+1];//display data　フルサイズ：128*8まで繰り返し
    memset(d_data, ptn, sizeof(d_data));
    d_data[0] = D_DATA;
    for (int i=0; i<16; i++) {
    	err = tk_swri_dev(dd_i2c, S_ADR, d_data, sizeof(d_data), &asz);
    }
}
void area(UB x1, UB x2, UB y1, UB y2)
{
    //area setting
    UB area[] = {
    	    SET_COL_ADDR,
			x1,
			x2,
    	    SET_PAGE_ADDR,
			y1,
			y2
    };
	UB w_data2[sizeof(area)*2];
    int cnt = 0;
	for (int i=0; i < sizeof(area); i++){
    	w_data2[cnt++] = CNTLB;
    	w_data2[cnt++] = area[i];
    }
	err = tk_swri_dev(dd_i2c, S_ADR, w_data2, sizeof(w_data2), &asz);
}
void ole_prt(char* c, UB len)
{
	UB d_data[96+1]; // less than 100(DEVCNF_I2C_MAX_SDATSZ)
    for (int k=0; k < len /16 +1; k++) {
       memset(d_data, 0x00, sizeof(d_data));
       int cnt = 0;
       d_data[cnt++] = 0x40;
    	for (int i=0; i < 16; i++){
    		if (k*16+i > len-1) break;
    		int col = c[i+k*16] - font_prm[3];
    		if (col < 0) col = 0;
    		for (int j=0; j<5; j++){
    			d_data[cnt++] = font_8x5[col*5+j];
    		}
    		d_data[cnt++] = 0x0; //6列目 文字の間
    	}
    	err = tk_swri_dev(dd_i2c, S_ADR, d_data, sizeof(d_data), &asz);
    }
}
/* usermain関数 */
EXPORT INT usermain(void)
{
	i2c_init();
	clear(0x0);
	area(1, 126, 1, 6); // 128 % 6 = 2 両側１ドットずつをエリア外とする。
	static char c[95]; // font_prm[4]-font_prm[3]+1
	for (int i=0; i < sizeof(c); i++ ) {
		c[i] = i + font_prm[3];
	}
	ole_prt(c, sizeof(c));

	tk_slp_tsk(TMO_FEVR);

	return 0;				// ここは実行されない
}
