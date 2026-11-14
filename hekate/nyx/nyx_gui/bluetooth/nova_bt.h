#ifndef NOVA_BT_H
#define NOVA_BT_H
#include <stdint.h>
/* Fixed-width ABI: BDK and BTstack use different definitions of bool. */
typedef struct {
    uint8_t valid, mac[6], host[6], key[16]; /* key in HCI wire order */
} nova_bt_peer;
typedef struct {
    uint32_t buttons, updated;
    uint16_t x, y;
    uint8_t connected, battery;
    uint8_t report_id, mode_ack, hid_open, last_error;
    uint32_t reports;
    uint16_t last_psm;
    uint8_t stage, auth_error;
} nova_bt_pad;
enum { NOVA_BT_OFF, NOVA_BT_STARTING, NOVA_BT_READY, NOVA_BT_ERROR };
typedef struct {
    int state, error;
    uint8_t host_identity; /* 0 unknown, 1 controller matches imported host, 2 mismatch */
    uint32_t rx_packets, tx_packets;
    uint32_t rx_acl, tx_acl;
    nova_bt_pad pad[2];
} nova_bt_status;
typedef struct {uint32_t ms;uint16_t kind,a,b,c;} nova_bt_trace;
unsigned nova_bt_trace_count(void);
int nova_bt_trace_get(unsigned index,nova_bt_trace *record);
/* Metadata only: never put addresses, link keys or packet payloads here. */
void nova_bt_trace_reset(void);
void nova_bt_trace_add(uint16_t kind,uint16_t a,uint16_t b,uint16_t c);
int nova_bt_start(const nova_bt_peer peers[2]);
void nova_bt_stop(void);
void nova_bt_poll(void);
const nova_bt_status *nova_bt_get_status(void);
/* Pure protocol helpers are exercised on the host as well as on ARM. */
int nova_bt_decode(nova_bt_pad *pad, unsigned side, const uint8_t *report, unsigned len, uint32_t now);
int nova_bt_validate_peers(const nova_bt_peer peers[2]);
/* Radio operations never wait for a byte. Startup alone has a bounded POR. */
uint32_t nova_bt_now(void);
int nova_bt_radio_open(void);
void nova_bt_radio_close(void);
int nova_bt_radio_read(void); /* -1 empty, -2 UART fault */
int nova_bt_radio_write(uint8_t value); /* 0 backpressure */
#endif
