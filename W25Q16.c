#include "W25Q16.h"

void SPI1_Init(void)
{
    /* =================================================
     * 1. Enable GPIOB clock
     * ================================================= */
    RCC->AHB1ENR |= (1U << 1);

    /* =================================================
     * 2. Enable SPI1 clock
     * ================================================= */
    RCC->APB2ENR |= (1U << 12);

    /* =================================================
     * 4. PB3, PB4, PB5
     *
     * PB3 -> SCK
     * PB4 -> MISO
     * PB5 -> MOSI
     *
     * Alternate Function = AF5
     * ================================================= */
    GPIOB->MODER &= ~( (3U << 6)|(3U << 8)|(3U << 10));
    GPIOB->MODER |= ((2U << 6)|(2U << 8)|(2U << 10));

    /* =================================================
     * No Pull
     * ================================================= */
    GPIOB->PUPDR &= ~((3U << 6)|(3U << 8)|(3U << 10));

    /* =================================================
     * Very High Speed
     * ================================================= */
    GPIOB->OSPEEDR &= ~((3U << 6)|(3U << 8)|(3U << 10));
    GPIOB->OSPEEDR |= ((3U << 6)|(3U << 8)|(3U << 10));

    /* =================================================
     * AF5
     *
     * PB3 -> AFRL bits 12-15
     * PB4 -> AFRL bits 16-19
     * PB5 -> AFRL bits 20-23
     * ================================================= */
    GPIOB->AFR[0] &= ~((0xFU << 12)|(0xFU << 16)|(0xFU << 20));
    GPIOB->AFR[0] |= ((5U << 12)|(5U << 16)|(5U << 20));

    /* =================================================
     * 5. PB0 -> CS
     * ================================================= */
    GPIOB->MODER &= ~(3U << 0);
    GPIOB->MODER |=  (1U << 0);

    /* Pull-up */
    GPIOB->PUPDR &= ~(3U << 0);
    GPIOB->PUPDR |=  (1U << 0);

    /* Very High Speed */
    GPIOB->OSPEEDR &= ~(3U << 0);
    GPIOB->OSPEEDR |=  (3U << 0);

    /* CS HIGH */
    GPIOB->BSRR = (1U << 0);

    /* =================================================
     * 6. Reset SPI1
     * ================================================= */
    RCC->APB2RSTR |= (1U << 12);
    RCC->APB2RSTR &= ~(1U << 12);

    /* =================================================
     * 7. Configure SPI1
     * ================================================= */
    SPI1->CR1 = 0;

    /* Master */
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |=SPI_CR1_BR_1 | SPI_CR1_BR_0;

    /*
     * SPI Mode 0
     *
     * CPOL = 0
     * CPHA = 0
     */

    /*
     * MSB first
     *
     * LSBFIRST = 0
     */

    /*
     * 8-bit data
     *
     * DFF = 0
     */

    /* Software NSS */
    SPI1->CR1 |= (1U << 9);

    /* Internal NSS = HIGH */
    SPI1->CR1 |= (1U << 8);

    /* Enable SPI */
    SPI1->CR1 |= (1U << 6);
}

uint8_t SPI1_TransmitReceive(uint8_t data)
{
    // Wait TX buffer empty
    while (!(SPI1->SR & (1U << 1)));

    // Send data
    SPI1->DR = data;

    // Wait receive data
    while (!(SPI1->SR & (1U << 0)));

    // Read data received
    return (uint8_t)SPI1->DR;
}

uint32_t W25Q16_ReadID(void)
{
    uint8_t manufacturer;
    uint8_t memory_type;
    uint8_t capacity;

    W25Q16_CS_LOW();

    SPI1_TransmitReceive(0x9F);

    manufacturer = SPI1_TransmitReceive(0xFF);
    memory_type  = SPI1_TransmitReceive(0xFF);
    capacity     = SPI1_TransmitReceive(0xFF);

    W25Q16_CS_HIGH();

    return (
        ((uint32_t)manufacturer << 16) |
        ((uint32_t)memory_type << 8) |
        capacity
    );
}

void W25Q16_WriteEnable(void)
{
    W25Q16_CS_LOW();

    SPI1_TransmitReceive(W25Q16_CMD_WRITE_ENABLE);

    while (SPI1->SR & (1U << 7));

    W25Q16_CS_HIGH();
}

void W25Q16_WriteDisable(void)
{
    W25Q16_CS_LOW();

    SPI1_TransmitReceive(W25Q16_CMD_WRITE_DISABLE);

    while (SPI1->SR & (1U << 7));

    W25Q16_CS_HIGH();
}

uint8_t W25Q16_ReadStatus1(void)
{
    uint8_t status;

    W25Q16_CS_LOW();

    SPI1_TransmitReceive(W25Q16_CMD_READ_STATUS1);

    status = SPI1_TransmitReceive(0xFF);

    while (SPI1->SR & (1U << 7));

    W25Q16_CS_HIGH();

    return status;
}

void W25Q16_WaitBusy(uint32_t timeout)
{
//    while (timeout--)
//    {
//        if (!(W25Q16_ReadStatus1() & W25Q16_STATUS_BUSY)) return 0;
//    }
//    return 1;
	while (W25Q16_ReadStatus1() & W25Q16_STATUS_BUSY); 
}

void W25Q16_ReadData(uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint32_t i;

    W25Q16_CS_LOW();

    /* Read Data command */
    SPI1_TransmitReceive(W25Q16_CMD_READ_DATA);

    /* Address byte 1 */
    SPI1_TransmitReceive((address >> 16) & 0xFF);

    /* Address byte 2 */
    SPI1_TransmitReceive((address >> 8) & 0xFF);

    /* Address byte 3 */
    SPI1_TransmitReceive(address & 0xFF);

    /* Read data */
    for (i = 0; i < length; i++)
    {
        buffer[i] = SPI1_TransmitReceive(0xFF);
    }

    while (SPI1->SR & (1U << 7));

    W25Q16_CS_HIGH();
}

void W25Q16_EraseSector(uint32_t address)
{
    /* Enable write */
    W25Q16_WriteEnable();

    /* CS LOW */
    W25Q16_CS_LOW();

    /* Sector Erase command */
    SPI1_TransmitReceive(W25Q16_CMD_SECTOR_ERASE);

    /* Address */
    SPI1_TransmitReceive((address >> 16) & 0xFF);
    SPI1_TransmitReceive((address >> 8) & 0xFF);
    SPI1_TransmitReceive(address & 0xFF);

    while (SPI1->SR & (1U << 7));

    /* CS HIGH */
    W25Q16_CS_HIGH();

    /* Wait Flash erase */
    W25Q16_WaitBusy(1000);
}

void W25Q16_PageProgram(uint32_t address, uint8_t *buffer, uint16_t length)
{
    uint16_t i;

    /* Must not write > 256 byte */
    if (length == 0 || length > 256)
    {
        return;
    }

    /*
     * Check data over egde
     *
     * address & 0xFF = position in page
     */
    if (((address & 0xFF) + length) > 256)
    {
        return;
    }

    /*
     * Flash must enable before
     */
    W25Q16_WriteEnable();

    /*
     * CS LOW
     */
    W25Q16_CS_LOW();

    /*
     * Page Program command
     */
    SPI1_TransmitReceive(W25Q16_CMD_PAGE_PROGRAM);

    /*
     * Send 24-bit address
     */
    SPI1_TransmitReceive((address >> 16) & 0xFF);
    SPI1_TransmitReceive((address >> 8) & 0xFF);
    SPI1_TransmitReceive(address & 0xFF);

    /*
     * Send data
     */
    for (i = 0; i < length; i++)
    {
        SPI1_TransmitReceive(buffer[i]);
    }

    /*
     * Wait SPI send complete
     */
    while (SPI1->SR & (1U << 7));

    /*
     * Stop transaction
     */
    W25Q16_CS_HIGH();

    /*
     * Wait flash write complete
     */
    W25Q16_WaitBusy(1000);
}

void W25Q16_Write(uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint32_t page_remain;
    uint32_t write_length;

    while (length > 0)
    {
        /*
         * Number of byte available in page
         */
        page_remain = 256 - (address & 0xFF);

        /*
         * Limit number of byte
         */
        if (length < page_remain)
        {
            write_length = length;
        }
        else
        {
            write_length = page_remain;
        }

        /*
         * Write one page
         */
        W25Q16_PageProgram(address, buffer, write_length);

        /*
         * Updata address
         */
        address += write_length;

        /*
         * update buffer
         */
        buffer += write_length;

        /*
         * Update number of byte
         */
        length -= write_length;
    }
}

void W25Q16_EraseBlock(uint32_t address)
{
    /* Write Enable */
    W25Q16_WriteEnable();

    /* CS LOW */
    W25Q16_CS_LOW();

    /* Block Erase 64KB */
    SPI1_TransmitReceive(W25Q16_CMD_BLOCK_ERASE_64K);

    /* 24-bit address */
    SPI1_TransmitReceive((address >> 16) & 0xFF);
    SPI1_TransmitReceive((address >> 8) & 0xFF);
    SPI1_TransmitReceive(address & 0xFF);

    /* Wait SPI transmission */
    while (SPI1->SR & (1U << 7));

    /* CS HIGH */
    W25Q16_CS_HIGH();

    /* Wait Flash erase */
    W25Q16_WaitBusy(1000);
}

void W25Q16_ChipErase(void)
{
    /* Write Enable */
    W25Q16_WriteEnable();

    /* CS LOW */
    W25Q16_CS_LOW();

    /* Chip Erase */
    SPI1_TransmitReceive(W25Q16_CMD_CHIP_ERASE);

    /* Wait SPI */
    while (SPI1->SR & (1U << 7));

    /* CS HIGH */
    W25Q16_CS_HIGH();

    /* Wait entire Flash erase */
    W25Q16_WaitBusy(1000);
}

void W25Q16_WriteByte(uint32_t address, uint8_t data)
{
    W25Q16_Write(address, &data, 1);
}

uint8_t W25Q16_ReadByte(uint32_t address)
{
    uint8_t data;

    W25Q16_ReadData(
        address,
        &data,
        1
    );

    return data;
}

void W25Q16_WriteFloat(uint32_t address, float data)
{
    W25Q16_Write(address, (uint8_t *)&data, sizeof(float));
}

float W25Q16_ReadFloat(uint32_t address)
{
    float data;

    W25Q16_ReadData(address, (uint8_t *)&data, sizeof(float));

    return data;
}