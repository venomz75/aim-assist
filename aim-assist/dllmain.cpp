﻿#include <Windows.h>
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
	bool bParseTargets = true, bTorsoAim = false, bUpdateUI = true, bUpdateUI2 = true, bWaitingForGame = true;
	int iDesiredFOV = 110, iLastPress = 0;
	float fDesiredViewmodelFOV = 90;
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
			bWaitingForGame = false;
			bUpdateUI2 = true;
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
			if (GetAsyncKeyState(keyIncreaseFOV) && GetTickCount() - iLastPress > 10)
			{
				iLastPress = GetTickCount();
				if (iDesiredFOV < 170) 
				{
					iDesiredFOV += 10;
				}
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyDecreaseFOV) && GetTickCount() - iLastPress > 10)
			{
				iLastPress = GetTickCount();
				if (iDesiredFOV > 10) 
				{
					iDesiredFOV -= 10;
				}
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyDefaultFOV) && GetTickCount() - iLastPress > 10)
			{
				iLastPress = GetTickCount();
				iDesiredFOV = 110;
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyIncreaseViewmodelFOV) && GetTickCount() - iLastPress > 50)
			{
				iLastPress = GetTickCount();
				if (fDesiredViewmodelFOV < 170) 
				{
					fDesiredViewmodelFOV += 10;
				}
				bUpdateUI = true;
			}
			if (GetAsyncKeyState(keyDecreaseViewmodelFOV) && GetTickCount() - iLastPress > 50)
			{
				iLastPress = GetTickCount();
				if (fDesiredViewmodelFOV > 10) 
				{
					fDesiredViewmodelFOV -= 10;
				}
				bUpdateUI = true;
			}
			//Aimbot
			bParseTargets = (GetAsyncKeyState(keyAimlock) == -32768) ? false : true; //if aimkey held stop finding new closest target
			 //get our camera's position in game world
			if (bParseTargets) 
			{
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
			
			//Glowhack
			
			for (int i = 0; i < 64; i++)
			{
				uintptr_t ptrTarget = *(uintptr_t*)(ptrEntityList + offsets::firstEntity + (i * offsets::nextEntity));
				if (ptrTarget)
				{
					bool* tDormant = (bool*)(ptrTarget + offsets::plrDormant);
					int* tLifeState = (int*)(ptrTarget + offsets::plrLifeState);
					int* tHealth = (int*)(ptrTarget + offsets::plrHealth);
					if (!*tDormant && *tLifeState != 2 && *tLifeState != 3 && *tLifeState != 4 && ptrTarget != ptrLocalPlayer && *tHealth > 0 && *tHealth < 101)
					{
						*(bool*)(ptrTarget + offsets::bEnableGlow) = true;
						//std::cout << "Entity " << i << " @ " << ptrTarget << " with glow set to " << *glow << ".\n";
					}
				}
			}
			
			//Bunnyhop
			while (GetAsyncKeyState(VK_SPACE) && GetTickCount() - iLastPress > 1)
			{
				float fPlayerVelocityX = *(float*)(ptrLocalPlayer + offsets::plrVelocityX);
				float fPlayerVelocityY = *(float*)(ptrLocalPlayer + offsets::plrVelocityY);
				if (fPlayerVelocityX > 20 || fPlayerVelocityX < -20 || fPlayerVelocityY > 20 || fPlayerVelocityY < -20)
				{
					bhop::bunnyHop((ptrClientModule + offsets::dwForceJump), (ptrLocalPlayer + offsets::fFlags));
				}
			}

			//Anti Recoil/Screenshake and FOV lock
			anti_recoil::removeScreenshake(ptrLocalPlayer, offsets::plrRecoilPitch, offsets::plrRecoilYaw, offsets::plrRecoilRoll, offsets::plrScreenshake);
			fov::setFOV((ptrLocalPlayer + offsets::plrFOV), iDesiredFOV);
			uintptr_t viewmodelFOV = *(uintptr_t*)(ptrClientModule + offsets::clientViewmodelFOV);
			*(float*)(viewmodelFOV + 0x2C) = fDesiredViewmodelFOV;
			
		}
		else
		{	
			bWaitingForGame = true;	
		}

		if (bUpdateUI && bUpdateUI2)
		{
			/*if (bWaitingForGame) 
			{
				system("CLS");
				//std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
				std::cout << "Deadeye 0.9.2\n";
				std::cout << "Do not close this window!\n";
				std::cout << "Waiting for game...";
				bUpdateUI = false;
			}
			else
			{
				system("CLS");
				//std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
				std::cout << "Deadeye 0.9.2\n";
				if (!bTorsoAim)
				{
					std::cout << "TARGETING: HEAD\n";
				}
				else
				{
					std::cout << "TARGETING: TORSO\n";
				}

				std::cout << "FOV: " << iDesiredFOV << "\n";
				std::cout << "VIEWMODEL FOV: " << fDesiredViewmodelFOV << "\n";
				bUpdateUI = false;
			}*/
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
