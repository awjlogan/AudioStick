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

ISR(TIM0_COMPB_vect) {
	// Turn LED off for PWM effect
	PORTB &= ~(1 << LED);
}

ISR (TIM0_OVF_vect) {
    // Turn LED on for PWM effect
    PORTB |= (1 << LED);
}

int main(void)
{

    // Power FSM
    power_fsm_t fsm_state = OFF;
    power_fsm_t fsm_state_nxt = OFF;
    bool rpi_ack = false;

    // Switch debouncing
    bool sw_pressed = false;
    uint8_t cnt_ovf_debounce = 0x0U;
    uint8_t sw_debounce = 0x0U;
    sw_t sw_current = OPEN;
    sw_t sw_last = OPEN;

    uint8_t cnt_ovf_pulse = 0x0U;
    uint8_t idx_pwm = 0x0U;
    bool brighten = true;

    const uint8_t pwm_vals[8] = {4, 8, 16, 32, 64, 128, 200, 255};

    setup();

    /* Forever */
    for (;;) {

        // Turn on LED
        cnt_ovf_pulse++;
        if (cnt_ovf_pulse == 146) {
        	cnt_ovf_pulse = 0x0U;

        	if (brighten) {
        		idx_pwm++;
        	} else {
        		idx_pwm--;
        	}
        	if (idx_pwm == 7U) {
    			brighten = false;
    		} else if (idx_pwm == 0x0U) {
    			brighten = true;
    		}
    		OCR0B = pwm_vals[idx_pwm];
        }

        // Every 8 ms, check switch
        // REVISIT magic number calculation in defines
        // REVISIT refactor
        sw_pressed = false;
        cnt_ovf_debounce++;
        if (cnt_ovf_debounce == 106) {
            cnt_ovf_debounce = 0;

            sw_debounce = sw_debounce << 1;
            if (!(PINB & (0x1U << SW))) {
                sw_debounce++;
            }

            if (sw_debounce == 0xFFU) {
                sw_current = CLOSED;
            } else if (sw_debounce == 0x0U) {
                sw_current = OPEN;
            }

            if (sw_current != sw_last) {
                if (sw_current == CLOSED) {
                    // REVISIT remove when working
                    // PORTB ^= (1 << LED) | (1 << PWR);
                    sw_pressed = true;
                }
            }
            sw_last = sw_current;

        }

        fsm_state_nxt = fsm_state;
        rpi_ack = PINB & (1 << ACK);
        switch (fsm_state) {
            case OFF:
                if (sw_pressed) {
                    fsm_state_nxt = START;
                }
                break;
            case START:
                if (rpi_ack) {
                    fsm_state_nxt = ON;
                }
                break;
            case ON:
                if (!rpi_ack) {
                    fsm_state_nxt = SHUTDOWN_REQ;
                } else if (sw_pressed) {
                    fsm_state_nxt = SHUTDOWN;
                }
                break;
            case SHUTDOWN_REQ:
                if (rpi_ack) {
                    fsm_state_nxt = SHUTDOWN;
                }
                break;
            case SHUTDOWN:
                if (!rpi_ack) {
                    fsm_state_nxt = OFF;
                }
                break;
            default:
                fsm_state_nxt = ERROR;
        }


        // Sleep and wait for timer0 interrupt
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
    TCCR0B = (1U<<CS01);  // clk / 8 -> 1.176 kHz overflow
    TIMSK0 = (1U << TOIE0) | (1U << OCIE0B); // overflow, compare B

    /* Port B: */
    MCUCR = (1U<<PUD);  // disable pull ups
    // Configure outputs
    DDRB = (1U<<DDB0) | (1U<<DDB3) | (1U<<DDB4);

    /* Power reduction */
    PRR = (1U<<PRADC);  // ADC off
    ACSR = (1U<<ACD);  // analog comparator off
}

