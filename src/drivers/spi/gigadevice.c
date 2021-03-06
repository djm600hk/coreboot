/*
 * This file is part of the coreboot project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <commonlib/helpers.h>
#include <spi_flash.h>
#include <spi-generic.h>
#include <string.h>

#include "spi_flash_internal.h"

/* GD25Pxx-specific commands */
#define CMD_GD25_WREN		0x06	/* Write Enable */
#define CMD_GD25_WRDI		0x04	/* Write Disable */
#define CMD_GD25_RDSR		0x05	/* Read Status Register */
#define CMD_GD25_WRSR		0x01	/* Write Status Register */
#define CMD_GD25_READ		0x03	/* Read Data Bytes */
#define CMD_GD25_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_GD25_PP		0x02	/* Page Program */
#define CMD_GD25_SE		0x20	/* Sector (4K) Erase */
#define CMD_GD25_BE		0xd8	/* Block (64K) Erase */
#define CMD_GD25_CE		0xc7	/* Chip Erase */
#define CMD_GD25_DP		0xb9	/* Deep Power-down */
#define CMD_GD25_RES		0xab	/* Release from DP, and Read Signature */

struct gigadevice_spi_flash_params {
	uint16_t	id;
	uint8_t		dual_spi : 1;
	uint8_t		_reserved_for_flags : 3;
	uint8_t		l2_page_size_shift : 4;
	uint8_t		pages_per_sector_shift : 4;
	uint8_t		sectors_per_block_shift : 4;
	uint8_t		nr_blocks_shift;
	const char	name[10];
};

static const struct gigadevice_spi_flash_params gigadevice_spi_flash_table[] = {
	{
		.id				= 0x3114,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 4,
		.name				= "GD25T80",
	},
	{
		.id				= 0x4014,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 4,
		.dual_spi			= 1,
		.name				= "GD25Q80",
	},					/* also GD25Q80B */
	{
		.id				= 0x4015,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 5,
		.dual_spi			= 1,
		.name				= "GD25Q16",
	},					/* also GD25Q16B */
	{
		.id				= 0x4016,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 6,
		.dual_spi			= 1,
		.name				= "GD25Q32B",
	},					/* also GD25Q32B */
	{
		.id				= 0x4017,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 7,
		.dual_spi			= 1,
		.name				= "GD25Q64",
	},					/* also GD25Q64B, GD25B64C */
	{
		.id				= 0x4018,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 8,
		.dual_spi			= 1,
		.name				= "GD25Q128",
	},					/* also GD25Q128B */
	{
		.id				= 0x4214,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 4,
		.dual_spi			= 1,
		.name				= "GD25VQ80C",
	},
	{
		.id				= 0x4215,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 5,
		.dual_spi			= 1,
		.name				= "GD25VQ16C",
	},
	{
		.id				= 0x6014,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 4,
		.dual_spi			= 1,
		.name				= "GD25LQ80",
	},
	{
		.id				= 0x6015,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 5,
		.dual_spi			= 1,
		.name				= "GD25LQ16",
	},
	{
		.id				= 0x6016,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 6,
		.dual_spi			= 1,
		.name				= "GD25LQ32",
	},
	{
		.id				= 0x6017,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 7,
		.dual_spi			= 1,
		.name				= "GD25LQ64C",
	},					/* also GD25LB64C */
	{
		.id				= 0x6018,
		.l2_page_size_shift		= 8,
		.pages_per_sector_shift		= 4,
		.sectors_per_block_shift	= 4,
		.nr_blocks_shift		= 8,
		.dual_spi			= 1,
		.name				= "GD25LQ128",
	},
};

static const struct spi_flash_ops spi_flash_ops = {
	.read = spi_flash_cmd_read,
	.write = spi_flash_cmd_write_page_program,
	.erase = spi_flash_cmd_erase,
	.status = spi_flash_cmd_status,
};

int spi_flash_probe_gigadevice(const struct spi_slave *spi, u8 *idcode,
				struct spi_flash *flash)
{
	const struct gigadevice_spi_flash_params *params;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(gigadevice_spi_flash_table); i++) {
		params = &gigadevice_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(gigadevice_spi_flash_table)) {
		printk(BIOS_WARNING,
		       "SF gigadevice.c: Unsupported ID %#02x%02x\n",
		       idcode[1], idcode[2]);
		return -1;
	}

	memcpy(&flash->spi, spi, sizeof(*spi));
	flash->name = params->name;

	/* Assuming power-of-two page size initially. */
	flash->page_size = 1 << params->l2_page_size_shift;
	flash->sector_size = flash->page_size *
			     (1 << params->pages_per_sector_shift);
	flash->size = flash->sector_size *
		      (1 << params->sectors_per_block_shift) *
		      (1 << params->nr_blocks_shift);
	flash->erase_cmd = CMD_GD25_SE;
	flash->status_cmd = CMD_GD25_RDSR;
	flash->pp_cmd = CMD_GD25_PP;
	flash->wren_cmd = CMD_GD25_WREN;

	flash->ops = &spi_flash_ops;

	return 0;
}
