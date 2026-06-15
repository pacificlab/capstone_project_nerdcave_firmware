/*
 * desk_control.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Whath
 */

#include "stdint.h"
#include "stm32l4xx_it.h"
#include "stdio.h"
#include "stdlib.h"

int upliftDecoder (uint8_t* b)
{
	uint8_t ordered[4];
	int start = 0;
	int result = 0;
	for (int i = 0; i < 4; i++)
	{
		if (b[i] == 0b00000001 && b[(i+1) % 4] == 0b00000001 &&  (b[(i+2) % 4] & 0b11111110) == 0)
		{
			start = i;
			break;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		ordered[i] = b[(i+start) % 4];
	}

	result = ordered[2] << 8 | ordered[3];
	return result;
}

//Profile Stuff (for now)
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

struct profiles* addProfile(uint64_t pid, int *profile)
{
	struct profiles *currProfile = malloc(sizeof(struct profiles));
	currProfile->UID_Card = pid;
	currProfile->Desk_Height = profile[0];
	currProfile->LED_Red = profile[1];
	currProfile->LED_Green = profile[2];
	currProfile->LED_Blue = profile[3];
	currProfile->Relay_Preset = profile[4];
	currProfile->next = NULL;
	return currProfile;
}

struct profiles* addProfiles(uint64_t* pids, int pdata[][5], int numOfIDs)
{
	//Some very basic profile code. Eventually profile handling will be handled with IoT
	// so until then this crap will do.
	struct profiles *head = NULL;
	struct profiles *tail = NULL;
	for (int i = 0; i < numOfIDs; i++)
	{
		struct profiles *newNode = addProfile(pids[i], pdata[i]);
		if (head == NULL)
		{
			head = newNode;
			tail = newNode;
		}
		else
		{
			tail->next = newNode;
			tail = newNode;
		}
	}
	return head;
}


