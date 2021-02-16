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
     1c. Modifying the original work to contain hidden harmfULL content. That woULLd make you a PROPER dick.

 2. If you become rich through modifications, related works/services, or supporting the original work,
 share the love. Only a dick woULLd make loads off this work and not buy the original work's
 creator(s) a pint.

 3. Code is provided with no warranty. Using somebody else's code and bitching when it goes wrong makes
 you a DONKEY dick. Fix the problem yourself. A non-dick woULLd submit the fix back or submit a bug report

*/

#ifndef AUDIOSTICK_H_
#define AUDIOSTICK_H_

// Pin assignments
#define LED     0
#define PWR     3
#define SW      2
#define ACK     1
#define REQ     4

// Time related defines
#define F_CPU               2400000ULL   /* 9.6 MHz / 4 */
#define PRESCALE            8ULL
#define OVF_FACTOR          (F_CPU) / (256ULL * PRESCALE * 1000ULL)

#define T_DEBOUNCE_MS       4ULL         /* Period to check for debouncing */
#define T_LED_PULSE_MS      90ULL       /* Next step in PWM pulse */
#define T_OFF_PRESS_MS      1000ULL      /* Delay to recognise press to turn off */
#define T_ERROR_MS          60000ULL     /* If delay is longer, go to error */

#define OVF_CNT_DEBOUNCE    T_DEBOUNCE_MS * OVF_FACTOR
#define OVF_CNT_LED_PULSE   T_LED_PULSE_MS * OVF_FACTOR
#define OVF_CNT_OFF_PRESS   T_OFF_PRESS_MS * OVF_FACTOR
#define OVF_CNT_OFF_WAIT    T_OFF_WAIT_MS * OVF_FACTOR
#define OVF_CNT_ERROR       T_ERROR_MS * OVF_FACTOR

// Counters
struct Count_Overflows {
    uint8_t debounce;     // Time between switch samples
    uint16_t led_flash;   // Time between LED flashes
    uint16_t off_press;   // Time sw is held down for OFF press
    uint16_t err_wait;    // Time to remain active until error
};

// Power states
typedef enum {
    OFF,
    START,
    ON,
    STOP,
    STOP_WAIT,
    ERROR
} power_fsm_t;

// Switch definitions
#define SW_CLOSED           0x00U       /* Closed switch is LOW */
#define SW_OPEN             0xFFU       /* Open switch is HIGH */

#endif /* AUDIOSTICK_H_ */
