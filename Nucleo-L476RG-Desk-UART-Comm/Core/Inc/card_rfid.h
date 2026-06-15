/*
 * card_rfid.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Whath
 */

#ifndef INC_CARD_RFID_H_
#define INC_CARD_RFID_H_

#include "stdio.h"
#include "main.h"

void valid_id();
void invalid_id();
uint64_t buf_to_num(uint8_t* buffer);

#endif /* INC_CARD_RFID_H_ */
