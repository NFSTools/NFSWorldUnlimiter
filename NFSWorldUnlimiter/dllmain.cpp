// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

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
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

