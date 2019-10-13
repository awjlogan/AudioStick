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
#define SW      5
#define RP_ACK  4
#define S_REQ   3
#define RP_REQ  2
#define LED     1
#define PWR     0

// Time related defines
#define F_CPU       1200UL                                  /* Default clocking: 9.6 MHz / 8 (kHz) */
#define CLK_DIV     64U                                     /* TMR0 clock divider from F_CPU */
#define INT_PER     8U                                      /* Time between wakeups (ms) */
#define TMR0_VAL    (255U - (F_CPU * INT_PER / CLK_DIV))    /* Value to reset TMR0 */
#define NORM_PER    500UL                                   /* Normal flashing period (ms) */
#define NORM_VAL    NORM_PER / INT_PER                      /* Compare value */
#define ERR_PER     250U                                    /* Error flashing period (ms) */
#define ERR_VAL     ERR_PER / INT_PER                       /* Compare value */
#define RST_PER     2500UL                                  /* Time to hold for reset */
#define RST_VAL     RST_PER / INT_PER                       /* Compare value */
#define TIMEOUT_PER 32000UL                                 /* Error time out (ms) */
#define TIMEOUT_VAL TIMEOUT_PER / INT_PER                   /* Compare value */

// Power states
typedef enum {
    OFF,
    START_REQ,
    START_ACK,
    ON,
    SHUT_REQ,
    SHUT_ACK,
    ERROR
} power_fsm_t;

// Button states
typedef enum {
    OPEN,
    CLOSED
} sw_t;

#endif /* AUDIOSTICK_H_ */
