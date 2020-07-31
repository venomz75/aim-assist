#pragma once
#include <Windows.h>

namespace offsets
{
	//Entity data
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
	
	uintptr_t plrRecoilPitch = 0xEEC;
	uintptr_t plrRecoilYaw = 0xEF0;
	uintptr_t plrRecoilRoll = 0xEF4;
	uintptr_t plrDrunkness = 0x443C;
	uintptr_t plrFOV = 0x107C;

	uintptr_t engPitch = 0x4791B4;
	uintptr_t engYaw = 0x4791B8;

	uintptr_t plrConsecutiveJumps = 0x10C8;
	uintptr_t dwForceJump = 0x5A36DC;
	uintptr_t fFlags = 0x358;

	/*uintptr_t playerAccuracy = 0x440C;
	uintptr_t crosshairAperture = 0x15C4;
	uintptr_t targetCrosshairAperture = 0x15CC;
	uintptr_t sightExpFactor = 0x15D4;
	uintptr_t nextBone = 0x30;*/
}