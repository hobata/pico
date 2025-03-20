/*
 * gpio_rp2040.h
 *
 *  Created on: 2024/10/13
 *      Author: user
 */

#ifndef DEVICE_GPIO_SYSDEPEND_RP2040_GPIO_SYSDEP_H_
#define DEVICE_GPIO_SYSDEPEND_RP2040_GPIO_SYSDEP_H_

/*
 * GPIO interrupt number
 */

#define	INTNO_IRQ_BANK0	13

#define GPIO_EDGE_HIGH	8
#define GPIO_EDGE_LOW	4
#define GPIO_LEVEL_HIGH	2
#define GPIO_LEVEL_LOW	1

void gpio_set_irq_enabled(UH gpio, UW events, UB enabled);

#endif /* DEVICE_GPIO_SYSDEPEND_RP2040_GPIO_SYSDEP_H_ */
