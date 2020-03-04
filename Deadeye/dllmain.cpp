#include <Windows.h>
#include <iostream>
#include <math.h>
#include <chrono>
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
uintptr_t localPlayer = 0x0575BA0;
uintptr_t entityList = 0x53B828;
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
uintptr_t playerPitch = 0x47F1B4;
uintptr_t playerYaw = 0x47F1B8;


/*---SETTINGS---*/
uintptr_t keyExit = VK_END;
uintptr_t keyAimlock = VK_XBUTTON2;
uintptr_t keyBoneToggle = VK_XBUTTON1;

/*---FUNCTIONS---*/
void aimCalculate (Vec3 cameraPos, Vec3 targetPos, float* pitch, float* yaw) 
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
		*pitch = pitchFinal;
		*yaw = yawFinal;
	}
}

uintptr_t getClosestPlayer(uintptr_t entList, uintptr_t player, Vec3 playerPos)
{
	int closestPlayer = -1;
	int closestDistance = 99999;
	for (int i = 0; i < 64; i++)
	{
		uintptr_t target = *(uintptr_t*)(entList + firstEntity + (i * nextEntity));
		if (target)
		{
			float* tX = (float*)(target + entVecOriginX);
			float* tY = (float*)(target + entVecOriginY);
			float* tZ = (float*)(target + entVecOriginZ);
			bool* tDormant = (bool*)(target + entDormant);
			int* tLifeState = (int*)(target + entLifeState);
			int* tHealth = (int*)(target + entHealth);
			int* tTeam = (int*)(target + entTeam);
			int* playerTeam = (int*)(player + entTeam);
			if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && target != player && *tHealth > 0 && *tHealth < 101 && *tX != 0 && *tY != 0 && *tZ != 0 && (*tTeam == 0 || *tTeam != *playerTeam))
			{
				Vec3 targetPos = { *tX, *tY, *tZ };
				Vec3 delta = targetPos - playerPos;
				float totalDelta = abs(delta.x) + abs(delta.y) + abs(delta.z);
				if (totalDelta < closestDistance && totalDelta > 0)
				{
					closestPlayer = i;
					closestDistance = totalDelta;
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
	uintptr_t clientModule = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t engineModule = (uintptr_t)GetModuleHandle(L"engine.dll");
	bool bodyAim = false;
	DWORD lastPress = 0;
	while (!GetAsyncKeyState(keyExit))
	{
		uintptr_t player = *(uintptr_t*)(clientModule + localPlayer);
		uintptr_t entList = *(uintptr_t*)(clientModule + entityList);
		if (player) 
		{
			if (GetAsyncKeyState(keyBoneToggle) && GetTickCount() - lastPress > 250)
			{
				lastPress = GetTickCount();
				bodyAim = !bodyAim;
			}
			float* pitch = (float*)(engineModule + playerPitch);
			float* yaw = (float*)(engineModule + playerYaw);
			float* pX = (float*)(player + entVecOriginX);
			float* pY = (float*)(player + entVecOriginY);
			float* pZ = (float*)(player + entVecOriginZ);
			float* pZViewOffset = (float*)(player + entVecViewOffset);
			Vec3 playerPos = { *pX, *pY, *pZ };
			Vec3 cameraPos = playerPos; cameraPos.z += *pZViewOffset;
			uintptr_t target = *(uintptr_t*)getClosestPlayer(entList, player, playerPos);
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
						*tZ += 2;
						if (bodyAim) {
							tX = (float*)(tgtBones + entTorsoBoneX);
							tY = (float*)(tgtBones + entTorsoBoneY);
							tZ = (float*)(tgtBones + entTorsoBoneZ);
						}
						Vec3 targetPos = { *tX, *tY, *tZ };
						aimCalculate(cameraPos, targetPos,  pitch, yaw);
					}
					else break;
				}
				else break;
			}
		}
	}

	FreeLibraryAndExitThread(hModule, 0);
	CloseHandle(hModule);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) 
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Deadeye, hModule, 0, 0);
	return TRUE;
}

/*TODO
-Anti drunk
-Aim lock needs to lock onto 1 target only
-Teams need to be filtered.
-Fix any weird bugs issues
-Bhop/autostrafe
...*/