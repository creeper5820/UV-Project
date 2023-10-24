/*-----------------------------------------------------------------------------
 * Name:    Joystick_7B3I_EVAL.c
 * Purpose: Joystick interface for STM32H7B3I-EVAL Board
 * Rev.:    1.0.1
 * Note:    Joystick shares pins with SDRAM. Do not use both at the same time!
 *----------------------------------------------------------------------------*/

/* Copyright (c) 2020 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

#include "stm32h7b3i_eval.h"
#include "Board_Joystick.h"


/**
  \fn          int32_t Joystick_Initialize (void)
  \brief       Initialize joystick
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Joystick_Initialize (void) {
  int32_t ret;

  ret = BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);

  if (ret != BSP_ERROR_NONE) {
    return -1;
  }

  return 0;
}

/**
  \fn          int32_t Joystick_Uninitialize (void)
  \brief       De-initialize joystick
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Joystick_Uninitialize (void) {

  BSP_JOY_DeInit(JOY1, JOY_ALL);

  return 0;
}

/**
  \fn          uint32_t Joystick_GetState (void)
  \brief       Get joystick state
  \returns     Joystick state
*/
uint32_t Joystick_GetState (void) {
  int32_t state;

  state = BSP_JOY_GetState(JOY1);

  switch (state) {
    case JOY_NONE:
      return 0U;
    case JOY_SEL:
      return JOYSTICK_CENTER;
    case JOY_DOWN:
      return JOYSTICK_DOWN;
    case JOY_LEFT:
      return JOYSTICK_LEFT;
    case JOY_RIGHT:
      return JOYSTICK_RIGHT;
    case JOY_UP:
      return JOYSTICK_UP;
  }

  return 0U;
}
