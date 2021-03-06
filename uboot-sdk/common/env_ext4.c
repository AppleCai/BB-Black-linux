/*
 * (c) Copyright 2016 by VRT Technology
 *
 * Author:
 *  Stuart Longland <stuartl@vrt.com.au>
 *
 * Based on FAT environment driver
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 *
 * and EXT4 filesystem implementation
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <errno.h>
#include <ext4fs.h>
#include <mmc.h>

char *env_name_spec = "EXT4";

env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

#ifdef CONFIG_SPL_BUILD
	/* intialize the MMC sub-system if env is stored on a MMC*/
	if (!strcmp(EXT4_ENV_INTERFACE, "mmc"))
		mmc_initialize(NULL);
#endif

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
	env_t	env_new;
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;

	err = env_export(&env_new);
	if (err)
		return err;

	part = blk_get_device_part_str(EXT4_ENV_INTERFACE,
					EXT4_ENV_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->devnum;
	ext4fs_set_blk_dev(dev_desc, &info);

	if (!ext4fs_mount(info.size)) {
		printf("\n** Unable to use %s %s for saveenv **\n",
		       EXT4_ENV_INTERFACE, EXT4_ENV_DEVICE_AND_PART);
		return 1;
	}

	err = ext4fs_write(EXT4_ENV_FILE, (void *)&env_new, sizeof(env_t));
	ext4fs_close();

	if (err == -1) {
		printf("\n** Unable to write \"%s\" from %s%d:%d **\n",
			EXT4_ENV_FILE, EXT4_ENV_INTERFACE, dev, part);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

void env_relocate_spec(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;
	loff_t off;

	part = blk_get_device_part_str(EXT4_ENV_INTERFACE,
					EXT4_ENV_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		goto err_env_relocate;

	dev = dev_desc->devnum;
	ext4fs_set_blk_dev(dev_desc, &info);

	if (!ext4fs_mount(info.size)) {
		printf("\n** Unable to use %s %s for loading the env **\n",
		       EXT4_ENV_INTERFACE, EXT4_ENV_DEVICE_AND_PART);
		goto err_env_relocate;
	}

	err = ext4_read_file(EXT4_ENV_FILE, buf, 0, CONFIG_ENV_SIZE, &off);
	ext4fs_close();

	if (err == -1) {
		printf("\n** Unable to read \"%s\" from %s%d:%d **\n",
			EXT4_ENV_FILE, EXT4_ENV_INTERFACE, dev, part);
		goto err_env_relocate;
	}

	env_import(buf, 1);
	return;

err_env_relocate:
	set_default_env(NULL);
}
