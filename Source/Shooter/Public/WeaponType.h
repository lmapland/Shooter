#pragma once


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun = 0 UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle = 1 UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol = 2 UMETA(DisplayName = "Pistol"),
};