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
 *  3           LED             PB0         o
 *  4           < GND >                     -
 *  5           REQ             PB4         o
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

// ISRs
EMPTY_INTERRUPT (TIM0_OVF_vect)             // Timer0 overflow just wakes up

static inline void setup_avr(void) {
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

static inline void update_sw(const struct Count_Overflows *p_cnt_ovf, bool *p_sw_pressed) {

    static uint8_t sw_debounce = SW_OPEN;

    if (p_cnt_ovf->debounce > OVF_CNT_DEBOUNCE) {

        // Shift left, and add switch input (open == HIGH)
        sw_debounce = sw_debounce << 1;
        if (PINB & (0x1U << SW)) {
            sw_debounce++;
        }

        if (sw_debounce == SW_CLOSED) {
            *p_sw_pressed = true;
        } else if (sw_debounce == SW_OPEN) {
            *p_sw_pressed = false;
        }
    }
}

static inline void update_fsm(power_fsm_t *p_fsm_state, const struct Count_Overflows *p_cnt_ovf, bool sw_pressed) {

    switch (*p_fsm_state) {
        case OFF:

            *p_fsm_state = sw_pressed ? START : OFF;
            break;

        case START:

            *p_fsm_state = (PINB & (0x1U << ACK)) ? ON : START;
            *p_fsm_state = (p_cnt_ovf->err_wait > OVF_CNT_ERROR) ? ERROR : *p_fsm_state;
            break;

        case ON:

            // Button must be pressed and held
            *p_fsm_state = (p_cnt_ovf->off_press > OVF_CNT_OFF_PRESS) ? STOP : ON;
            break;

        case STOP:

            *p_fsm_state = (!(PINB & (0x1U << ACK))) ? OFF : STOP;
            *p_fsm_state = (p_cnt_ovf->err_wait > OVF_CNT_ERROR) ? ERROR : *p_fsm_state;
            break;

        case ERROR:

            // Button must be held 3X off_press to reset to OFF
            *p_fsm_state = (p_cnt_ovf->off_press > 3 * OVF_CNT_OFF_PRESS) ? OFF : ERROR;
        default:
            break;
    }
    return;
}

void update_counters(const power_fsm_t *p_fsm_state, struct Count_Overflows *p_cnt_ovf, const bool *p_sw_pressed) {

    // Update debounce counter in all states
    if (p_cnt_ovf->debounce > OVF_CNT_DEBOUNCE) {
        p_cnt_ovf->debounce = 0x00U;
    } else {
        p_cnt_ovf->debounce++;
    }

    switch (*p_fsm_state) {

        case OFF:

            p_cnt_ovf->off_press = 0x0000U;
            break;

        case ON:

            p_cnt_ovf->err_wait = 0x0000U;

            // Switch must be pressed and held
            if (*p_sw_pressed) {
                p_cnt_ovf->off_press++;
            } else {
                p_cnt_ovf->off_press = 0x0000U;
            }
            break;

        case START:

            p_cnt_ovf->err_wait++;

            if (p_cnt_ovf->led_flash > OVF_CNT_LED_PULSE) {
                p_cnt_ovf->led_flash = 0x0000U;
            } else {
                p_cnt_ovf->led_flash++;
            }
            break;

        case STOP:

            p_cnt_ovf->err_wait++;

            if (p_cnt_ovf->led_flash > OVF_CNT_LED_PULSE) {
                p_cnt_ovf->led_flash = 0x0000U;
            } else {
                p_cnt_ovf->led_flash++;
            }

            // Reset button press for OFF counter
            p_cnt_ovf->off_press = 0x0000U;
            break;

        case ERROR:

            p_cnt_ovf->err_wait = 0x0000U;

            if (*p_sw_pressed) {
                p_cnt_ovf->off_press++;
            } else {
                p_cnt_ovf->off_press = 0x0000U;
            }

        default:
            break;
    }
    return;
}

void pulse_led_update(const struct Count_Overflows *p_cnt_ovf) {

    // LED PWM values
    const uint8_t pwm_values[16] = {0, 2, 4, 16, 32, 64, 128, 255, 255, 128, 64, 32, 16, 4, 2, 0};
    static uint8_t pwm_position = 0x00U;

    // Enable the compare-match output on OCOA
    // Fast PWM, output on OCOA
    TCCR0A = (0x1U << COM0A1) | (0x1U << WGM01) | (0x1U << WGM00);

    // Set PWM value
    if (p_cnt_ovf->led_flash > OVF_CNT_LED_PULSE) {
        // mod 16 overflow
        OCR0A = pwm_values[pwm_position++ & 0x0FU];
    }
}

static inline void update_outputs(const power_fsm_t *p_fsm_state, const struct Count_Overflows *p_cnt_ovf) {

    switch (*p_fsm_state) {
        case OFF:

            // Everything OFF
            PORTB = 0x00U;
            // Disable LED PWM
            TCCR0A = (0x1U << WGM01) | (0x1U << WGM00);
            break;

        case START:

            PORTB |= (0x1U << PWR) | (0x1U << REQ);
            pulse_led_update(p_cnt_ovf);
            break;

        case ON:

            // Disable LED PWM, set ON
            TCCR0A = (0x1U << WGM01) | (0x1U << WGM00);
            PORTB |= (0x1U << LED);
            break;

        case STOP:

            PORTB &= ~(0x1U << REQ);
            pulse_led_update(p_cnt_ovf);
            break;

        case ERROR:

            TCCR0A = (0x1U << WGM01) | (0x1U << WGM00);
            // REVISIT flash LED
            break;

        default:
            break;
    }
    return;
}



// Main program
int main(void)
{

    power_fsm_t fsm_state = OFF;          // Start in OFF, stops wakeup on power cut etc
    bool sw_pressed = false;
    struct Count_Overflows cnt_ovf = {0}; // Init all counters to 0

    setup_avr();

    /* Forever */
    for (;;) {

        update_sw(&cnt_ovf, &sw_pressed);
        update_counters(&fsm_state, &cnt_ovf, &sw_pressed);
        update_fsm(&fsm_state, &cnt_ovf, sw_pressed);
        update_outputs(&fsm_state, &cnt_ovf);

        // Sleep and wait for TIM0 interrupt
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
}

