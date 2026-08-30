#ifndef W25Q16_H
#define W25Q16_H

#include "stm32f4xx.h"

/* =====================================================
 * CS
 * ===================================================== */

#define W25Q16_CS_PORT       GPIOB
#define W25Q16_CS_PIN        0

#define W25Q16_CS_LOW() (W25Q16_CS_PORT->BSRR = (1U << (W25Q16_CS_PIN + 16)))

#define W25Q16_CS_HIGH() (W25Q16_CS_PORT->BSRR = (1U << W25Q16_CS_PIN))


/* =====================================================
 * Commands
 * ===================================================== */

#define W25Q16_CMD_WRITE_ENABLE       0x06
#define W25Q16_CMD_WRITE_DISABLE      0x04
#define W25Q16_CMD_READ_STATUS1       0x05
#define W25Q16_CMD_READ_ID            0x9F
#define W25Q16_CMD_READ_DATA          0x03
#define W25Q16_CMD_PAGE_PROGRAM       0x02
#define W25Q16_CMD_SECTOR_ERASE       0x20
#define W25Q16_CMD_BLOCK_ERASE_32K    0x52
#define W25Q16_CMD_BLOCK_ERASE_64K    0xD8
#define W25Q16_CMD_CHIP_ERASE         0xC7


/* =====================================================
 * Status bits
 * ===================================================== */

#define W25Q16_STATUS_BUSY            0x01
#define W25Q16_STATUS_WEL             0x02


/* =====================================================
 * Functions
 * ===================================================== */

void SPI1_Init(void);

uint32_t W25Q16_ReadID(void);
void W25Q16_ReadData(uint32_t address, uint8_t *buffer, uint32_t length);
void W25Q16_EraseSector(uint32_t address);
void W25Q16_PageProgram(uint32_t address, uint8_t *buffer, uint16_t length);
void W25Q16_Write(uint32_t address, uint8_t *buffer, uint32_t length);
void W25Q16_EraseBlock(uint32_t address);
void W25Q16_ChipErase(void);

void W25Q16_WriteByte(uint32_t address, uint8_t data);
uint8_t W25Q16_ReadByte(uint32_t address);
void W25Q16_WriteFloat(uint32_t address,float data);
float W25Q16_ReadFloat(uint32_t address);

#endif
