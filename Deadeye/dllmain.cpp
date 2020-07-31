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


/*---OFFSETS---*/
/*Entities*/
//Navigating the entities
uintptr_t localPlayer = 0x575E20;
uintptr_t entityList = 0x53B824;
uintptr_t firstEntity = 0x14;
uintptr_t nextEntity = 0x10;
uintptr_t nextBone = 0x30;
//Entity properties
uintptr_t entVecOriginX = 0x31C;
uintptr_t entVecOriginY = 0x32C;
uintptr_t entVecOriginZ = 0x33C;
uintptr_t entVecViewOffset = 0x28;
uintptr_t entTeam = 0xB8;
uintptr_t entDormant = 0x186;
uintptr_t entHealth = 0x38;
uintptr_t entLifeState = 0xB6;
//Bones
uintptr_t entBoneMatrix = 0x580;
uintptr_t entHeadBoneX = 0x12C;
uintptr_t entHeadBoneY = 0x13C;
uintptr_t entHeadBoneZ = 0x14C;
uintptr_t entTorsoBoneX = 0x6C;
uintptr_t entTorsoBoneY = 0x7C;
uintptr_t entTorsoBoneZ = 0x8C;
//Player view
uintptr_t playerPitch = 0x4791B4;
uintptr_t playerYaw = 0x4791B8;
uintptr_t aimPunchPitch = 0xEEC;
uintptr_t aimPunchYaw = 0xEF0;
uintptr_t aimPunchRoll = 0xEF4;
uintptr_t aimPunchVelPitch = 0xF28;
uintptr_t aimPunchVelYaw = 0xF2C;
uintptr_t aimPunchVelRoll = 0xF30;
uintptr_t playerDrunkness = 0x443C;
uintptr_t playerFOV = 0x107C;
uintptr_t playerConsecutiveJumps = 0x10C8;
//Other
uintptr_t dwForceJump = 0x5A36DC;
uintptr_t fFlags = 0x358;
uintptr_t playerAccuracy = 0x440C;
uintptr_t crosshairAperture = 0x15C4;
uintptr_t targetCrosshairAperture = 0x15CC;
uintptr_t sightExpFactor = 0x15D4;

/*---SETTINGS---*/
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
		uintptr_t target = *(uintptr_t*)(entList + firstEntity + (i * nextEntity));
		if (target)
		{
			uintptr_t tgtBones = *(uintptr_t*)(target + entBoneMatrix);
			if (tgtBones) {
				float* tX = (float*)(tgtBones + entHeadBoneX);
				float* tY = (float*)(tgtBones + entHeadBoneY);
				float* tZ = (float*)(tgtBones + entHeadBoneZ);
				bool* tDormant = (bool*)(target + entDormant);
				int* tLifeState = (int*)(target + entLifeState);
				int* tHealth = (int*)(target + entHealth);
				int* tTeam = (int*)(target + entTeam);
				int* playerTeam = (int*)(player + entTeam);
				if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && target != player && *tHealth > 0 && *tHealth < 101 && *tX != 0 && *tY != 0 && *tZ != 0 && (*tTeam == 0 || *tTeam != *playerTeam))
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
	uintptr_t targetFinal = (entList + firstEntity + (closestPlayer * nextEntity));
	return targetFinal;
}

/*---ENTRY POINT---*/
int WINAPI Deadeye(HMODULE hModule)
{
	//INIT CONSOLE
	
	AllocConsole();
	FILE * f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	uintptr_t clientModule = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t engineModule = (uintptr_t)GetModuleHandle(L"engine.dll");

	bool scanEntities = true, bodyAim = false, allowUIUpdate = true;
	int desiredFOV = 110;
	DWORD lastPress = 0;
	uintptr_t target;

	while (1)
	{
		if (GetAsyncKeyState(keyExit) && GetTickCount() - lastPress > 1)
		{
			break;
		}
		uintptr_t player = *(uintptr_t*)(clientModule + localPlayer);
		uintptr_t entList = *(uintptr_t*)(clientModule + entityList);
		//When ingame
		if (player)
		{
			//Settings

			if (GetAsyncKeyState(keyBodyAim) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				bodyAim = true;
				allowUIUpdate = true;
			}
			if (GetAsyncKeyState(keyHeadAim) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				bodyAim = false;
				allowUIUpdate = true;
			}
			if (GetAsyncKeyState(keyIncreaseFOV) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				if (desiredFOV < 170) {
					desiredFOV += 10;
				}
				allowUIUpdate = true;
			}
			if (GetAsyncKeyState(keyDecreaseFOV) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				if (desiredFOV > 10) {
					desiredFOV -= 10;
				}
				allowUIUpdate = true;
			}
			if (GetAsyncKeyState(keyDefaultFOV) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				desiredFOV = 110;
				allowUIUpdate = true;
			}
			//Aimbot
			if (GetAsyncKeyState(keyAimlock) == -32768) //if we have a lock, stop looking for closest target until aimlock is released.
			{
				scanEntities = false;
			}
			else
			{
				scanEntities = true;
			}
			float* pitch = (float*)(engineModule + playerPitch);
			float* yaw = (float*)(engineModule + playerYaw);
			float* pX = (float*)(player + entVecOriginX);
			float* pY = (float*)(player + entVecOriginY);
			float* pZ = (float*)(player + entVecOriginZ);
			float* pZViewOffset = (float*)(player + entVecViewOffset);
			Vec3 playerPos = { *pX, *pY, *pZ };
			Vec3 cameraPos = playerPos; cameraPos.z += *pZViewOffset;

			if (scanEntities) {
				target = *(uintptr_t*)getClosestPlayerFOV(entList, player, cameraPos, pitch, yaw);
			}
			
			while (GetAsyncKeyState(keyAimlock) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				if (target) 
				{
					pitch = (float*)(engineModule + playerPitch);
					yaw = (float*)(engineModule + playerYaw);
					pX = (float*)(player + entVecOriginX);
					pY = (float*)(player + entVecOriginY);
					pZ = (float*)(player + entVecOriginZ);
					pZViewOffset = (float*)(player + entVecViewOffset);
					playerPos = { *pX, *pY, *pZ };
					cameraPos = playerPos; cameraPos.z += *pZViewOffset;

					bool* tDormant = (bool*)(target + entDormant);
					__int8* tLifeState = (__int8*)(target + entLifeState);
					uintptr_t tgtBones = *(uintptr_t*)(target + entBoneMatrix);
					if (tgtBones) 
					{
						float* tX = (float*)(tgtBones + entHeadBoneX);
						float* tY = (float*)(tgtBones + entHeadBoneY);
						float* tZ = (float*)(tgtBones + entHeadBoneZ);
						if (bodyAim) {
							tX = (float*)(tgtBones + entTorsoBoneX);
							tY = (float*)(tgtBones + entTorsoBoneY);
							tZ = (float*)(tgtBones + entTorsoBoneZ);
						}
						bool* tDormant = (bool*)(target + entDormant);
						int* tLifeState = (int*)(target + entLifeState);
						int* tHealth = (int*)(target + entHealth);
						int* tTeam = (int*)(target + entTeam);
						int* playerTeam = (int*)(player + entTeam);
						Vec3 targetPos = { *tX, *tY, *tZ+2 };
						if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && target != player && *tHealth > 0 && *tHealth < 101 && (*tX != 0 && *tY != 0 && *tZ != 0) && (*tTeam == 0 || *tTeam != *playerTeam))
						{
							std::pair<float, float> finalAngles = aimCalculate(cameraPos, targetPos);
							*pitch = std::get<0>(finalAngles);
							*yaw = std::get<1>(finalAngles);
						}
						else
						{
							scanEntities = true;
						}
						
					}
					else break;
				}
				else break;
			}
			//Bunnyhop

			int* forceJump = (int*)(clientModule + dwForceJump);
			int* flags = (int*)(player + fFlags);
			while (GetAsyncKeyState(VK_SPACE) && GetTickCount() - lastPress > 1)
			{
				lastPress = GetTickCount();
				if (forceJump && flags) 
				{
					if (*flags == 257) *forceJump = 6;
				}
				
			}

			//Anti Recoil/Drunk and FOV lock
			float* recoilPitch = (float*)(player + aimPunchPitch);
			float* recoilYaw = (float*)(player + aimPunchYaw);
			float* recoilRoll = (float*)(player + aimPunchRoll);
			float* recoilVelPitch = (float*)(player + aimPunchVelPitch);
			float* recoilVelYaw = (float*)(player + aimPunchVelYaw);
			float* recoilVelRoll = (float*)(player + aimPunchVelRoll);
			float* drunkness = (float*)(player + playerDrunkness);
			float* xhairAperture = (float*)(player + crosshairAperture);
			float* targetXhairAperture = (float*)(player + targetCrosshairAperture);
			float* sightFactor = (float*)(player + sightExpFactor);
			int* FOV = (int*)(player + playerFOV);
			/**recoilVelPitch = 0;
			*recoilVelYaw = 0;
			*recoilVelRoll = 0;*/
			*recoilPitch = 0;
			*recoilYaw = 0;
			*recoilRoll = 0;
			*FOV = desiredFOV;
			*drunkness = 0;
		}
		//OUTPUT

		if (allowUIUpdate)
		{
			system("CLS");

			std::cout << "Deadeye v0.9.0 by venomz75\n\n";
			if (player)
			{
				std::cout << ">>SETTINGS<<\n" << "AIMBOT TARGETING: ";
				if (bodyAim)
				{
					std::cout << "BODY\n";
				}
				else
				{
					std::cout << "HEAD\n";
				}
				std::cout << "FIELD OF VIEW: " << desiredFOV << "\n\n";
				allowUIUpdate = false;
			}
			else
			{
				std::cout << "WAITING FOR GAME...";
				allowUIUpdate = true;
			}
			
			
		}
	}
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) 
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Deadeye, hModule, 0, nullptr));
	return TRUE;
}
