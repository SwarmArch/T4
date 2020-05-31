/** $lic$
 * Copyright (C) 2014-2020 by Massachusetts Institute of Technology
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 * This software was developed and released to accompany the T4 ISCA 2020 paper
 * ("T4: Compiling Sequential Code for Effective Speculative Parallelization in
 * Hardware", Ying et al., ISCA-47, June 2020).  If you use this software in
 * your research, we request that you reference the T4 paper, and we ask that
 * you send us a citation of your work.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FPUTS_FGETS_AUTO_H__
#define __FPUTS_FGETS_AUTO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void read_and_encode(const char* fname, char* buffer, int count);

#ifdef __cplusplus
}
#endif

#endif //__FPUTS_FGETS_AUTO_H__
