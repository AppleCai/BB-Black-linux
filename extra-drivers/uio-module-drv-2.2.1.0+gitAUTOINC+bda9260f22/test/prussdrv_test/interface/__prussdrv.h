/*
 * __prussdrv.h
 * 
 *
 * Copyright (C) 2012-2016 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/unistd.h>

#define DISABLE_L3RAM_SUPPORT

#define	PAGE_SIZE                     4096

#define PRUSS_V1                    1
#define PRUSS_V2                    2

#define AM57XX_AM33XX_PRUSS_INTC_REV         0x4E82A900
#define AM18XX_PRUSS_INTC_REV         0x4E825900

#define PRUSS_MAX_IRAM_SIZE                  8192

#define MAX_DEVICE_NAME_LENGTH 16

//PRUSS INTC register offsets
#define PRU_INTC_REVID_REG   0x000
#define PRU_INTC_CR_REG      0x004
#define PRU_INTC_HCR_REG     0x00C
#define PRU_INTC_GER_REG     0x010
#define PRU_INTC_GNLR_REG    0x01C
#define PRU_INTC_SISR_REG    0x020
#define PRU_INTC_SICR_REG    0x024
#define PRU_INTC_EISR_REG    0x028
#define PRU_INTC_EICR_REG    0x02C
#define PRU_INTC_HIEISR_REG  0x034
#define PRU_INTC_HIDISR_REG  0x038
#define PRU_INTC_GPIR_REG    0x080

#define PRU_INTC_SRSR1_REG   0x200
#define PRU_INTC_SRSR2_REG   0x204

#define PRU_INTC_SECR1_REG   0x280
#define PRU_INTC_SECR2_REG   0x284

#define PRU_INTC_ESR1_REG    0x300
#define PRU_INTC_ESR2_REG    0x304

#define PRU_INTC_ECR1_REG    0x380
#define PRU_INTC_ECR2_REG    0x384

#define PRU_INTC_CMR1_REG    0x400
#define PRU_INTC_CMR2_REG    0x404
#define PRU_INTC_CMR3_REG    0x408
#define PRU_INTC_CMR4_REG    0x40C
#define PRU_INTC_CMR5_REG    0x410
#define PRU_INTC_CMR6_REG    0x414
#define PRU_INTC_CMR7_REG    0x418
#define PRU_INTC_CMR8_REG    0x41C
#define PRU_INTC_CMR9_REG    0x420
#define PRU_INTC_CMR10_REG   0x424
#define PRU_INTC_CMR11_REG   0x428
#define PRU_INTC_CMR12_REG   0x42C
#define PRU_INTC_CMR13_REG   0x430
#define PRU_INTC_CMR14_REG   0x434
#define PRU_INTC_CMR15_REG   0x438
#define PRU_INTC_CMR16_REG   0x43C

#define PRU_INTC_HMR1_REG    0x800
#define PRU_INTC_HMR2_REG    0x804
#define PRU_INTC_HMR3_REG    0x808

#define PRU_INTC_SIPR1_REG   0xD00
#define PRU_INTC_SIPR2_REG   0xD04

#define PRU_INTC_SITR1_REG   0xD80
#define PRU_INTC_SITR2_REG   0xD84

#define PRU_INTC_HIER_REG    0x1500

#define MAX_HOSTS_SUPPORTED	10

typedef struct __prussdrv {
    int version;
    char device_name[MAX_DEVICE_NAME_LENGTH];
    int fd[NUM_PRU_HOSTIRQS];
    void *pru0_dataram_base;
    void *pru1_dataram_base;
    void *intc_base;
    void *pru0_control_base;
    void *pru0_debug_base;
    void *pru1_control_base;
    void *pru1_debug_base;
    void *pru0_iram_base;
    void *pru1_iram_base;
    void *l3ram_base;
    void *extram_base;
    pthread_t irq_thread[NUM_PRU_HOSTIRQS];
    int mmap_fd;
    int mmap_fd2;
    int pru0_mmap_fd;
    int pru1_mmap_fd;
    void *pruss_sharedram_base;
    void *pruss_cfg_base;
    void *pruss_uart_base;
    void *pruss_iep_base;
    void *pruss_ecap_base;
    void *pruss_miirt_base;
    void *pruss_mdio_base;
    unsigned int pru0_dataram_phy_base;
    unsigned int pru0_dataram_size;
    unsigned int pru1_dataram_phy_base;
    unsigned int pru1_dataram_size;
    unsigned int intc_phy_base;
    unsigned int intc_size;
    unsigned int pru0_control_phy_base;
    unsigned int pru0_control_size;
    unsigned int pru0_debug_phy_base;
    unsigned int pru0_debug_size;
    unsigned int pru1_control_phy_base;
    unsigned int pru1_control_size;
    unsigned int pru1_debug_phy_base;
    unsigned int pru1_debug_size;
    unsigned int pru0_iram_phy_base;
    unsigned int pru0_iram_size;
    unsigned int pru1_iram_phy_base;
    unsigned int pru1_iram_size;
    unsigned int pruss_sharedram_phy_base;
    unsigned int pruss_sharedram_size;
    unsigned int pruss_cfg_phy_base;
    unsigned int pruss_cfg_size;
    unsigned int pruss_uart_phy_base;
    unsigned int pruss_uart_size;
    unsigned int pruss_iep_phy_base;
    unsigned int pruss_iep_size;
    unsigned int pruss_ecap_phy_base;
    unsigned int pruss_ecap_size;
    unsigned int pruss_miirt_phy_base;
    unsigned int pruss_miirt_size;
    unsigned int pruss_mdio_phy_base;
    unsigned int pruss_mdio_size;
#ifdef DISABLE_L3RAM_SUPPORT
    unsigned int l3ram_phys_base;
    unsigned int l3ram_map_size;
#endif
#ifdef DISABLE_EXTRAM_SUPPORT
    unsigned int extram_phys_base;
    unsigned int extram_map_size;
#endif
} tprussdrv;


int __pruss_detect_hw_version(unsigned int *pruss_io)
{
    switch (pruss_io[0]) {
    case AM18XX_PRUSS_INTC_REV:
        return PRUSS_V1;
    break;
    case AM57XX_AM33XX_PRUSS_INTC_REV:
        return PRUSS_V2;
    break;
    default:
        return -1;
    }
}

void __prussintc_set_cmr(volatile unsigned int *pruintc_io, unsigned short sysevt,
                         unsigned short channel)
{
    pruintc_io[(PRU_INTC_CMR1_REG + (sysevt & ~(0x3))) >> 2] |=
        ((channel & 0xF) << ((sysevt & 0x3) << 3));

}

void __prussintc_set_hmr(volatile unsigned int *pruintc_io, unsigned short channel,
                         unsigned short host)
{
    pruintc_io[(PRU_INTC_HMR1_REG + (channel & ~(0x3))) >> 2] =
        pruintc_io[(PRU_INTC_HMR1_REG +
                    (channel & ~(0x3))) >> 2] | (((host) & 0xF) <<
                                                 (((channel) & 0x3) << 3));
}
