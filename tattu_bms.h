#ifndef BMS_TATTU_PROTOCOL_H
#define BMS_TATTU_PROTOCOL_H

/*
        *  *                   *  *
     *        *    2   1    *        *
    *  __|__   *   o   o   *    ___   *
    *    |     *   o   o   *          *
     *        *    4   3    *        *
        *  *                   *  *

 * 1 (White): CAN_H
 * 2 (Red): NULL (Tattu), Signal (OKCELL)
 * 3 (Black): GND
 * 4 (Yellow): CAN_L
 *
 */

// Do not use for other device. Do not use the same ID batteries on the same bus.
#define TATTU_CAN_EXT_ID          0x01109216
#define TATTU_PACK_BUFFER_SIZE    80

#define CAN_SOT_MASK    0x80
#define CAN_EOT_MASK    0x40
#define CAN_TOGGLE_MASK 0x20
#define CAN_TRANSFER_ID 0x1F

#include <Arduino.h>
#include <stdint.h>

typedef struct {
  uint16_t crc_received;
  uint16_t crc_calculated;
  uint16_t man_id;
  uint16_t sku_code;
  uint16_t vbatt;
  int16_t ibatt;
  uint16_t temp;
  uint16_t rem_capa_percent;
  uint16_t cycle_life;
  uint16_t health_status;
  uint16_t vcell[14];
  uint16_t std_capa_mah;
  uint16_t rem_capa_mah;
  uint32_t error_info;
} TattuData_t;

uint16_t CCITT_CRC16Init(const uint8_t* bytes, uint16_t len);

#endif	/* BMS_TATTU_PROTOCOL_H */
