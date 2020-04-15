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
#define F_CPU               2400000UL   /* 9.6 MHz / 4 */
#define PRESCALE            8UL
#define OVF_FACTOR          (F_CPU) / (256 * PRESCALE * 1000)

#define T_DEBOUNCE_MS       8UL         /* Period to check for debouncing */
#define T_OFF_PRESS_MS      1000UL      /* Delay to recognise press to turn off */
#define T_LED_FLASH_MS      500UL       /* 1/2 Period of LED flash */

#define OVF_CNT_DEBOUNCE    T_DEBOUNCE_MS * OVF_FACTOR
#define OVF_CNT_OFF_PRESS   T_OFF_PRESS_MS * OVF_FACTOR
#define OVF_CNT_LED_FLASH   T_LED_FLASH_MS * OVF_FACTOR

// Power states
typedef enum {
    OFF,
    START,
    ON,
    STOP
} power_fsm_t;

// Button states
typedef enum {
    OPEN,
    CLOSED,
} sw_t;

#define SW_CLOSED           0x00U       /* Closed switch is LOW */
#define SW_OPEN             0xFFU       /* Open switch is HIGH */

// Functions
static inline void setup(void);

#endif /* AUDIOSTICK_H_ */
