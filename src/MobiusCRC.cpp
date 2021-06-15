/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#include "MobiusCRC.h"

/*!
 * @brief Generates a 16 bit CRC.
 *
 * @param data bytes for generating the check
 * @param length size the byte array
 * @return 16 bit CRC value
 */
uint16_t MobiusCRC::crc16(uint8_t* data, int length) {
    uint16_t crc16 = -1;
    for (int i = 0; i < length; i++) {
        uint8_t dex = (data[i] ^ ((uint8_t)(crc16 >> 8))) & 0xff;
        crc16 = (uint16_t)((crc16 << 8) ^ Mobius::CRC16_TABLE[dex]);
    }
    return crc16;
}
