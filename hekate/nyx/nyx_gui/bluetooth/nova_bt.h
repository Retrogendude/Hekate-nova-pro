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
} nova_bt_pad;
enum { NOVA_BT_OFF, NOVA_BT_STARTING, NOVA_BT_READY, NOVA_BT_ERROR };
typedef struct {
    int state, error;
    uint32_t rx_packets, tx_packets;
    nova_bt_pad pad[2];
} nova_bt_status;
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
