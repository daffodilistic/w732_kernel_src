/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 *
 * Base on code from TI. Original Notices follow:
 *
 * (C) Copyright 2008, Texas Instruments, Inc. http://www.ti.com/
 *
 * Modified for DA8xx EVM.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts are shamelessly stolen from various TI sources, original copyright
 * follows:
 * -----------------------------------------------------------------
 *
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include "../common/misc.h"

DECLARE_GLOBAL_DATA_PTR;

#define pinmux	&davinci_syscfg_regs->pinmux

/* SPI0 pin muxer settings */
static const struct pinmux_config spi0_pins[] = {
	{ pinmux[7], 1, 3 },
	{ pinmux[7], 1, 4 },
	{ pinmux[7], 1, 5 },
	{ pinmux[7], 1, 6 },
	{ pinmux[7], 1, 7 }
};

/* EMIF-A bus pins for 8-bit NAND support on CS3 */
static const struct pinmux_config emifa_nand_pins[] = {
	{ pinmux[13], 1, 6 },
	{ pinmux[13], 1, 7 },
	{ pinmux[14], 1, 0 },
	{ pinmux[14], 1, 1 },
	{ pinmux[14], 1, 2 },
	{ pinmux[14], 1, 3 },
	{ pinmux[14], 1, 4 },
	{ pinmux[14], 1, 5 },
	{ pinmux[15], 1, 7 },
	{ pinmux[16], 1, 0 },
	{ pinmux[18], 1, 1 },
	{ pinmux[18], 1, 4 },
	{ pinmux[18], 1, 5 },
};

/* EMAC PHY interface pins */
static const struct pinmux_config emac_pins[] = {
	{ pinmux[9], 0, 5 },
	{ pinmux[10], 2, 1 },
	{ pinmux[10], 2, 2 },
	{ pinmux[10], 2, 3 },
	{ pinmux[10], 2, 4 },
	{ pinmux[10], 2, 5 },
	{ pinmux[10], 2, 6 },
	{ pinmux[10], 2, 7 },
	{ pinmux[11], 2, 0 },
	{ pinmux[11], 2, 1 },
};

/* UART pin muxer settings */
static const struct pinmux_config uart_pins[] = {
	{ pinmux[8], 2, 7 },
	{ pinmux[9], 2, 0 }
};

/* I2C pin muxer settings */
static const struct pinmux_config i2c_pins[] = {
	{ pinmux[8], 2, 3 },
	{ pinmux[8], 2, 4 }
};

/* USB0_DRVVBUS pin muxer settings */
static const struct pinmux_config usb_pins[] = {
	{ pinmux[9], 1, 1 }
};

static const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi0_pins),
#endif
	PINMUX_ITEM(uart_pins),
	PINMUX_ITEM(i2c_pins),
#ifdef CONFIG_USB_DA8XX
	PINMUX_ITEM(usb_pins),
#endif
#ifdef CONFIG_USE_NAND
	PINMUX_ITEM(emifa_nand_pins),
#endif
#if defined(CONFIG_DRIVER_TI_EMAC)
	PINMUX_ITEM(emac_pins),
#endif
};

int board_init(void)
{
#ifndef CONFIG_USE_IRQ
	/*
	 * Mask all IRQs by clearing the global enable and setting
	 * the enable clear for all the 90 interrupts.
	 */

	writel(0, &davinci_aintc_regs->ger);

	writel(0, &davinci_aintc_regs->hier);

	writel(0xffffffff, &davinci_aintc_regs->ecr1);
	writel(0xffffffff, &davinci_aintc_regs->ecr2);
	writel(0xffffffff, &davinci_aintc_regs->ecr3);
#endif

#ifdef CONFIG_NAND_DAVINCI
	/* EMIFA 100MHz clock select */
	writel(readl(&davinci_syscfg_regs->cfgchip3) & ~2,
	       &davinci_syscfg_regs->cfgchip3);
	/* NAND CS setup */
	writel((DAVINCI_ABCR_WSETUP(0) |
		DAVINCI_ABCR_WSTROBE(2) |
		DAVINCI_ABCR_WHOLD(0) |
		DAVINCI_ABCR_RSETUP(0) |
		DAVINCI_ABCR_RSTROBE(2) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(2) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab2cr);
#endif

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA830_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	lpsc_on(DAVINCI_LPSC_AEMIF);    /* NAND, NOR */
	lpsc_on(DAVINCI_LPSC_SPI0);     /* Serial Flash */
	lpsc_on(DAVINCI_LPSC_EMAC);     /* image download */
	lpsc_on(DAVINCI_LPSC_UART2);    /* console */
	lpsc_on(DAVINCI_LPSC_GPIO);

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI0 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	return(0);
}

#if defined(CONFIG_DRIVER_TI_EMAC)

#define PHY_SW_I2C_ADDR	0x5f /* Address of PHY on i2c bus */

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	u_int8_t mac_addr[6];
	u_int8_t switch_start_cmd[2] = { 0x01, 0x23 };

	/* Read Ethernet MAC address from EEPROM */
	if (dvevm_read_mac_address(mac_addr))
		/* set address env if not already set */
		dv_configure_mac_address(mac_addr);

	/* read the address back from env */
	if (!eth_getenv_enetaddr("ethaddr", mac_addr))
		return -1;

	/* provide the resulting addr to the driver */
	davinci_eth_set_mac_addr(mac_addr);

	/* enable the Ethernet switch in the 3 port PHY */
	if (i2c_write(PHY_SW_I2C_ADDR, 0, 0,
			switch_start_cmd, sizeof(switch_start_cmd))) {
		printf("Ethernet switch start failed!\n");
		return -1;
	}

	/* finally, initialise the driver */
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
