#include <Windows.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <string>
#include "offsets.h"

/*---CLASSES---*/
struct Vec3 
{
	float x, y, z;

	Vec3 operator+ (Vec3 d) 
	{
		return { x + d.x, y + d.y, z + d.z };
	}

	Vec3 operator- (Vec3 d) 
	{
		return { x - d.x, y - d.y, z - d.z };
	}

	Vec3 operator* (float d) 
	{
		return { x * d, y * d, z * d };
	}
	
	void Normalise() 
	{
		while (y < -180) y += 360;
		while (y > 180) y -= 360;
		if (x > 89) x = 89;
		if (x < -89) x = -89;
	}
};


/*---KEYBINDS---*/
uintptr_t keyExit = VK_END;
uintptr_t keyAimlock = VK_XBUTTON2;
uintptr_t keyBodyAim = VK_XBUTTON1;
uintptr_t keyHeadAim = VK_NUMPAD0;
uintptr_t keyIncreaseFOV = VK_NUMPAD8;
uintptr_t keyDefaultFOV = VK_NUMPAD5;
uintptr_t keyDecreaseFOV = VK_NUMPAD2;

/*---FUNCTIONS---*/
std::pair<float, float> aimCalculate (Vec3 cameraPos, Vec3 targetPos) 
{
	Vec3 delta = targetPos - cameraPos;
	float opp, adj, hyp;

	if (abs(delta.x) > abs(delta.y))
	{
		adj = delta.x;
		opp = delta.y;
	}
	else
	{
		adj = delta.y;
		opp = delta.x;
	}
	hyp = adj / cos(atan2(opp, adj));

	float yawFinal = atan2(delta.y, delta.x) * (180 / 3.14159265);
	float pitchFinal = -atan(delta.z / hyp) * (180 / 3.14159265);

	if (pitchFinal >= -89.00 && pitchFinal <= 89.00 && yawFinal >= -180.00 && yawFinal <= 180.00)
	{
		return std::make_pair(pitchFinal, yawFinal);
	}
}

uintptr_t getClosestPlayerFOV(uintptr_t entList, uintptr_t player, Vec3 cameraPos, float* pitch, float* yaw)
{
	int closestPlayer = -1;
	float closestAngleDiff = 99999;
	for (int i = 0; i < 64; i++)
	{
		uintptr_t ptrTarget = *(uintptr_t*)(entList + offsets::firstEntity + (i * offsets::nextEntity));
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
				int* playerTeam = (int*)(player + offsets::plrTeam);
				if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && ptrTarget != player && *tHealth > 0 && *tHealth < 101 && *tX != 0 && *tY != 0 && *tZ != 0 && (*tTeam == 0 || *tTeam != *playerTeam))
				{
					Vec3 targetPos = { *tX, *tY, *tZ };
					Vec3 delta = targetPos - cameraPos;
					float targetDistance = abs(delta.x + delta.y + delta.z) * 0.005;
					std::pair<float, float> targetAngles = aimCalculate(cameraPos, targetPos);
					float targetAngleDiffPitch = std::get<0>(targetAngles) - *pitch;
					float targetAngleDiffYaw = std::get<1>(targetAngles) - *yaw;

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
	uintptr_t targetFinal = (entList + offsets::firstEntity + (closestPlayer * offsets::nextEntity));
	return targetFinal;
}

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
			Vec3 vecCameraPos =
			{ 
				*(float*)(ptrLocalPlayer + offsets::plrVecX),
				*(float*)(ptrLocalPlayer + offsets::plrVecY),
				*(float*)(ptrLocalPlayer + offsets::plrVecZ) + *(float*)(ptrLocalPlayer + offsets::plrVecViewOffset)
			}; //get our camera's position in game world
			if (bParseTargets) {
				ptrTarget = *(uintptr_t*)getClosestPlayerFOV(ptrEntityList, ptrLocalPlayer, vecCameraPos, (float*)(ptrEngineModule + offsets::engPitch), (float*)(ptrEngineModule + offsets::engYaw));
			} //if we are looking for closest target find the closest and keep it saved
			
			while (GetAsyncKeyState(keyAimlock) && GetTickCount() - iLastPress > 1) //when aimkey is held
			{
				iLastPress = GetTickCount();
				if (ptrTarget) //if we have a non null target, continue
				{
					uintptr_t ptrTargetBones = *(uintptr_t*)(ptrTarget + offsets::plrBoneMatrix); //get target bonematrix
					if (ptrTargetBones) //if the target's bonematrix is non null, continue
					{
						vecCameraPos =
						{
							*(float*)(ptrLocalPlayer + offsets::plrVecX),
							*(float*)(ptrLocalPlayer + offsets::plrVecY),
							*(float*)(ptrLocalPlayer + offsets::plrVecZ) + *(float*)(ptrLocalPlayer + offsets::plrVecViewOffset)
						}; //update our camera position constantly

						Vec3 vecTargetPos = {
							(bTorsoAim) ? *(float*)(ptrTargetBones + offsets::plrTorsoX) : *(float*)(ptrTargetBones + offsets::plrHeadX),
							(bTorsoAim) ? *(float*)(ptrTargetBones + offsets::plrTorsoY) : *(float*)(ptrTargetBones + offsets::plrHeadY),
							(bTorsoAim) ? *(float*)(ptrTargetBones + offsets::plrTorsoZ) : *(float*)(ptrTargetBones + offsets::plrHeadZ)
						}; //find the target's position in game world, head or torso depending on user setting

						//init check variables to ensure target is legitimate
						bool* bTargetDormant = (bool*)(ptrTarget + offsets::plrDormant);
						int* iTargetLifeState = (int*)(ptrTarget + offsets::plrLifeState);
						int* iTargetHealth = (int*)(ptrTarget + offsets::plrHealth);
						int* iTargetTeam = (int*)(ptrTarget + offsets::plrTeam);
						int* iPlayerTeam = (int*)(ptrLocalPlayer + offsets::plrTeam);
						
						if (!*bTargetDormant && *iTargetLifeState == 0  && ptrTarget != ptrLocalPlayer && *iTargetHealth > 0 && *iTargetHealth <= 100 && (*iTargetTeam == 0 || *iTargetTeam != *iPlayerTeam))
						{
							std::pair<float, float> finalAngles = aimCalculate(vecCameraPos, vecTargetPos);
							*(float*)(ptrEngineModule + offsets::engPitch) = std::get<0>(finalAngles);
							*(float*)(ptrEngineModule + offsets::engYaw) = std::get<1>(finalAngles);
						} //if the target is legit, lock on!
						else
						{
							bParseTargets = true;
						} // if the target is bogus, break out of the lock.
					}
					else break;
				}
				else break;
			}
			//Bunnyhop
			int* forceJump = (int*)(ptrClientModule + offsets::dwForceJump);
			int* flags = (int*)(ptrLocalPlayer + offsets::fFlags);
			while (GetAsyncKeyState(VK_SPACE) && GetTickCount() - iLastPress > 1)
			{
				iLastPress = GetTickCount();
				if (forceJump && flags) 
				{
					if (*flags == 257) *forceJump = 6;
				}
				
			}
			//Anti Recoil/Drunk and FOV lock
			*(float*)(ptrLocalPlayer + offsets::plrRecoilPitch) = 0;
			*(float*)(ptrLocalPlayer + offsets::plrRecoilYaw) = 0;
			*(float*)(ptrLocalPlayer + offsets::plrRecoilRoll) = 0;
			*(float*)(ptrLocalPlayer + offsets::plrDrunkness) = 0;
			*(int*)(ptrLocalPlayer + offsets::plrFOV) = iDesiredFOV;
		}
		//OUTPUT
		/*
		if (bUpdateUI)
		{
			system("CLS");

			std::cout << "Deadeye v0.9.0 by venomz75\n\n";
			if (ptrLocalPlayer)
			{
				std::cout << ">>SETTINGS<<\n" << "AIMBOT TARGETING: ";
				if (bTorsoAim)
				{
					std::cout << "BODY\n";
				}
				else
				{
					std::cout << "HEAD\n";
				}
				std::cout << "FIELD OF VIEW: " << iDesiredFOV << "\n\n";
				bUpdateUI = false;
			}
			else
			{
				std::cout << "WAITING FOR GAME...";
				bUpdateUI = true;
			}
		}
		*/
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
