#ifndef BRIDGE_CONTROL_H
#define BRIDGE_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BRIDGE_STATE_STARTING = 0,
    BRIDGE_STATE_SCANNING,
    BRIDGE_STATE_CONNECTING,
    BRIDGE_STATE_PAIRING,
    BRIDGE_STATE_DISCOVERING,
    BRIDGE_STATE_READY,
    BRIDGE_STATE_DISCONNECTED,
    BRIDGE_STATE_FORGETTING,
} bridge_state_t;

void bridge_control_init(void);
void bridge_control_task(void);
void bridge_control_publish_state(bridge_state_t state);
void bridge_control_publish_passkey(uint32_t passkey);
void bridge_control_clear_passkey(void);
bool bridge_control_take_pair_new_request(void);

#endif
