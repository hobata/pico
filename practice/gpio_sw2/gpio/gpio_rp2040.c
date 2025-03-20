/*
 * gpio_rp2040.c
 *
 *  Created on: 2024/10/14
 *      Author: user
 */

#include <tk/syslib.h>
#include <tk/tkernel.h>

#include "io_bank0.h"
#include "gpio_rp2040.h"

#define CPUID			(SIO_BASE+0x0)

FP gpio_callee;

static UW get_core_num(void)
{
	return *(io_rw_32 *)CPUID;
}

static B check_gpio_param(UH gpio)
{
    if (gpio >= NUM_BANK0_GPIOS) return 1;
    return 0;
}

void gpio_set_hndler(FP* callback, FP callee)
{
	*callback = gpio_callback;
	gpio_callee = callee;
}

void gpio_callback(UINT intno)
{
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ?
                                      &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
	for (UW gpio = 0; gpio < NUM_BANK0_GPIOS; gpio+=8) {
        UW events8 = irq_ctrl_base->ints[gpio >> 3u];
        // note we assume events8 is 0 for non-existent GPIO
        for(UW i=gpio;events8 && i<gpio+8;i++) {
            UW events = events8 & 0xfu;
            if (events) {
                gpio_acknowledge_irq(i, events);
                if (gpio_callee) {
                	gpio_callee(i, events);
                }
            }
            events8 >>= 4;
        }
    }
}
void gpio_acknowledge_irq(UH gpio, UW events)
{
    if (check_gpio_param(gpio)) return; // invalid param
    io_bank0_hw->intr[gpio / 8] = events << (4 * (gpio % 8));
}
void gpio_set_irq_enabled(UH gpio, UW events, UB enabled) {
    // either this call disables the interrupt
    // or callback should already be set (raw or using gpio_set_irq_callback)
    // this protects against enabling the interrupt without callback set
#if 0
	assert(!enabled
                || (raw_irq_mask[get_core_num()] & (1u<<gpio))
                || callbacks[get_core_num()]);
#endif
    // Separate mask/force/status per-core, so check which core called, and
    // set the relevant IRQ controls.
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ?
                                      &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
    // Clear stale events which might cause immediate spurious handler entry
    gpio_acknowledge_irq(gpio, events);
    io_rw_32 *en_reg = &irq_ctrl_base->inte[gpio / 8];
    events <<= 4 * (gpio % 8);

    if (enabled)
    	set_w((_UW)en_reg, events);
    else
    	clr_w((_UW)en_reg, events);
}
