#include <Arduino.h>
#include <SPI.h>
#include <mcp2515_can.h>
#include <stdarg.h>
#include "tattu_bms.h"

#ifdef ARDUINO_AVR_MEGA2560
#define USE_OLED_DISPLAY
#elif defined(ARDUINO_AVR_UNO)
#elif defined(ARDUINO_AVR_NANO)
#else
#endif

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

const char* str_line_separator = "----------------------------------%u";

const char* str_can_init_fail = "CAN Init Fail! Retry...";
const char* str_can_init_ok = "CAN Init OK";

const char* str_warn_low_temp = "Low Temp";
const char* str_warn_over_temp = "Over Temp";
const char* str_warn_over_curr_charging = "Over Curr Charging";
const char* str_warn_over_curr_disharging = "Over Curr DisCharging";
const char* str_warn_vbat_low = "VBatt is LOW";
const char* str_warn_vbat_high = "VBatt is HIGH";
const char* str_warn_vcell_diff_high = "VCell Difference is HIGH";
const char* str_warn_vcell_high = "VCell is HIGH";
const char* str_warn_vcell_low = "VCell is LOW";
const char* str_warn_short_circuit_charging = "Short Circuit Charging";
const char* str_warn_short_circuit_discharging = "Short Circuit DisCharging";
const char* str_warn_rem_capa_low = "Remaining Capacity is LOW";
const char* str_warn_clone_charger = "Using Clone Charger";

const char* str_man_id_sku_code = "ManID: %04X SKU: %04X";
const char* str_vi_capa_temp = "V: %02u.%02uV I: %04d.%01uACapa: %03d%% Temp: %02dC";
const char* str_rem_capa = "Rem Capa: %dmAh";
const char* str_std_capa = "Std Capa: %dmAh";
const char* str_cycle_health = "Cycle: %d Health: %d%%";
const char* str_ind_vcell = "VCell[%02d]: %umV";
const char* str_crc_info = "CRC_R: 0x%04X, CRC_C: 0x%04X";
const char* str_crc_error = "CRC Error!";

uint8_t* msgBuffer = NULL;
uint8_t dataAvailable = 0;

mcp2515_can CAN(SPI_CS_PIN);  // Set CS pin

#ifdef USE_OLED_DISPLAY
#include <GyverOLED.h>
GyverOLED<SSH1106_128x64, OLED_BUFFER> oled;
#endif

void oled_println(char* str);
void log_println(const char* message);
const char* log_printf(const char* format, ...);
TattuData_t* ParseData(uint8_t* buffer, int len);
void printPackData(TattuData_t* tattuData);
void parseErrorMessage(uint32_t error_info);

void setup() {
  Serial.begin(115200);
  while (!SERIAL_PORT_MONITOR)
    ;  // wait for serial port to connect. Needed for native USB port only

  dataAvailable = 0;

  while (CAN_OK != CAN.begin(CAN_1000KBPS)) {  // init can bus : baudrate = 500k
    log_println(str_can_init_fail);
    delay(1000);
  }
  log_println(str_can_init_ok);

#ifdef USE_OLED_DISPLAY
  oled.init();
  oled.autoPrintln(true);
  oled.setScale(2);
#endif

  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING);  // start interrupt
}

void MCP2515_ISR() {
  static uint8_t dataBuffer1[TATTU_PACK_BUFFER_SIZE];
  static uint8_t dataBuffer2[TATTU_PACK_BUFFER_SIZE];
  static int index = 0;
  static uint8_t* rxBuffer = dataBuffer1;

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    uint8_t len;
    CAN.readMsgBuf(&len, &rxBuffer[index]);
    if (len == 8) {
      uint8_t tail = rxBuffer[index + 7];
      index += len - 1; // discard tail byte for CRC Calculation!
      if (((tail & CAN_EOT_MASK) == CAN_EOT_MASK) || (index >= TATTU_PACK_BUFFER_SIZE)) {  // Check End Of Transfer
        msgBuffer = rxBuffer;
        rxBuffer = (rxBuffer == dataBuffer1) ? dataBuffer2 : dataBuffer1;  // swap buffers
        dataAvailable = index; // save data count
        index = 0;
      }
    }
  }
}

void loop() {
  static uint32_t dataCounter = 0;
  if (dataAvailable > 0 && msgBuffer != NULL) {
    TattuData_t* tattuData = ParseData(msgBuffer, dataAvailable);
    printPackData(tattuData);
    dataAvailable = 0;
    log_printf(str_line_separator, ++dataCounter);
  }
  delay(50);  // reduce idle power
}

#define pack2Byte(b) (*b++ | (*b++ << 8))
#define pack4Byte(b) (*b++ | (*b++ << 8) | (*b++ << 8) | (*b++ << 8))

TattuData_t* ParseData(uint8_t* buffer, int len) {
  static TattuData_t tattuData;

  tattuData.crc_received = pack2Byte(buffer);
  tattuData.crc_calculated = CCITT_CRC16Init(buffer, len - 2);  // discard first two bytes, containing received CRC

  if (tattuData.crc_received == tattuData.crc_calculated) {
    tattuData.man_id = pack2Byte(buffer);
    tattuData.sku_code = pack2Byte(buffer);
    tattuData.vbatt = pack2Byte(buffer);
    tattuData.ibatt = pack2Byte(buffer);
    tattuData.temp = pack2Byte(buffer);
    tattuData.rem_capa_percent = pack2Byte(buffer);
    tattuData.cycle_life = pack2Byte(buffer);
    tattuData.health_status = pack2Byte(buffer);

    tattuData.vcell[0] = pack2Byte(buffer);
    tattuData.vcell[1] = pack2Byte(buffer);
    tattuData.vcell[2] = pack2Byte(buffer);
    tattuData.vcell[3] = pack2Byte(buffer);
    tattuData.vcell[4] = pack2Byte(buffer);
    tattuData.vcell[5] = pack2Byte(buffer);
    tattuData.vcell[6] = pack2Byte(buffer);
    tattuData.vcell[7] = pack2Byte(buffer);
    tattuData.vcell[8] = pack2Byte(buffer);
    tattuData.vcell[9] = pack2Byte(buffer);
    tattuData.vcell[10] = pack2Byte(buffer);
    tattuData.vcell[11] = pack2Byte(buffer);

    // The following two cells' data may not be present on 12S LiPO!
    tattuData.vcell[12] = pack2Byte(buffer);
    tattuData.vcell[13] = pack2Byte(buffer);

    tattuData.std_capa_mah = pack2Byte(buffer);
    tattuData.rem_capa_mah = pack2Byte(buffer);

    tattuData.error_info = pack4Byte(buffer);
  }

  return &tattuData;
}

void printPackData(TattuData_t* tattuData) {
  log_printf(str_crc_info, tattuData->crc_received, tattuData->crc_calculated);

  if (tattuData->crc_received != tattuData->crc_calculated) {
    log_println(str_crc_error);
    oled_println(str_crc_error);
    return;
  }

  log_printf(str_man_id_sku_code, tattuData->man_id, tattuData->sku_code);

  tattuData->vbatt /= 10;
  uint16_t voltage_int_part = tattuData->vbatt / 100;
  uint16_t voltage_remainder = tattuData->vbatt % 100;

  tattuData->ibatt /= 10;
  int16_t current_int_part = tattuData->ibatt / 10;  // Current is reported as x10mA
  uint16_t current_remainder = tattuData->ibatt % 10;

  char* txBuffer = log_printf(str_vi_capa_temp,
                              voltage_int_part, voltage_remainder,
                              current_int_part, current_remainder,
                              tattuData->rem_capa_percent, tattuData->temp);

  oled_println(txBuffer);

  log_printf(str_rem_capa, tattuData->rem_capa_mah);
  log_printf(str_std_capa, tattuData->std_capa_mah);
  log_printf(str_cycle_health, tattuData->cycle_life, tattuData->health_status);

  for (int i = 0; i < 14; i++) {
    log_printf(str_ind_vcell, i + 1, tattuData->vcell[i]);
  }

  parseErrorMessage(tattuData->error_info);
}

void parseErrorMessage(uint32_t error_info) {
  error_info &= 0x00000FFF;  // Bit0 - Bit12 are Valid

  if (error_info == 0) {
    return;
  }

  if (error_info & 0x0001) {
    log_println(str_warn_low_temp);
  } else if (error_info & 0x0002) {
    log_println(str_warn_over_temp);
  } else if (error_info & 0x0004) {
    log_println(str_warn_over_curr_charging);
  } else if (error_info & 0x0008) {
    log_println(str_warn_over_curr_disharging);
  } else if (error_info & 0x00010) {
    log_println(str_warn_vbat_low);
  } else if (error_info & 0x00020) {
    log_println(str_warn_vbat_high);
  } else if (error_info & 0x00040) {
    log_println(str_warn_vcell_diff_high);
  } else if (error_info & 0x00080) {
    log_println(str_warn_vcell_high);
  } else if (error_info & 0x00100) {
    log_println(str_warn_vcell_low);
  } else if (error_info & 0x00200) {
    log_println(str_warn_short_circuit_charging);
  } else if (error_info & 0x00400) {
    log_println(str_warn_short_circuit_discharging);
  } else if (error_info & 0x00800) {
    log_println(str_warn_rem_capa_low);
  } else if (error_info & 0x01000) {
    log_println(str_warn_clone_charger);
  }
}

void oled_println(char* str) {
#ifdef USE_OLED_DISPLAY
  oled.clear();
  oled.home();
  oled.println(str);
  oled.update();
#endif
}

void log_println(const char* message) {
  Serial.println(message);
}

const char* log_printf(const char* format, ...) {
  static char txBuffer[64];
  va_list args;
  va_start(args, format);
  vsprintf(txBuffer, format, args);
  va_end(args);
  log_println(txBuffer);

  return txBuffer;
}
