#include "fov_changer.h"

void fov::setFOV(uintptr_t ptrFOV, int iDesiredFOV)
{
	*(int*)(ptrFOV) = iDesiredFOV;
}