#pragma once
#include <windows.h>
#include "vector3.h"

namespace aimbot
{
	uintptr_t getTarget(uintptr_t ptrEntityList, uintptr_t ptrLocalPlayer, Vec3 vecCameraPos, Vec3 vecViewAngles);
	Vec3 calculateAngles(Vec3 vecCameraPos, Vec3 vecTargetPos);
}