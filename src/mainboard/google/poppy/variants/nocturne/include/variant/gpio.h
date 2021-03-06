/*
 * This file is part of the coreboot project.
 *
 * Copyright 2018 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#ifndef __BASEBOARD_GPIO_H__
#define __BASEBOARD_GPIO_H__

#include <soc/gpe.h>
#include <soc/gpio.h>

/* EC in RW */
#define GPIO_EC_IN_RW		GPP_C6

/* BIOS Flash Write Protect */
#define GPIO_PCH_WP		GPP_C23

/* Memory configuration board straps */
#define GPIO_MEM_CONFIG_0	GPP_C12
#define GPIO_MEM_CONFIG_1	GPP_C13
#define GPIO_MEM_CONFIG_2	GPP_C14
#define GPIO_MEM_CONFIG_3	GPP_C15

/* EC wake is LAN_WAKE# which is a special DeepSX wake pin */
#define GPE_EC_WAKE		GPE0_LAN_WAK

/* EC sync irq is tied to GPP_D17 */
#define EC_SYNC_IRQ		GPP_D17_IRQ

/* EC sync gpio */
#define EC_SYNC_GPIO		GPP_D17

/* eSPI virtual wire reporting */
#define EC_SCI_GPI		GPE0_ESPI

/* Camera Sensor config */
#define GPIO_RCAM_PWR_EN	GPP_D7
#define GPIO_PCH_RCAM_CLK_EN	GPP_D14
#define GPIO_RCAM_RST_L		GPP_D16
#define GPIO_FCAM_PWR_EN	GPP_B4
#define GPIO_PCH_FCAM_CLK_EN	GPP_D13
#define GPIO_FCAM_RST_L		GPP_D15

#endif /* BASEBOARD_GPIO_H */
