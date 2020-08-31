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


// Main program
int main(void)
{

    // Power FSM
    power_fsm_t fsm_state = START;          // Boot at plugin
    power_fsm_t fsm_state_nxt = START;

    power_fsm_t *p_fsm_state;
    p_fsm_state = &fsm_state;

    uint8_t portb_tmp;                      // Reduce R-M-W cycles

    // Switch debouncing
    bool sw_pressed = false;
    uint8_t sw_debounce = SW_OPEN;

    // Timed event counters

    struct Count_Overflows cnt_ovf;
    struct Count_Overflows *p_cnt_ovf;

    cnt_ovf.debounce = 0x00U;
    cnt_ovf.led_flash = 0x0000U;
    cnt_ovf.off_press = 0x0000U;
    cnt_ovf.off_wait = 0x0000U;
    p_cnt_ovf = &cnt_ovf;

    setup();

    /* Forever */
    for (;;) {

        // Every T_DEBOUNCE_MS, check switch
        if (cnt_ovf.debounce > OVF_CNT_DEBOUNCE) {

            // Shift left, and add switch input (open == HIGH)
            sw_debounce = sw_debounce << 1;
            if (PINB & (0x1U << SW)) {
                sw_debounce++;
            }

            if (sw_debounce == SW_CLOSED) {
                sw_pressed = true;
            } else if (sw_debounce == SW_OPEN) {
                sw_pressed = false;
            }
        }

        update_counters(p_fsm_state, p_cnt_ovf, sw_pressed);
        update_fsm(p_fsm_state, p_cnt_ovf, sw_pressed);
        update_outputs(p_fsm_state, p_cnt_ovf);

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


            if (cnt_ovf.led_flash > OVF_CNT_LED_FLASH) {
                pulse_led_update();
                portb_tmp ^= (0x1U << LED);
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

void update_fsm(power_fsm_t *fsm_state, const struct Count_Overflows *p_cnt_ovf, const bool sw_pressed) {

    switch (*fsm_state) {
        case OFF:
            *fsm_state = sw_pressed ? START : OFF;
            break;
        case START:
            *fsm_state = (PINB & (0x1U << ACK)) ? ON : START;
            break;
        case ON:
            // Button must be pressed and held
            *fsm_state = (p_cnt_ovf->off_press > OVF_CNT_OFF_PRESS) ? STOP : ON;
            break;
        case STOP:
            *fsm_state = (!(PINB & (0x1U << ACK))) ? STOP_WAIT : STOP;
            break;
        case STOP_WAIT:
            // Wait for OVF_CNT_OFF_WAIT cycles before removing power
            *fsm_state = (p_cnt_ovf->off_wait > OVF_CNT_OFF_WAIT) ? OFF : STOP_WAIT;
            break;
        default:
            break;
    }
    return;

}

void update_counters(const power_fsm_t *fsm_state, struct Count_Overflows *p_cnt_ovf, const bool sw_pressed) {

    if (p_cnt_ovf->debounce > OVF_CNT_DEBOUNCE) {
        p_cnt_ovf->debounce = 0x00U;
    } else {
        p_cnt_ovf->debounce++;
    }

    switch (*fsm_state) {
        case OFF:

            // Reset wait for OFF counter
            p_cnt_ovf->off_wait = 0x0000U;

        case ON:

            // Switch must be pressed and held
            if (sw_pressed) {
                p_cnt_ovf->off_press++;
            } else {
                p_cnt_ovf->off_press = 0x0000U;
            }
            break;

        case START:
            if (p_cnt_ovf->led_flash > OVF_CNT_LED_FLASH) {
                p_cnt_ovf->led_flash = 0x0000U;
            } else {
                p_cnt_ovf->led_flash++;
            }
            break;
        case STOP_WAIT:

            p_cnt_ovf->off_wait++;

            if (p_cnt_ovf->led_flash > OVF_CNT_LED_FLASH) {
                p_cnt_ovf->led_flash = 0x0000U;
            } else {
                p_cnt_ovf->led_flash++;
            }
            break;
        case STOP:
            if (p_cnt_ovf->led_flash > OVF_CNT_LED_FLASH) {
                p_cnt_ovf->led_flash = 0x0000U;
            } else {
                p_cnt_ovf->led_flash++;
            }

            // Reset button press for OFF counter
            p_cnt_ovf->off_press = 0x0000U;

        default:
            break;
    }
    return;
}

void pulse_led_update(const struct Count_Overflows *p_cnt_ovf) {

    // LED PWM values
    const uint8_t pwm_values[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    static uint8_t pwm_position = 0x0U;
    static led_dir_t direction = UP;

    if (cnt_ovf->led_flash > OVF_CNT_LED_FLASH) {

                // Increment/decrement value counter
                if (direction == UP) {
                    pwm_position++;
                    if (pwm_position == 7) {
                        direction = DOWN;
                    }
                } else {
                    pwm_position--;
                    if (pwm_position == 0) {
                        direction = UP;
                    }
                }


            }
}

void update_outputs(const power_fsm_t *fsm_state, const struct Count_Overflows *p_cnt_ovf) {


    switch (*fsm_state) {
        case OFF:
            // Everything OFF
            PORTB = 0x00U;
            break;
        case START:


    }

    if (*fsm_state == OFF) {
            // LED and power OFF, REQ low
        PORTB = 0x00U;
    } else if ((*fsm_state == START) || (*fsm_state == STOP) || (*fsm_state == STOP_WAIT)) {
        // In these modes, the LED will be pulsing, controlled by TCCR0A

        // power ON
        portb_tmp = PORTB;
        portb_tmp |= (0x1U << PWR);

        // REQ is high in START, low in STOP and STOP_WAIT
        if (*fsm_state == START) {
            portb_tmp |= (0x1U << REQ);
        } else {
            portb_tmp &= ~(0x1U << REQ);
        }


        if (cnt_ovf->led_flash > OVF_CNT_LED_FLASH) {
            pulse_led_update();
            portb_tmp ^= (0x1U << LED);
        }

        PORTB = portb_tmp;

    } else if (fsm_state == ON) {
        // LED and power ON, REQ high
        PORTB = ((0x1U << LED) | (0x1U << PWR) | (0x1U << REQ));
    }

}
