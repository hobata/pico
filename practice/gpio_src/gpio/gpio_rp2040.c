/*
 * gpio_rp2040.c
 *
 *  Created on: 2024/10/14
 *      Author: user
 */

#include <tk/syslib.h>
#include "io_bank0.h"

#define CPUID			(SIO_BASE+0x0)

static UW get_core_num(void)
{
	return *(io_rw_32 *)CPUID;
}

static B check_gpio_param(UH gpio) {
    if (gpio >= NUM_BANK0_GPIOS) return 1;
    return 0;
}

void gpio_acknowledge_irq(UH gpio, UW events) {
    if (check_gpio_param(gpio)) return; // invalid param
    io_bank0_hw->intr[gpio / 8] = events << (4 * (gpio % 8));
}

static void _gpio_set_irq_enabled(UH gpio, UW events, UB enabled, io_bank0_irq_ctrl_hw_t *irq_ctrl_base) {
	// Clear stale events which might cause immediate spurious handler entry
    gpio_acknowledge_irq(gpio, events);
    io_rw_32 *en_reg = &irq_ctrl_base->inte[gpio / 8];
    events <<= 4 * (gpio % 8);

    if (enabled)
        //hw_set_bits(en_reg, events);
    	set_w((_UW)en_reg, events);
    else
        //hw_clear_bits(en_reg, events);
    	clr_w((_UW)en_reg, events);
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
    _gpio_set_irq_enabled(gpio, events, enabled, irq_ctrl_base);
}
