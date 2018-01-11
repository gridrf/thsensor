/* mbed LED Font Library, for TM1637 LED controller
 * Copyright (c) 2016, v01: WH, Initial version, Test in CATALEX
 *               2017, v02: WH, Added RobotDyn 6 Digit module,
 *                          Added Eyewink 6 Digit + 5 Keys module,
 *                          Constructor adapted to 2 pins: dio, clk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "Font_7Seg.h"

// Select one of the testboards for TM1637 LED controller
#if ((EYEWINK_TEST == 1) || (ROBOTDYN_TEST == 1) || (CATALEX_TEST == 1))

//Mask for blending out and restoring Icons
const char MASK_ICON_GRID[] = {
                                LO(S7_ICON_GR1),
                                LO(S7_ICON_GR2),
                                LO(S7_ICON_GR3),
                                LO(S7_ICON_GR4),
                                LO(S7_ICON_GR5),
                                LO(S7_ICON_GR6)
                              };

// ASCII Font definition table for transmission to TM1637
//
//#define FONT_7S_START     0x20
//#define FONT_7S_END       0x7F
//#define FONT_7S_NR_CHARS (FONT_7_END - FONT_7S_START + 1)

#if (SHOW_ASCII == 1)
//display all ASCII characters
const short FONT_7S[]  = {
                             C7_SPC, //32 0x20, Space
                             C7_EXC,
                             C7_QTE,
                             C7_HSH,
                             C7_DLR,
                             C7_PCT,
                             C7_AMP,
                             C7_ACC,
                             C7_LBR,
                             C7_RBR,
                             C7_MLT,
                             C7_PLS,
                             C7_CMA,
                             C7_MIN,
                             C7_DPT,
                             C7_RS,
                             C7_0,   //48 0x30
                             C7_1,
                             C7_2,
                             C7_3,
                             C7_4,
                             C7_5,
                             C7_6,
                             C7_7,
                             C7_8,
                             C7_9,
                             C7_COL, //58 0x3A
                             C7_SCL,
                             C7_LT,
                             C7_EQ,
                             C7_GT,
                             C7_QM,
                             C7_AT,  //64 0x40
                             C7_A,   //65 0x41, A
                             C7_B,
                             C7_C,
                             C7_D,
                             C7_E,
                             C7_F,
                             C7_G,
                             C7_H,
                             C7_I,
                             C7_J,
                             C7_K,
                             C7_L,
                             C7_M,
                             C7_N,
                             C7_O,
                             C7_P,
                             C7_Q,
                             C7_R,
                             C7_S,
                             C7_T,
                             C7_U,
                             C7_V,
                             C7_W,
                             C7_X,
                             C7_Y,
                             C7_Z,   //90 0x5A, Z
                             C7_SBL, //91 0x5B
                             C7_LS,
                             C7_SBR,
                             C7_PWR,
                             C7_UDS,
                             C7_ACC,
                             C7_A,   //97 0x61, A replacing a
                             C7_B,
                             C7_C,
                             C7_D,
                             C7_E,
                             C7_F,
                             C7_G,
                             C7_H,
                             C7_I,
                             C7_J,
                             C7_K,
                             C7_L,
                             C7_M,
                             C7_N,
                             C7_O,
                             C7_P,
                             C7_Q,
                             C7_R,
                             C7_S,
                             C7_T,
                             C7_U,
                             C7_V,
                             C7_W,
                             C7_X,
                             C7_Y,
                             C7_Z,   // 122 0x7A, Z replacing z
                             C7_CBL, // 123 0x7B
                             C7_OR,
                             C7_CBR,
                             C7_TLD,
                             C7_DEL  // 127
                        };

#else
//display only digits and hex characters
const short FONT_7S[] = {
                           C7_0, //48 0x30
                           C7_1,
                           C7_2,
                           C7_3,
                           C7_4,
                           C7_5,
                           C7_6,
                           C7_7,
                           C7_8,
                           C7_9,
                           C7_A, //65 0x41, A
                           C7_B,
                           C7_C,
                           C7_D,
                           C7_E,
                           C7_F
                        };// 127
#endif

#endif
