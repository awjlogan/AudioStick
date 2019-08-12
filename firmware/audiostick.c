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

 * Author:  Angus Logan
 * Date:    June 2019
 * Target:  Microchip [Atmel] ATtiny13A
 *
 * Physical <-> logical pin mapping
 *
 * Package: SS (SOIC8)
 * Physical     Function        Logical     i/o
 *----------------------------------------------
 *  1           !RST (switch)   PB5         i
 *  2           S_REQ           PB3         o
 *  3           RP_ACK          PB4         i
 *  4           < GND >
 *  5           PWR             PB0         o
 *  6           LED             PB1         o
 *  7           RP_REQ          PB2         i
 *  8           < VDD >
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "audiostick.h"

static inline void setup(void);

ISR (TIM0_OVF_vect) {
    // Reset for idle interrupt wakeup
    TCNT0 = TIM_VAL;
}

int main(void)
{
    /* Switch handler */
    const uint8_t SW_OPEN = 0xFF;
    const uint8_t SW_CLOSED = 0x0;
    uint8_t sw_state = SW_OPEN;
    sw_t sw_curr = OPEN;
    sw_t sw_last = OPEN;
    uint8_t was_pressed = 0U;

    /* Timers */
    uint16_t rst_assert = 0x0;  // timer for reset button
    uint16_t wdt_assert = 0x0;  // timer to catch a hang in a state
    uint8_t wdt_overflow = 0U;

    /* FSM */
    power_fsm_t power_fsm = OFF;
    power_fsm_t power_fsm_nxt = OFF;
    uint8_t rp_ack= 0;

    /* Driven outputs */
    uint8_t portb_nxt;
    uint8_t flash_val = NORM_VAL;
    uint8_t flash_cnt = 0;

    /* Main application */
    setup();

    for (;;) {

        /* Debounce switch
           Shift all left, then add current to LSB
            -> compare again CLOSED/OPEN (depending on pullup/down)
        */
        sw_state <<= 1;
        sw_state += (PINB & (1<<SW)) >> SW;
        if (sw_state == SW_CLOSED) sw_curr = CLOSED;
        else if (sw_state == SW_OPEN) sw_curr = OPEN;

        /* Reused conditions */
        was_pressed = (sw_curr == CLOSED) && (sw_last == OPEN);
        wdt_overflow = (wdt_assert > TIMEOUT_VAL);
        rp_ack = (PINB & (1<<RP_ACK));

        // update FSM
        switch (power_fsm) {
        case OFF:
            if (was_pressed) power_fsm_nxt = START_REQ;
            break;
        case START_REQ:
            if (wdt_overflow) power_fsm_nxt = ERROR;
            else if (rp_ack) power_fsm_nxt = START_ACK;
            break;
        case START_ACK:
            if (wdt_overflow) power_fsm_nxt = ERROR;
            else if (!rp_ack) power_fsm_nxt = ON;
            break;
        case ON:
            if (was_pressed) power_fsm_nxt = SHUT_REQ;
            // request to shutdown from RPi
            else if (PINB & (1<<RP_REQ)) power_fsm_nxt = SHUT_REQ;
            break;
        case SHUT_REQ:
            if (wdt_overflow) power_fsm_nxt = ERROR;
            else if (rp_ack) power_fsm_nxt = SHUT_ACK;
            break;
        case SHUT_ACK:
            if (wdt_overflow) power_fsm_nxt = ERROR;
            else if (!rp_ack) power_fsm_nxt = OFF;
            break;
        case ERROR:
            break;
        }

        /* Update the watchdog timer */
        if (power_fsm != power_fsm_nxt) wdt_assert = 0U;
        else wdt_assert++;

        /* Check for reset */
        if ((sw_curr == CLOSED) && (sw_last == CLOSED)) rst_assert++;
        else rst_assert = 0U;
        if (rst_assert > RST_VAL) power_fsm_nxt = OFF;

        power_fsm = power_fsm_nxt;
        sw_last = sw_curr;

        /* Drive FSM outputs */

        portb_nxt = PORTB;

        // Flash by default, exceptions ON/OFF are handled in FSM
        flash_cnt++;
        if (flash_cnt > flash_val) {
            portb_nxt ^= (1<<LED);
            flash_cnt = 0U;
        }

        switch (power_fsm) {
        case OFF:
            flash_val = NORM_VAL;
            portb_nxt &= ~(1<<LED) & ~(1<<PWR);  // LED & power off
            break;
        case START_REQ:
            portb_nxt |= (1<<PWR) | (1<<S_REQ);  // power on and assert S_REQ
            break;
        case START_ACK:
            portb_nxt &= ~(1<<S_REQ);  // Clear S_REQ
            break;
        case ON:
            portb_nxt |= (1<<LED);  // LED on
            break;
        case SHUT_REQ:
            portb_nxt |= (1<<S_REQ);  // Assert S_REQ
            break;
        case SHUT_ACK:
            portb_nxt &= ~(1<<S_REQ);  // Clear S_REQ
            break;
        case ERROR:
            flash_val = ERR_VAL;
            break;
        }
        PORTB = portb_nxt;

        // Sleep and wait for timer0 interrupt
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
}

static inline void setup(void) {
    /* Clocking: default */
    /* Interrupts: default */

    /* Timer0 */
    /* Overflow every INT_PER ms */
    TCCR0B = (1<<CS01) | (1<<CS00);  // clk / 64
    TIMSK0 = (1<<TOIE0);

    /* Ports: */
    MCUCR = (1<<PUD);  // disable pull ups
    DDRB = (1<<DDB0) | (1<<DDB1) | (1<<DDB3);

    /* Power reduction */
    PRR = (1<<PRADC);  // ADC off
    ACSR = (1<<ACD);  // analog comparator off
}
