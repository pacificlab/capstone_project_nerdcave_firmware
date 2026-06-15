/*
 * store_flash_data.h
 *
 *  Created on: Jan 27, 2026
 *      Author: Whath
 */

#ifndef INC_STORE_FLASH_DATA_H_
#define INC_STORE_FLASH_DATA_H_

#include "stdio.h"
#include "main.h"

void store_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length);
void read_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length);
void parse_for_profile(uint32_t id_num, uint32_t memory_address, uint16_t data_length);
//void store_profile_data();

#endif /* INC_STORE_FLASH_DATA_H_ */
