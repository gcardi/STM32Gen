/*
 * StorageData.cpp
 *
 */

#include <cstring>
#include <cstdio>

#include "main.h"
#include "cmsis_os.h"
#include "StorageData.h"


/*
extern "C" {
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
}
*/

#define EEPROM_MAGIC  0x5C
#define AT24C16_ADDR  (0x50 << 1)
#define STORAGE_BASE_ADDR 0x10

struct __attribute__((packed)) eeprom_record_t
{
    uint8_t  magic_;   // 0xA5
    uint8_t  ampl_;    // 0..100
    uint16_t freq_;    // 10..20000 (LE)
    uint8_t  crc_;
};

static inline uint16_t read_u16( const void *p )
{
    uint16_t v;
    memcpy(&v, p, sizeof(v));
    return v;
}
//------------------------------------------------------

static uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
    }
    return crc;
}
//------------------------------------------------------

static uint8_t calc_crc(const eeprom_record_t *rec)
{
    return crc8(&rec->ampl_, 3);
}
//------------------------------------------------------

void WriteDataToEEPROM( uint8_t Ampl, uint16_t Freq )
{
	eeprom_record_t rec;

	rec.magic_  = EEPROM_MAGIC;
	rec.ampl_ = Ampl;

	memcpy( &rec.freq_, &Freq, sizeof rec.freq_ );

	rec.crc_ = calc_crc( &rec );

	HAL_I2C_Mem_Write(
		&hi2c3,
		AT24C16_ADDR,
		STORAGE_BASE_ADDR,
		I2C_MEMADD_SIZE_8BIT,
		reinterpret_cast<uint8_t *>( &rec ),
		sizeof rec,
		HAL_MAX_DELAY
	);

	vTaskDelay( pdMS_TO_TICKS( 5 ) );
}
//------------------------------------------------------

bool ReadDataFromEEPROM( uint8_t& Ampl, uint16_t& Freq )
{
	eeprom_record_t rec;

	HAL_I2C_Mem_Read(
	    &hi2c3,
		AT24C16_ADDR,
		STORAGE_BASE_ADDR,
	    I2C_MEMADD_SIZE_8BIT,
		reinterpret_cast<uint8_t *>( &rec ),
	    sizeof rec,
	    HAL_MAX_DELAY
	);

	// verifica magic
	if (rec.magic_ != EEPROM_MAGIC) {
printf( "EEPROM Error bad EEPROM_MAGIC\r\n" );
	    return false;
	}

	// estrazione sicura
	uint16_t TempFreq = read_u16( &rec.freq_ );

	// range check
	if ( rec.ampl_ > 100 || TempFreq < 10 || TempFreq >  hi2s2.Init.AudioFreq / 2 ) {
printf( "EEPROM Error data is out of range\r\n" );
	    return false;
	}

	// CRC
	if ( rec.crc_ != calc_crc( &rec ) ) {
printf( "EEPROM Error bad CRC\r\n" );
	    return false;
	}

	Ampl =  rec.ampl_;
	Freq = TempFreq;

	return true;
}
//------------------------------------------------------



