#include "bunnyhop.h"

void bhop::bunnyHop(uintptr_t ptrForceJump, uintptr_t ptrFlags)
{
	if (*(int*)(ptrForceJump) && (ptrFlags) && *(int*)(ptrFlags) == 257)
	{
		*(int*)(ptrForceJump) = 6;
	}
	/*
	else
	{
		*(int*)(ptrForceJump) = 4;
	}
	*/
}