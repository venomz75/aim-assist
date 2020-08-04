#include "bunnyhop.h"

void bhop::bunnyHop(uintptr_t ptrForceJump, uintptr_t ptrFlags)
{
	if (*(int*)(ptrForceJump) && *(int*)(ptrFlags) && *(int*)(ptrFlags) == 257)
	{
		*(int*)(ptrForceJump) = 6;
	}
}