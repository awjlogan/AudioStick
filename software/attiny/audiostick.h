/*

Copyright 2019 Angus Logan

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef AUDIOSTICK_H_
#define AUDIOSTICK_H_

// Pin assignments
#define LED     4
#define PWR     3
#define SW      2
#define ACK     1
#define REQ     0

// Time related defines
#define F_CPU               2400UL /* 9.6 MHz / 4 (kHz) */

#define PWM_RES             8U
#define PWM_UPDATE_CNT      (F_CPU / 256) / PWM_RES

// Power states
typedef enum {
    OFF,
    START,
    ON,
    SHUTDOWN_REQ,
    SHUTDOWN,
    ERROR
} power_fsm_t;

// Button states
typedef enum {
    OPEN,
    CLOSED,
    BOUNCE
} sw_t;

// Functions
static inline void setup(void);
static inline sw_t debounce_switch(uint8_t *sw_raw);

#endif /* AUDIOSTICK_H_ */
