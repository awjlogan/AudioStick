/*

> Copyright (C) 2020 Angus Logan

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document.

> DON'T BE A DICK PUBLIC LICENSE
> TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 1. Do whatever you like with the original work, just don't be a dick.

     Being a dick includes - but is not limited to - the following instances:

     1a. Outright copyright infringement - Don't just copy the original work/works and change the name.
     1b. Selling the unmodified original with no work done what-so-ever, that's REALLY being a dick.
     1c. Modifying the original work to contain hidden harmful content. That would make you a PROPER dick.

 2. If you become rich through modifications, related works/services, or supporting the original work,
 share the love. Only a dick would make loads off this work and not buy the original work's
 creator(s) a pint.

 3. Code is provided with no warranty. Using somebody else's code and bitching when it goes wrong makes
 you a DONKEY dick. Fix the problem yourself. A non-dick would submit the fix back or submit a bug report

 * Author:  Angus Logan
 * Version: 2.0
 * Date:    March 2020
 * Target:  Microchip [Atmel] ATtiny13A
 *
 * Physical <-> logical pin mapping
 *
 * Package: SOIC8/DIP8
 * Physical     Function        Logical     i/o
 *----------------------------------------------
 *  1           !RST            PB5         i
 *  2           PWR             PB3         o
 *  3           LED             PB4         o
 *  4           < GND >                     -
 *  5           REQ             PB0         o
 *  6           ACK             PB1         i
 *  7           SW              PB2         i
 *  8           < VDD >                     -
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdbool.h>
#include <stdint.h>

#include "audiostick.h"

ISR (TIM0_OVF_vect) {
    // Just roll over to trigger wake up
}

int main(void)
{

    // Power FSM
    power_fsm_t fsm_state = OFF;          // Boot at plugin
    power_fsm_t fsm_state_nxt = OFF;
    uint8_t portb_tmp;                      // Reduce R-M-W cycles

    // Switch debouncing
    bool sw_pressed = false;
    uint8_t sw_debounce = SW_OPEN;
    sw_t sw_current = OPEN;

    // Timed event counters
    uint8_t cnt_ovf_debounce = 0x00U;       // Time between switch samples
    uint16_t cnt_ovf_led_flash = 0x0000U;   // Time between LED flashes
    uint16_t cnt_ovf_off_press = 0x0000U;   // Time sw is held down for OFF press
    uint16_t cnt_ovf_off_wait = 0x0000U;    // Time to remain powered after !ACK

    setup();

    /* Forever */
    for (;;) {

        // Every T_DEBOUNCE_MS, check switch
        sw_pressed = false;
        cnt_ovf_debounce++;
        if (cnt_ovf_debounce >= OVF_CNT_DEBOUNCE) {
            cnt_ovf_debounce = 0x00U;

            // Shift left, and add switch input (open == HIGH)
            sw_debounce = sw_debounce << 1;
            if (PINB & (0x1U << SW)) {
                sw_debounce++;
            }

            if (sw_debounce == SW_CLOSED) {
                sw_current = CLOSED;
            } else if (sw_debounce == SW_OPEN) {
                sw_current = OPEN;
            }

            if (sw_current == CLOSED) {
                sw_pressed = true;
                cnt_ovf_off_press++;
            }
        }

        // FSM update
        switch (fsm_state) {
            case OFF:
                fsm_state_nxt = sw_pressed ? START : OFF;
                // Reset timer counters
                cnt_ovf_off_wait = 0x0000U;
                cnt_ovf_off_press = 0x0000U;
                break;
            case START:
                fsm_state_nxt = (PINB & (0x1U << ACK)) ? ON : START;
                break;
            case ON:
                fsm_state_nxt = (sw_pressed && (cnt_ovf_off_press > OVF_CNT_OFF_PRESS)) ? STOP : ON;
                break;
            case STOP:
                fsm_state_nxt = (!(PINB & (0x1U << ACK))) ? STOP_WAIT : STOP;
                break;
            case STOP_WAIT:
                // Wait for OVF_CNT_OFF_WAIT cycles before removing power
                cnt_ovf_off_wait++;
                fsm_state_nxt = (cnt_ovf_off_wait > OVF_CNT_OFF_WAIT) ? OFF : STOP_WAIT;
                break;
            default:
                break;
        }

        // Outputs
        fsm_state = fsm_state_nxt;
        if (fsm_state == OFF) {
            // LED and power OFF, REQ low
            // REVISIT pulse LED
            PORTB = 0x00U;
        } else if ((fsm_state == START) || (fsm_state == STOP) || (fsm_state == STOP_WAIT)) {
            // LED FLASH, power ON
            portb_tmp = PORTB;
            portb_tmp |= (0x1U << PWR);

            // REQ is high in START, low in STOP and STOP_WAIT
            if (fsm_state == START) {
                portb_tmp |= (0x1U << REQ);
            } else {
                portb_tmp &= ~(0x1U << REQ);
            }

            cnt_ovf_led_flash++;
            if (cnt_ovf_led_flash > OVF_CNT_LED_FLASH) {
                portb_tmp ^= (0x1U << LED);
                cnt_ovf_led_flash = 0x0000U;
            }

            PORTB = portb_tmp;

        } else if (fsm_state == ON) {
            // LED and power ON, REQ high
            PORTB = ((0x1U << LED) | (0x1U << PWR) | (0x1U << REQ));
        }

        // Sleep and wait for TIM0 interrupt
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
}

inline void setup(void) {
    /*  Clocking
        F_CPU = 9.6 MHz / 4 = 2.4 MHz
        Set top bit 1 and then write within 4 cycles
    */
    cli();
    CLKPR = (0x1U << CLKPCE);
    CLKPR = (0x1U << CLKPS1);
    sei();

    /* Interrupts: default */

    /* Timer0 */
    /* Overflow every INT_PER ms */
    TCCR0B = (0x1U << CS01);  // clk / 8 -> 1.176 kHz overflow
    TIMSK0 = (0x1U << TOIE0); // Interrupt on overflow

    /* Port B: */
    MCUCR = (0x1U << PUD);  // disable pull ups
    // Configure outputs
    DDRB = (0x1U << REQ) | (0x1U << PWR) | (0x1U << LED);

    /* Power reduction */
    PRR = (0x1U << PRADC);  // ADC off
    ACSR = (0x1U << ACD);  // analog comparator off
}

