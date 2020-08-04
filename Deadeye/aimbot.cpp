#include "aimbot.h"
#include <math.h>
#include <iostream>

uintptr_t localPlayer = 0x575E20;
uintptr_t entityList = 0x53B824;
uintptr_t firstEntity = 0x14;
uintptr_t nextEntity = 0x10;

uintptr_t plrVecX = 0x31C;
uintptr_t plrVecY = 0x32C;
uintptr_t plrVecZ = 0x33C;
uintptr_t plrVecViewOffset = 0x28;
uintptr_t plrTeam = 0xB8;
uintptr_t plrHealth = 0x38;
uintptr_t plrDormant = 0x186;
uintptr_t plrLifeState = 0xB6;

uintptr_t plrBoneMatrix = 0x580;
uintptr_t plrHeadX = 0x12C;
uintptr_t plrHeadY = 0x13C;
uintptr_t plrHeadZ = 0x14C;
uintptr_t plrTorsoX = 0x6C;
uintptr_t plrTorsoY = 0x7C;
uintptr_t plrTorsoZ = 0x8C;

uintptr_t engPitch = 0x4791B4;
uintptr_t engYaw = 0x4791B8;

Vec3 aimbot::calculateAngles(Vec3 vecCameraPos, Vec3 vecTargetPos)
{
	Vec3 vecDelta = vecTargetPos - vecCameraPos;
	float fOpp, fAdj, fHyp;

	if (abs(vecDelta.x) > abs(vecDelta.y))
	{
		fAdj = vecDelta.x;
		fOpp = vecDelta.y;
	}
	else
	{
		fAdj = vecDelta.y;
		fOpp = vecDelta.x;
	}
	fHyp = fAdj / cos(atan2(fOpp, fAdj));

	Vec3 vecNewAngles = { -atan(vecDelta.z / fHyp) * (180 / 3.14159265), atan2(vecDelta.y, vecDelta.x) * (180 / 3.14159265) }; //(PITCH, YAW)
	vecNewAngles.Normalise(); //If angles are somehow illegal they will be set to legal ones
	return vecNewAngles;
}

uintptr_t aimbot::getClosestTargetToCrosshair(uintptr_t ptrEntityList, uintptr_t ptrLocalPlayer, Vec3 vecViewAngles)
{
	int iClosestPlayer = -1;
	float fLowestAngleDifference = 99999;
	for (int i = 0; i < 64; i++)
	{
		uintptr_t ptrTarget = *(uintptr_t*)(ptrEntityList + firstEntity + (i * nextEntity));
		if (ptrTarget)
		{
			uintptr_t ptrTargetBones = *(uintptr_t*)(ptrTarget + plrBoneMatrix);
			if (ptrTargetBones) {
				Vec3 vecCameraPos =
				{
					*(float*)(ptrLocalPlayer + plrVecX),
					*(float*)(ptrLocalPlayer + plrVecY),
					*(float*)(ptrLocalPlayer + plrVecZ) + *(float*)(ptrLocalPlayer + plrVecViewOffset)
				};
				float* tX = (float*)(ptrTargetBones + plrHeadX);
				float* tY = (float*)(ptrTargetBones + plrHeadY);
				float* tZ = (float*)(ptrTargetBones + plrHeadZ);
				bool* tDormant = (bool*)(ptrTarget + plrDormant);
				int* tLifeState = (int*)(ptrTarget + plrLifeState);
				int* tHealth = (int*)(ptrTarget + plrHealth);
				int* tTeam = (int*)(ptrTarget + plrTeam);
				int* playerTeam = (int*)(ptrLocalPlayer + plrTeam);
				if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && ptrTarget != ptrLocalPlayer && *tHealth > 0 && *tHealth < 101 && *tX != 0 && *tY != 0 && *tZ != 0 && (*tTeam == 0 || *tTeam != *playerTeam))
				{
					Vec3 vecTargetPos = { *tX, *tY, *tZ };
					Vec3 vecDelta = vecTargetPos - vecCameraPos;
					float fTargetDistance = abs(vecDelta.x + vecDelta.y + vecDelta.z) * 0.005;
					Vec3 vecTargetAngles = aimbot::calculateAngles(vecCameraPos, vecTargetPos);
					float fTargetAngleDiffPitch = vecTargetAngles.x - vecViewAngles.x;
					float fTargetAngleDiffYaw = vecTargetAngles.y - vecViewAngles.y;

					if (fTargetAngleDiffYaw > 180) {
						fTargetAngleDiffYaw -= 360;
					}
					if (fTargetAngleDiffYaw < -180) {
						fTargetAngleDiffYaw += 360;
					}
					if (fTargetAngleDiffPitch > 89) {
						fTargetAngleDiffPitch -= 180;
					}
					if (fTargetAngleDiffPitch < -89) {
						fTargetAngleDiffPitch += 180;
					}
					fTargetAngleDiffPitch = abs(fTargetAngleDiffPitch);
					fTargetAngleDiffYaw = abs(fTargetAngleDiffYaw);
					float targetAngleDiff = hypotf(fTargetAngleDiffPitch, fTargetAngleDiffYaw) + fTargetDistance;
					if (targetAngleDiff < fLowestAngleDifference && fTargetAngleDiffYaw < 55)
					{
						iClosestPlayer = i;
						fLowestAngleDifference = targetAngleDiff;
					}
				}
			}
		}
	}
	uintptr_t targetFinal = (ptrEntityList + firstEntity + (iClosestPlayer * nextEntity));
	return targetFinal;
}

bool aimbot::validateTarget(uintptr_t ptrTarget, uintptr_t ptrLocalPlayer)
{
	bool* bTargetDormant = (bool*)(ptrTarget + plrDormant);
	__int8* iTargetLifeState = (__int8*)(ptrTarget + plrLifeState);
	int* iTargetHealth = (int*)(ptrTarget + plrHealth);
	__int8* iTargetTeam = (__int8*)(ptrTarget + plrTeam);
	__int8* iPlayerTeam = (__int8*)(ptrLocalPlayer + plrTeam);
	
	bool bValid = (!*bTargetDormant && *iTargetLifeState == 0 && ptrTarget != ptrLocalPlayer && *iTargetHealth > 0 && *iTargetHealth <= 100 && (*iTargetTeam == 0 || *iTargetTeam != *iPlayerTeam)) ? true : false;
	return bValid;
}

void aimbot::trackTarget(uintptr_t ptrEngineModule, uintptr_t ptrLocalPlayer, uintptr_t ptrTarget, bool bTorsoAim)
{
	if (ptrTarget) //if we have a non null target, continue
	{
		uintptr_t ptrTargetBones = *(uintptr_t*)(ptrTarget + plrBoneMatrix); //get target bonematrix
		if (ptrTargetBones) //if the target's bonematrix is non null, continue
		{
			Vec3 vecCameraPos =
			{
				*(float*)(ptrLocalPlayer + plrVecX),
				*(float*)(ptrLocalPlayer + plrVecY),
				*(float*)(ptrLocalPlayer + plrVecZ) + *(float*)(ptrLocalPlayer + plrVecViewOffset)
			}; //update our camera position constantly

			Vec3 vecTargetPos = {
				(bTorsoAim) ? *(float*)(ptrTargetBones + plrTorsoX) : *(float*)(ptrTargetBones + plrHeadX),
				(bTorsoAim) ? *(float*)(ptrTargetBones + plrTorsoY) : *(float*)(ptrTargetBones + plrHeadY),
				(bTorsoAim) ? *(float*)(ptrTargetBones + plrTorsoZ) : *(float*)(ptrTargetBones + plrHeadZ)
			}; //find the target's position in game world, head or torso depending on user setting

			Vec3 finalAngles = aimbot::calculateAngles(vecCameraPos, vecTargetPos);
			*(float*)(ptrEngineModule + engPitch) = finalAngles.x;
			*(float*)(ptrEngineModule + engYaw) = finalAngles.y;

		} // if the target is bogus, break out of the lock.
	}
}
