#include <Windows.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <string>
#include "offsets.h"
#include "vector3.h"
#include "keybinds.h"
#include "aimbot.h"
#include "bunnyhop.h"
#include "fov_changer.h"
#include "anti_recoil.h"

/*
uintptr_t getClosestPlayerFOV(uintptr_t ptrEntityList, uintptr_t ptrLocalPlayer, Vec3 vecCameraPos, Vec3 vecViewAngles)
{
	int closestPlayer = -1;
	float closestAngleDiff = 99999;
	for (int i = 0; i < 64; i++)
	{
		uintptr_t ptrTarget = *(uintptr_t*)(ptrEntityList + offsets::firstEntity + (i * offsets::nextEntity));
		if (ptrTarget)
		{
			uintptr_t ptrTargetBones = *(uintptr_t*)(ptrTarget + offsets::plrBoneMatrix);
			if (ptrTargetBones) {
				float* tX = (float*)(ptrTargetBones + offsets::plrHeadX);
				float* tY = (float*)(ptrTargetBones + offsets::plrHeadY);
				float* tZ = (float*)(ptrTargetBones + offsets::plrHeadZ);
				bool* tDormant = (bool*)(ptrTarget + offsets::plrDormant);
				int* tLifeState = (int*)(ptrTarget + offsets::plrLifeState);
				int* tHealth = (int*)(ptrTarget + offsets::plrHealth);
				int* tTeam = (int*)(ptrTarget + offsets::plrTeam);
				int* playerTeam = (int*)(ptrLocalPlayer + offsets::plrTeam);
				if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && ptrTarget != ptrLocalPlayer && *tHealth > 0 && *tHealth < 101 && *tX != 0 && *tY != 0 && *tZ != 0 && (*tTeam == 0 || *tTeam != *playerTeam))
				{
					Vec3 targetPos = { *tX, *tY, *tZ };
					Vec3 delta = targetPos - vecCameraPos;
					float targetDistance = abs(delta.x + delta.y + delta.z) * 0.005;
					Vec3 targetAngles = aimbot::calculateAngles(vecCameraPos, targetPos);
					float targetAngleDiffPitch = targetAngles.x - vecViewAngles.x;
					float targetAngleDiffYaw = targetAngles.y - vecViewAngles.y;

					if (targetAngleDiffYaw > 180) {
						targetAngleDiffYaw -= 360;
					}
					if (targetAngleDiffYaw < -180) {
						targetAngleDiffYaw += 360;
					}
					if (targetAngleDiffPitch > 89) {
						targetAngleDiffPitch -= 180;
					}
					if (targetAngleDiffPitch < -89) {
						targetAngleDiffPitch += 180;
					}
					targetAngleDiffPitch = abs(targetAngleDiffPitch);
					targetAngleDiffYaw = abs(targetAngleDiffYaw);
					float targetAngleDiff = hypotf(targetAngleDiffPitch, targetAngleDiffYaw) + targetDistance;
					if (targetAngleDiff < closestAngleDiff && targetAngleDiffYaw < 55)
					{
						closestPlayer = i;
						closestAngleDiff = targetAngleDiff;
					}
				}
			}
		}
	}
	uintptr_t targetFinal = (ptrEntityList + offsets::firstEntity + (closestPlayer * offsets::nextEntity));
	return targetFinal;
}
*/
DWORD WINAPI Deadeye(HMODULE hModule)
{
	//Initialise console
	//AllocConsole();
	//FILE * f;
	//freopen_s(&f, "CONOUT$", "w", stdout);

	//Initialise modules
	uintptr_t ptrClientModule = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t ptrEngineModule = (uintptr_t)GetModuleHandle(L"engine.dll");

	//Initialise global variables
	bool bParseTargets = true, bTorsoAim = false, bUpdateUI = true;
	int iDesiredFOV = 110, iLastPress = 0;
	uintptr_t ptrTarget;

	//Main loop
	while (1)
	{
		if (GetAsyncKeyState(keyExit) && GetTickCount() - iLastPress > 1)
		{
			break;
		}
		uintptr_t ptrLocalPlayer = *(uintptr_t*)(ptrClientModule + offsets::localPlayer);
		uintptr_t ptrEntityList = *(uintptr_t*)(ptrClientModule + offsets::entityList);
		//Check if ingame
		if (ptrLocalPlayer)
		{
			//Settings
			if (GetAsyncKeyState(keyBodyAim) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				bTorsoAim = true;
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyHeadAim) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				bTorsoAim = false;
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyIncreaseFOV) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				if (iDesiredFOV < 170) {
					iDesiredFOV += 10;
				}
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyDecreaseFOV) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				if (iDesiredFOV > 10) {
					iDesiredFOV -= 10;
				}
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyDefaultFOV) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				iDesiredFOV = 110;
				bUpdateUI = true;
			}

			//Aimbot
			bParseTargets = (GetAsyncKeyState(keyAimlock) == -32768) ? false : true; //if aimkey held stop finding new closest target
			 //get our camera's position in game world
			if (bParseTargets) {
				ptrTarget = *(uintptr_t*)aimbot::getClosestTargetToCrosshair(ptrEntityList, ptrLocalPlayer, Vec3{ *(float*)(ptrEngineModule + offsets::engPitch), *(float*)(ptrEngineModule + offsets::engYaw) });
			} //if we are looking for closest target find the closest and keep it saved
			
			while (GetAsyncKeyState(keyAimlock) && GetTickCount() - iLastPress > 1) //when aimkey is held
			{
				iLastPress = GetTickCount();
				//init check variables to ensure target is legitimate
				if (aimbot::validateTarget(ptrTarget, ptrLocalPlayer))
				{
					aimbot::trackTarget(ptrEngineModule, ptrLocalPlayer, ptrTarget, bTorsoAim);
				}
				else
				{
					bParseTargets = true;
				} // if the target is bogus, break out of the lock.
			}

			//Bunnyhop
			while (GetAsyncKeyState(VK_SPACE) && GetTickCount() - iLastPress > 1)
			{
				bhop::bunnyHop((ptrClientModule + offsets::dwForceJump), (ptrLocalPlayer + offsets::fFlags));
			}

			//Anti Recoil/Drunk and FOV lock
			anti_recoil::removeScreenshake(ptrLocalPlayer, offsets::plrRecoilPitch, offsets::plrRecoilYaw, offsets::plrRecoilRoll, offsets::plrDrunkness);
			fov::setFOV((ptrLocalPlayer + offsets::plrFOV), iDesiredFOV);
		}
	}

	//Close console
	//fclose(f);
	//FreeConsole();

	//Close thread
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) 
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Deadeye, hModule, 0, nullptr));
	return TRUE;
}
