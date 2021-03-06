/*
 * This file is part of the coreboot project.
 *
 * Copyright 2019 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <delay.h>
#include <drivers/analogix/anx7625/anx7625.h>
#include <edid.h>
#include <gpio.h>
#include <soc/i2c.h>
#include <string.h>

#include "panel.h"


static void power_on_anx7625(void)
{
	gpio_output(GPIO_MIPIBRDG_RST_L_1V8, 0);
	gpio_output(GPIO_PP1200_MIPIBRDG_EN, 1);
	gpio_output(GPIO_VDDIO_MIPIBRDG_EN, 1);
	gpio_output(GPIO_PP1800_LCM_EN, 1);
	mdelay(2);
	gpio_output(GPIO_MIPIBRDG_PWRDN_L_1V8, 1);
	mdelay(10);
	gpio_output(GPIO_MIPIBRDG_RST_L_1V8, 1);
	gpio_output(GPIO_PP3300_LCM_EN, 1);
}

static void dummy_power_on(void)
{
	/* The panel has been already powered on when getting panel information
	 * so we should do nothing here.
	 */
}

static struct panel_serializable_data anx7625_data = {
	.orientation = LB_FB_ORIENTATION_NORMAL,
	.init = { INIT_END_CMD },
};

static struct panel_description anx7625_panel = {
	.s = &anx7625_data,
	.power_on = dummy_power_on,
};

struct panel_description *get_panel_description(int panel_id)
{
	/* To read panel EDID, we have to first power on anx7625. */
	power_on_anx7625();

	u8 i2c_bus = 4;
	mtk_i2c_bus_init(i2c_bus);

	if (anx7625_init(i2c_bus)) {
		printk(BIOS_ERR, "Can't init ANX7625 bridge.\n");
		return NULL;
	}
	struct edid *edid = &anx7625_data.edid;
	if (anx7625_dp_get_edid(i2c_bus, edid)) {
		printk(BIOS_ERR, "Can't get panel's edid.\n");
		return NULL;
	}
	if (anx7625_dp_start(i2c_bus, edid) < 0) {
		printk(BIOS_ERR, "Can't start display via ANX7625.\n");
		return NULL;
	}
	return &anx7625_panel;
}
