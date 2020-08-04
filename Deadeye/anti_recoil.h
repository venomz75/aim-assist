#pragma once
#include <windows.h>
#include "vector3.h"

namespace anti_recoil
{
	void removeScreenshake(uintptr_t ptrLocalPlayer , uintptr_t ptrRecoilPitch, uintptr_t ptrRecoilYaw, uintptr_t ptrRecoilRoll, uintptr_t ptrDrunkness);
}