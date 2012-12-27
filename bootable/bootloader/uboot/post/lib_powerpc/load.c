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
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

/*
 * CPU test
 * Load instructions:		lbz(x)(u), lhz(x)(u), lha(x)(u), lwz(x)(u)
 *
 * All operations are performed on a 16-byte array. The array
 * is 4-byte aligned. The base register points to offset 8.
 * The immediate offset (index register) ranges in [-8 ... +7].
 * The test cases are composed so that they do not
 * cause alignment exceptions.
 * The test contains a pre-built table describing all test cases.
 * The table entry contains:
 * the instruction opcode, the array contents, the value of the index
 * register and the expected value of the destination register.
 * After executing the instruction, the test verifies the
 * value of the destination register and the value of the base
 * register (it must change for "load with update" instructions).
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_22w (ulong *code, ulong *op1, ulong op2, ulong *op3);
extern void cpu_post_exec_21w (ulong *code, ulong *op1, ulong *op2);

static struct cpu_post_load_s
{
    ulong cmd;
    uint width;
    int update;
    int index;
    ulong offset;
} cpu_post_load_table[] =
{
    {
	OP_LWZ,
	4,
	0,
	0,
	4
    },
    {
	OP_LHA,
	3,
	0,
	0,
	2
    },
    {
	OP_LHZ,
	2,
	0,
	0,
	2
    },
    {
	OP_LBZ,
	1,
	0,
	0,
	1
    },
    {
	OP_LWZU,
	4,
	1,
	0,
	4
    },
    {
	OP_LHAU,
	3,
	1,
	0,
	2
    },
    {
	OP_LHZU,
	2,
	1,
	0,
	2
    },
    {
	OP_LBZU,
	1,
	1,
	0,
	1
    },
    {
	OP_LWZX,
	4,
	0,
	1,
	4
    },
    {
	OP_LHAX,
	3,
	0,
	1,
	2
    },
    {
	OP_LHZX,
	2,
	0,
	1,
	2
    },
    {
	OP_LBZX,
	1,
	0,
	1,
	1
    },
    {
	OP_LWZUX,
	4,
	1,
	1,
	4
    },
    {
	OP_LHAUX,
	3,
	1,
	1,
	2
    },
    {
	OP_LHZUX,
	2,
	1,
	1,
	2
    },
    {
	OP_LBZUX,
	1,
	1,
	1,
	1
    },
};
static unsigned int cpu_post_load_size =
    sizeof (cpu_post_load_table) / sizeof (struct cpu_post_load_s);

int cpu_post_test_load (void)
{
    int ret = 0;
    unsigned int i;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_load_size && ret == 0; i++)
    {
	struct cpu_post_load_s *test = cpu_post_load_table + i;
	uchar data[16] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	ulong base0 = (ulong) (data + 8);
	ulong base = base0;
	ulong value;

	if (test->index)
	{
	    ulong code[] =
	    {
		ASM_12(test->cmd, 5, 3, 4),
		ASM_BLR,
	    };

	    cpu_post_exec_22w (code, &base, test->offset, &value);
	}
	else
	{
	    ulong code[] =
	    {
		ASM_11I(test->cmd, 4, 3, test->offset),
		ASM_BLR,
	    };

	    cpu_post_exec_21w (code, &base, &value);
	}

	if (ret == 0)
	{
	   if (test->update)
	       ret = base == base0 + test->offset ? 0 : -1;
	   else
	       ret = base == base0 ? 0 : -1;
	}

	if (ret == 0)
	{
	    switch (test->width)
	    {
	    case 1:
		ret = *(uchar *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 2:
		ret = *(ushort *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 3:
		ret = *(short *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 4:
		ret = *(ulong *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    }
	}

	if (ret != 0)
	{
	    post_log ("Error at load test %d !\n", i);
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif