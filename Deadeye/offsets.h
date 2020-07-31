#pragma once
#include <Windows.h>

namespace offsets
{
	//Entity data
	uintptr_t localPlayer = 0x575E20;
	uintptr_t entityList = 0x53B824;
	uintptr_t firstEntity = 0x14;
	uintptr_t nextEntity = 0x10;

	uintptr_t entVecOriginX = 0x31C;
	uintptr_t entVecOriginY = 0x32C;
	uintptr_t entVecOriginZ = 0x33C;
	uintptr_t entVecViewOffset = 0x28;
	uintptr_t entTeam = 0xB8;
	uintptr_t entDormant = 0x186;
	uintptr_t entHealth = 0x38;
	uintptr_t entLifeState = 0xB6;

	uintptr_t nextBone = 0x30;
	uintptr_t entBoneMatrix = 0x580;
	uintptr_t entHeadBoneX = 0x12C;
	uintptr_t entHeadBoneY = 0x13C;
	uintptr_t entHeadBoneZ = 0x14C;
	uintptr_t entTorsoBoneX = 0x6C;
	uintptr_t entTorsoBoneY = 0x7C;
	uintptr_t entTorsoBoneZ = 0x8C;
	
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
	uintptr_t dwForceJump = 0x5A36DC;
	uintptr_t fFlags = 0x358;
	uintptr_t playerAccuracy = 0x440C;
	uintptr_t crosshairAperture = 0x15C4;
	uintptr_t targetCrosshairAperture = 0x15CC;
	uintptr_t sightExpFactor = 0x15D4;
}