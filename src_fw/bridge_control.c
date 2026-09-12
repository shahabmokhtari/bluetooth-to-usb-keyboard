#include "bridge_control.h"

#include <stdio.h>
#include <string.h>

#include "Common.h"
#include "tusb.h"

#define CONTROL_PROTOCOL_VERSION 1
#define CONTROL_LINE_SIZE 64
#define CONTROL_MESSAGE_SIZE 128

typedef struct {
    bridge_state_t state;
    uint32_t passkey;
    uint32_t revision;
    bool has_passkey;
    bool pair_new_requested;
} bridge_control_state_t;

static bridge_control_state_t control_state;
static char receive_line[CONTROL_LINE_SIZE];
static size_t receive_length;
static uint32_t sent_revision;
static bool was_connected;
static bool hello_pending;
static bool acknowledgement_pending;

static const char *bridge_state_name(bridge_state_t state)
{
    switch (state) {
        case BRIDGE_STATE_STARTING:     return "starting";
        case BRIDGE_STATE_SCANNING:     return "scanning";
        case BRIDGE_STATE_CONNECTING:   return "connecting";
        case BRIDGE_STATE_PAIRING:      return "pairing";
        case BRIDGE_STATE_DISCOVERING:  return "discovering";
        case BRIDGE_STATE_READY:        return "ready";
        case BRIDGE_STATE_DISCONNECTED: return "disconnected";
        case BRIDGE_STATE_FORGETTING:   return "forgetting";
        default:                        return "unknown";
    }
}

static bool write_message(const char *message)
{
    size_t length = strlen(message);
    if (tud_cdc_write_available() < length) {
        return false;
    }

    if (tud_cdc_write(message, (uint32_t)length) != length) {
        return false;
    }

    tud_cdc_write_flush();
    return true;
}

static void process_command(const char *command)
{
    if (strcmp(command, "STATUS") == 0) {
        sent_revision = UINT32_MAX;
        return;
    }

    if (strcmp(command, "PAIR_NEW") == 0) {
        CMN_EntrySpinLock();
        control_state.pair_new_requested = true;
        CMN_ExitSpinLock();
        acknowledgement_pending = true;
    }
}

static void receive_commands(void)
{
    while (tud_cdc_available()) {
        char character;
        if (tud_cdc_read(&character, 1) != 1) {
            return;
        }

        if (character == '\n') {
            receive_line[receive_length] = '\0';
            if (receive_length > 0 && receive_line[receive_length - 1] == '\r') {
                receive_line[--receive_length] = '\0';
            }
            process_command(receive_line);
            receive_length = 0;
            continue;
        }

        if (receive_length + 1 < sizeof(receive_line)) {
            receive_line[receive_length++] = character;
        } else {
            receive_length = 0;
        }
    }
}

void bridge_control_init(void)
{
    control_state.state = BRIDGE_STATE_STARTING;
    control_state.revision = 1;
    sent_revision = UINT32_MAX;
}

void bridge_control_task(void)
{
    bool connected = tud_cdc_connected();
    if (!connected) {
        was_connected = false;
        receive_length = 0;
        return;
    }

    if (!was_connected) {
        was_connected = true;
        hello_pending = true;
        sent_revision = UINT32_MAX;
    }

    receive_commands();

    if (hello_pending) {
        char hello_message[CONTROL_LINE_SIZE];
        snprintf(hello_message, sizeof(hello_message),
                 "{\"type\":\"hello\",\"protocol\":%d}\n", CONTROL_PROTOCOL_VERSION);
        if (!write_message(hello_message)) {
            return;
        }
        hello_pending = false;
    }

    if (acknowledgement_pending) {
        if (!write_message("{\"type\":\"ack\",\"command\":\"pair_new\"}\n")) {
            return;
        }
        acknowledgement_pending = false;
    }

    bridge_control_state_t snapshot;
    CMN_EntrySpinLock();
    snapshot = control_state;
    CMN_ExitSpinLock();

    if (snapshot.revision == sent_revision) {
        return;
    }

    char message[CONTROL_MESSAGE_SIZE];
    int length;
    if (snapshot.has_passkey) {
        length = snprintf(message, sizeof(message),
                          "{\"type\":\"status\",\"state\":\"%s\",\"passkey\":\"%06lu\"}\n",
                          bridge_state_name(snapshot.state),
                          (unsigned long)snapshot.passkey);
    } else {
        length = snprintf(message, sizeof(message),
                          "{\"type\":\"status\",\"state\":\"%s\"}\n",
                          bridge_state_name(snapshot.state));
    }

    if (length > 0 && (size_t)length < sizeof(message) && write_message(message)) {
        sent_revision = snapshot.revision;
    }
}

void bridge_control_publish_state(bridge_state_t state)
{
    CMN_EntrySpinLock();
    if (control_state.state != state) {
        control_state.state = state;
        control_state.revision++;
    }
    CMN_ExitSpinLock();
}

void bridge_control_publish_passkey(uint32_t passkey)
{
    CMN_EntrySpinLock();
    control_state.passkey = passkey;
    control_state.has_passkey = true;
    control_state.revision++;
    CMN_ExitSpinLock();
}

void bridge_control_clear_passkey(void)
{
    CMN_EntrySpinLock();
    if (control_state.has_passkey) {
        control_state.has_passkey = false;
        control_state.revision++;
    }
    CMN_ExitSpinLock();
}

bool bridge_control_take_pair_new_request(void)
{
    CMN_EntrySpinLock();
    bool requested = control_state.pair_new_requested;
    control_state.pair_new_requested = false;
    CMN_ExitSpinLock();
    return requested;
}
