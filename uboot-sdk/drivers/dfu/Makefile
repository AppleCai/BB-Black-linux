#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_USB_FUNCTION_DFU) += dfu.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_DFU_MMC) += dfu_mmc.o
endif
obj-$(CONFIG_SPL_DFU_MMC) += dfu_mmc.o
obj-$(CONFIG_DFU_NAND) += dfu_nand.o
obj-$(CONFIG_DFU_RAM) += dfu_ram.o
obj-$(CONFIG_DFU_SF) += dfu_sf.o
obj-$(CONFIG_DFU_TFTP) += dfu_tftp.o
