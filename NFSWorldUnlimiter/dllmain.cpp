// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <detours/detours.h>
#include <limits>
#include "Memory.h"

DWORD __partTypeCheckFixRetAddr = 0x347880;
void __declspec(naked) __partTypeCheckFix() {
	__asm {
		mov cx, [eax + edx]
		mov dx, [ebp + 0x08]
		cmp dx, cx
		jmp[__partTypeCheckFixRetAddr]
	}
}

DWORD __carHashLoadFixRetAddr = 0x3156B6;
void __declspec(naked) __carHashLoadFix() {
	__asm {
		mov[ebp - 0x10], ax
		jmp[__carHashLoadFixRetAddr]
	}
}


#pragma pack(push, 1)

struct DBCarPart
{
	unsigned char Unused;
	unsigned short CarIndex;
	unsigned char Unused2;
	unsigned int AttributeTableOffset;
	unsigned int Hash;
};

struct CarTypeInfo
{
	char CarTypeName[16];
	char BaseModelName[16];
	char GeometryFilename[32];
	char ManufacturerName[16];
	unsigned int CarTypeNameHash;
	float HeadlightFOV;
	char padHighPerformance;
	char NumAvailableSkinNumbers;
	char WhatGame;
	char ConvertableFlag;
	char WheelOuterRadius;
	char WheelInnerRadiusMin;
	char WheelInnerRadiusMax;
	char pad0;
	float HeadlightPosition[4];
	float DriverRenderingOffset[4];
	float InCarSteeringWheelRenderingOffset[4];
	int Type;
	unsigned int UsageType;
	unsigned int CarMemTypeHash;
	char MaxInstances[5];
	char WantToKeepLoaded[5];
	char pad4[2];
	float MinTimeBetweenUses[5];
	char AvailableSkinNumbers[10];
	char DefaultSkinNumber;
	char Skinnable;
	int Padding;
	int DefaultBasePaint;
};

static_assert(sizeof(CarTypeInfo) == 0xD0, "what???");
static_assert(sizeof(DBCarPart) == 0xC, "what???");

typedef int(__thiscall* sub_7154B0_t)(void*, DBCarPart*);
sub_7154B0_t orig_sub_7154B0 = nullptr;
#pragma pack(pop)

int __fastcall hook_sub_7154B0(void* This, void* EDX, DBCarPart* part)
{
	int result = orig_sub_7154B0(This, part);
	printf("sub_7154B0(%p, %p) = %d\n", This, part, result);
	printf("\tPart: hash=0x%08x carIndex=%u attributeTableOffset=%u\n", part->Hash, part->CarIndex,
		part->AttributeTableOffset);

	if (part->CarIndex == 93)
		return /*30344*/31408;

	return result;
}

typedef signed int(__stdcall* GetCarTypeByHash_t)(int hash);

GetCarTypeByHash_t orig_GetCarTypeByHash = nullptr;
CarTypeInfo** CarTypeInfoArrayPtr = nullptr;

signed int __stdcall hook_GetCarTypeByHash(unsigned int hash)
{
	//printf("GetCarTypeByHash(0x%08x)\n", hash);
	CarTypeInfo* carArray = *CarTypeInfoArrayPtr;
	for (int i = 0; i < 65536; i++)
	{
		if (carArray[i].CarTypeNameHash == hash)
		{
			return i;
		}
	}
	
	return 65536;
}

DWORD WINAPI InitializeASI(LPVOID)
{
	Memory::Init();

	// car hooks
	static constexpr unsigned int car_limit = std::numeric_limits<unsigned short>::max();

	// increase car limit

	for (auto address : {
	   0x654104,
	   0x65415B,
	   0x65419B,
	   0x659935,
	   0x664f89,
	   0x6673dd,
	   0x66d36a,
	   0x6fb97c,
	   0x70bb63,
	   0x70bb70,
	   0x70f63e,
	   0x7364c3,
	   0x7368e3,
	   0x75fc45
		})
	{
		Memory::writeRaw(address - 0x400000, false, 4, car_limit & 0xFF, (car_limit >> 8) & 0xFF, (car_limit >> 16) & 0xFF, (car_limit >> 24) & 0xFF);
	}

	// fix missing parts (Thanks, nlgzrgn)
	// TODO: Why the hell does this work? This game is really puzzling sometimes. But whatever.
	Memory::writeNop(0x7289e2 - 0x400000, 2, false);

	// convert byte ptr access for car index to word ptr (b6->b7)
	Memory::writeRaw(
		0x70BA01 - 0x400000,
		false,
		1,
		0xb7);
	Memory::writeRaw(
		0x7153FB - 0x400000,
		false,
		1,
		0xb7);
	Memory::writeRaw(
		0x7155F1 - 0x400000,
		false,
		1,
		0xb7);
	Memory::writeRaw(
		0x7156F7 - 0x400000,
		false,
		1,
		0xb7);
	Memory::writeRaw(
		0x71E335 - 0x400000,
		false,
		1,
		0xb7);
	Memory::writeRaw(
		0x71FE07 - 0x400000,
		false,
		1,
		0xb7);

	// patch carpart loader to use 16-bit registers
	__partTypeCheckFixRetAddr = Memory::makeAbsolute(__partTypeCheckFixRetAddr);
	__carHashLoadFixRetAddr = Memory::makeAbsolute(__carHashLoadFixRetAddr);
	Memory::writeJMP(0x347878, (DWORD)__partTypeCheckFix, false);
	Memory::writeJMP(0x3156EE, (DWORD)__carHashLoadFix, false);

	// experimental
	//AllocConsole();
	//static FILE* pStdOut = nullptr;
	//freopen_s(&pStdOut, "CONOUT$", "w", stdout);
	//printf("NFS World Unlimiter\n");
	CarTypeInfoArrayPtr = (CarTypeInfo**)Memory::makeAbsolute(0xD20F2C - 0x400000);
	orig_sub_7154B0 = (sub_7154B0_t)Memory::makeAbsolute(0x3154B0);
	orig_GetCarTypeByHash = (GetCarTypeByHash_t)Memory::makeAbsolute(0x30BA10);
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)orig_GetCarTypeByHash, hook_GetCarTypeByHash);
	//DetourAttach(&(PVOID&)orig_sub_7154B0, hook_sub_7154B0);
	DetourTransactionCommit();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, InitializeASI, nullptr, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//FreeConsole();
		break;
	}
	return TRUE;
}

