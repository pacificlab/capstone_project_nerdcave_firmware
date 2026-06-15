/*
 * card_rfid.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Whath
 */

#include "stdint.h"
#include "stm32l4xx_it.h"
#include "stdio.h"

void valid_id()
{
	//TODO
	//For now, print that the ID was valid, this will eventually be a green flash
	printf("*Blink green* *blink green*, valid ID! \n");
}


void invalid_id()
{
	//TODO
	//For now, print that the ID was invalid, this will eventually be a red flash
	printf("*Blink red* *blink red*, invalid ID! \n");
}

uint64_t buf_to_num(uint8_t* buffer)
{
    uint64_t ID = 0;

    for (size_t i = 0; i < 14; i++)
    {
        uint8_t val;

        if (buffer[i] >= '0' && buffer[i] <= '9')
            val = buffer[i] - '0';
        else if (buffer[i] >= 'A' && buffer[i] <= 'F')
            val = buffer[i] - 'A' + 10;
        else if (buffer[i] >= 'a' && buffer[i] <= 'f')
            val = buffer[i] - 'a' + 10;

        ID = (ID << 4) | val;
    }
	return ID;
}


