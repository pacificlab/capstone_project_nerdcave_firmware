/*
 * desk_control.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Whath
 */

#ifndef INC_DESK_CONTROL_H_
#define INC_DESK_CONTROL_H_

#include "stdio.h"

int upliftDecoder (uint8_t* b);
struct profiles
{
	unsigned long long UID_Card;
	uint16_t Desk_Height;
	uint8_t LED_Red;
	uint8_t LED_Green;
	uint8_t LED_Blue;
	uint8_t Relay_Preset;
	struct profiles *next;
};
struct profiles* addProfile(uint64_t pid, int *profile);
struct profiles* addProfiles(uint64_t* pids, int pdata[][5], int numOfIDs);

#endif /* INC_DESK_CONTROL_H_ */
