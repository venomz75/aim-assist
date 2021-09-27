#include "anti_recoil.h"

void anti_recoil::removeScreenshake(uintptr_t ptrLocalPlayer, uintptr_t ptrRecoilPitch, uintptr_t ptrRecoilYaw, uintptr_t ptrRecoilRoll, uintptr_t ptrScreenshake)
{
	*(float*)(ptrLocalPlayer + ptrRecoilPitch) = 0;
	*(float*)(ptrLocalPlayer + ptrRecoilYaw) = 0;
	*(float*)(ptrLocalPlayer + ptrRecoilRoll) = 0;
	*(float*)(ptrLocalPlayer + ptrScreenshake) = 0;
}
