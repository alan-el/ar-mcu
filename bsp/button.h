#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

#define BUTTONS_NUMBER  3

#define BUTTON1_IO_NUM  15
#define BUTTON2_IO_NUM  16
#define BUTTON3_IO_NUM  17


#define BUTTON_LONG_PUSH_TIMEOUT_MS 2000
#define BUTTON_ACTIVE_STATE     APP_BUTTON_ACTIVE_HIGH

#define BUTTON_ACTION_LONG_PUSH 2

#define BUTTON_CFG(_pin_no) \
{ \
    .pin_no = _pin_no, \
    .active_state = BUTTON_ACTIVE_STATE, \
    .pull_cfg = NRF_GPIO_PIN_PULLDOWN, \
    .button_handler = button_evt_handler, \
}

typedef enum
{
    BUTTON_EVENT_NOTHING = 0,
    BUTTON_EVENT_FOO,
    
}button_event_t;

typedef struct
{
    button_event_t push_event;      /**< The event to fire on regular button press. */
    button_event_t release_event;   /**< The event to fire on button release. */
    button_event_t long_push_event; /**< The event to fire on long button press. */
} button_event_cfg_t;

void button_evt_handler(uint8_t pin_no, uint8_t button_action);
void buttons_init(void);
void buttons_init2(void);
#endif

