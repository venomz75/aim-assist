#pragma once
#include <windows.h>
#include "vector3.h"

namespace aimbot
{
	Vec3 calculateAngles(Vec3 vecCameraPos, Vec3 vecTargetPos);
	uintptr_t getClosestTargetToCrosshair(uintptr_t ptrEntityList, uintptr_t ptrLocalPlayer, Vec3 vecViewAngles);
	bool validateTarget(uintptr_t ptrTarget, uintptr_t ptrLocalPlayer);
	void trackTarget(uintptr_t ptrEngineModule, uintptr_t ptrLocalPlayer, uintptr_t ptrTarget, bool bTorsoAim);
}