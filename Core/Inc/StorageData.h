/*
 * StorageData.h
 *
 */

#ifndef INC_STORAGEDATA_H_
#define INC_STORAGEDATA_H_

#include <stdint.h>

extern void WriteDataToEEPROM( uint8_t Ampl, uint16_t Freq );
extern bool ReadDataFromEEPROM( uint8_t& Ampl, uint16_t& Freq );


#endif /* INC_STORAGEDATA_H_ */
