/*
 * store_flash_data.c
 *
 *  Created on: Jan 27, 2026
 *      Author: Whath
 *      https://www.youtube.com/watch?v=E4FWkNUyQYg
 */

#include "store_flash_data.h"
#include "stdint.h"
#include "stdlib.h"
#include "stm32l4xx_it.h"
#include "string.h"
#include "card_rfid.h"


typedef uint64_t flash_datatype;
#define DATA_SIZE sizeof (flash_datatype)

void store_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length)
{
	uint8_t double_word_data[DATA_SIZE];
	FLASH_EraseInitTypeDef flash_erase_struct = {0};
	HAL_FLASH_Unlock();
	// defining the members of a struct
	flash_erase_struct.TypeErase = FLASH_TYPEERASE_PAGES;
	// defining an onset number of pages to be erased
	flash_erase_struct.Page = (memory_address - FLASH_BASE) / FLASH_PAGE_SIZE;
	// number of pages to remove
	flash_erase_struct.NbPages = 1 + data_length / FLASH_PAGE_SIZE;
	// identify the flash bank
	if(memory_address > FLASH_BANK1_END && memory_address < FLASH_BANK2_END)
	{
		flash_erase_struct.Banks = FLASH_BANK_2;
	}
	else if(memory_address > FLASH_BASE && memory_address < FLASH_BANK1_END)
	{
		flash_erase_struct.Banks = FLASH_BANK_1;
	}
	else
	{
		printf("illegal memory address \n");
		UsageFault_Handler();
	}
	uint32_t error_status = 0;

	//erase pages
	HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
	int i = 0;
	// using while loop, convey all data to the flash memory
	while(i <= data_length)
	{
		double_word_data[i % DATA_SIZE] = data[i];
		i++;
		if (i % DATA_SIZE == 0)
		{
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memory_address + i - DATA_SIZE, *((uint64_t *)double_word_data));
		}
	}
	// convey data if something is left
	if (i % DATA_SIZE != 0)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memory_address + i - i % DATA_SIZE, *((flash_datatype *)double_word_data));
	}
	// lock the memory
	HAL_FLASH_Lock();
}

void read_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length)
{
	for(int i = 0; i < data_length; i++)
	{
		*(data + i) = (*(uint8_t *)(memory_address + i));
	}
}


void parse_for_profile(uint32_t id_num, uint32_t memory_address, uint16_t data_length)
{
	//TODO, used after read_flash_memory
	//Step 1: tokenize data
	//Step 2: Check to see if id matches
	//Step 2a: Matches -> Load data, lights blink green
	//Step 2b: Doesn't match -> Check next profile
	//Step 3: No matching profile -> Lights blink red
	char char_read[data_length];
	read_flash_memory(memory_address, (uint8_t *)char_read, data_length);
	char delim[] = ", ";
	char *token;
	int match = 0;
	token = strtok(char_read, delim);
	while (1)
	{
			if (id_num == atoi(token))
			{
				printf("ID found match, loading data, blink green \n");
				valid_id();
				//TODO: Then load data
				match = 1;
				break;
			}
			else if (token == NULL)
			{
				//End of string, break
				break;
			}
			//TODO: Else increment until match, for now, break
			break;
	}
	if (match == 0)
	{
		//No match found
		printf("No matching ID found, blink red \n");
		invalid_id();
	}


}




