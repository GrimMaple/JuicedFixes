#include <bit>

#include <IniReader.h>
#include <injector/injector.hpp>

#include "input.h"
#include "controller.h"

#include "stdafx.h"

#define CHECK_TIMEOUT 1000 // Check controllers every n ms

bool pressed[14];

// Patches
bool PatchVirtualMemory = false;
bool PatchCalendarCrash = false;
bool PatchInput = false;

// Version
int version;

ControllerButtons MenuCodes[16];
ControllerButtons RaceCodes[20];

Controllers controllers;

int controlType = 1;

DWORD VersionOneCodeCaveExit = 0x452602;

DWORD WINAPI Background(LPVOID unused)
{
	while (true)
	{
		controllers.Tick(1);
		Sleep(1);
	}
}

void FixCrashOnCalendar()
{
	char crashFix[] =
	{
		0xB0, 0x00, 0x90
	};
	if (version == 0) {
		injector::WriteMemoryRaw(0x004C14CF, crashFix, sizeof(crashFix), true); // Fix crash on calendar
	}
	else {
		injector::WriteMemoryRaw(0x004C317F, crashFix, sizeof(crashFix), true); // Fix crash on calendar
	}
}

void FixVirtualMemory()
{
	char memFix[] =
	{
		0xEB, 0x20
	};

	// Fix "Juiced requires virtual memory to be enabled"
	injector::MakeNOP(0x0059BE39, 2, true);
	injector::WriteMemoryRaw(0x0059BE40, memFix, sizeof(memFix), true);
}

extern "C" void /*__declspec(dllexport)*/ __cdecl SetControlType(char* fmt, int i)
{
	__asm push eax
	controlType = i;
	__asm pop eax
}

extern "C" float __declspec(dllexport) __stdcall GetAxle()
{
	__asm push ecx
	int a, b;
	__asm mov a, edi
	__asm mov b, esi
	__asm pop ecx
	if (controlType == ControlType::Menu)
	{
		switch (a)
		{
		case AxlesMenu::LookDown:
		case AxlesMenu::LookUp:
			return controllers.getValue(0, ControllerButtons::RightThumbY);
		case AxlesMenu::LookLeft:
		case AxlesMenu::LookRight:
			return controllers.getValue(0, ControllerButtons::RightThumbX);
		case 0:
			return controllers.getValue(0, ControllerButtons::LeftThumbX);
		case 1:
			return controllers.getValue(0, ControllerButtons::RightTrigger);
		}
	}
	else
	{
		switch (a)
		{
		case AxlesRace::Steering:
			return controllers.getValue(0, ControllerButtons::LeftThumbX);
		case AxlesRace::Throttle:
			return controllers.getValue(0, ControllerButtons::RightTrigger);
		case AxlesRace::Brake:
			return controllers.getValue(0, ControllerButtons::LeftTrigger);
		case AxlesRace::Reverse:
			return controllers.getValue(0, RaceCodes[static_cast<int>(RaceButtons::Reverse)]);
		case AxlesRace::Handbrake:
			return controllers.getValue(0, RaceCodes[static_cast<int>(RaceButtons::Handbrake)]);
		case AxlesRace::LookAround:
			return controllers.getValue(0, ControllerButtons::RightThumbX);
		case AxlesRace::LookAroundY:
			return controllers.getValue(0, ControllerButtons::RightThumbY);
		}
	}
	return 0.0f;
}

void RaceEnd()
{
	SetControlType(nullptr, ControlType::Menu);
}

void RaceEndv1()
{
	SetControlType(nullptr, ControlType::Menu);
	__asm mov ecx,dword ptr ds:[0x760C94]
}

void RaceStart()
{
	SetControlType(nullptr, ControlType::Race);
}

__declspec(noinline) int ProcessMenuInput(int key)
{
	if (key >= ControllerButtons::End || key < 0)
		return 0;
	switch (key)
	{
	case MenuButtons::Down:
	case MenuButtons::DownDigital:
		return controllers.getPressed(0, ControllerButtons::DPadDown);
	case MenuButtons::Up:
	case MenuButtons::UpDigital:
		return controllers.getPressed(0, ControllerButtons::DPadUp);
	case MenuButtons::Left:
	case MenuButtons::LeftDigital:
		return controllers.getPressed(0, ControllerButtons::DPadLeft);
	case MenuButtons::Right:
	case MenuButtons::RightDigital:
		return controllers.getPressed(0, ControllerButtons::DPadRight);
	default:
		return controllers.getSinglePress(0, static_cast<ControllerButtons>(MenuCodes[key]));
	}
	return 0;
}

__declspec(noinline) int ProcessRaceInput(int key)
{
	auto button = static_cast<ControllerButtons>(RaceCodes[key]);
	auto k = static_cast<RaceButtons>(key);
	switch (k)
	{
	case RaceButtons::Horn:
	case RaceButtons::LookBack:
	case RaceButtons::Nitro:
		return std::bit_cast<int>(controllers.getValue(0, static_cast<ControllerButtons>(RaceCodes[key])));
	case RaceButtons::ChangeView:
	case RaceButtons::GearUp:
	case RaceButtons::GearDown:
		return controllers.getSinglePress(0, button);
	default:
		return controllers.getPressed(0, static_cast<ControllerButtons>(RaceCodes[key]));
	}
	return 0;
}

extern "C" int __declspec(dllexport) __stdcall GetButton()
{
	int a, b;
	__asm mov a, eax
	__asm mov b, edi
	return controlType == ControlType::Race ? ProcessRaceInput(a) : ProcessMenuInput(a);
}

void ReadConfig()
{
	CIniReader iniReader("fixes.ini");

	PatchInput = iniReader.ReadBoolean("Fixes", "PatchInput", false);
	PatchVirtualMemory = iniReader.ReadBoolean("Fixes", "PatchVirtualMemory", false);
	PatchCalendarCrash = iniReader.ReadBoolean("Fixes", "PatchCalendarCrash", false);

	MenuCodes[0] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Left", JOY_DPAD_LEFT));
	//MenuCodes[1] = iniReader.ReadInteger("MenuControls", "LeftDifital", JOY_DPAD_LEFT);
	MenuCodes[2] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Right", JOY_DPAD_RIGHT));
	//MenuCodes[3] = iniReader.ReadInteger("MenuControls", "RightDigital", JOY_DPAD_LEFT);
	MenuCodes[4] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Up", JOY_DPAD_UP));
	//MenuCodes[5] = iniReader.ReadInteger("MenuControls", "UpDigital", JOY_DPAD_UP);
	MenuCodes[6] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Down", JOY_DPAD_DOWN));
	//MenuCodes[7] = iniReader.ReadInteger("MenuControls", "DownDigital", JOY_DPAD_DOWN);
	MenuCodes[8] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Accept", JOY_BTN_A));
	MenuCodes[9] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Menu9", 110));
	MenuCodes[10] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "PageDown", JOY_BTN_Y));
	MenuCodes[11] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Tab", JOY_BTN_X));
	MenuCodes[12] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Back", JOY_BTN_B));
	MenuCodes[13] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Back2", 110));
	MenuCodes[14] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Menu14", 110));
	MenuCodes[15] = static_cast<ControllerButtons>(iniReader.ReadInteger("MenuControls", "Menu15", 110));

	RaceCodes[0] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race0", 110));
	RaceCodes[1] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Pause", JOY_BTN_START));
	RaceCodes[2] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "ChangeView", JOY_BTN_BACK));
	RaceCodes[3] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Nitro", JOY_BTN_X));
	RaceCodes[4] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Horn", JOY_DPAD_UP));
	RaceCodes[5] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "LookBack", JOY_DPAD_DOWN));
	RaceCodes[6] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "GearDown", JOY_BTN_B));
	RaceCodes[7] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "GearUp", JOY_BTN_A));
	RaceCodes[8] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Skip", 110));
	RaceCodes[9] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race9", 110));
	RaceCodes[10] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race10", 110));
	RaceCodes[11] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race11", 110));
	RaceCodes[12] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race12", 110));
	RaceCodes[13] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race13", 110));
	RaceCodes[14] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race14", 110));
	RaceCodes[15] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Race15", 110));
	RaceCodes[16] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Handbrake", 110));
	RaceCodes[17] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Reverse", 110));
	RaceCodes[18] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Axle2", 110));
	RaceCodes[19] = static_cast<ControllerButtons>(iniReader.ReadInteger("RaceControls", "Axle3", 110));
}

int WINAPI DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		char VersionOneReturn[] = {
			0x90
		};
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

		if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x00CD6FB0 || (base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x005CD316) {  // Version Check. Credit goes to thelink2012 and MWisBest.
			version = 1;
		}
		else {
			version = 0;
		}

		// Clear controller mapping
		memset(MenuCodes, 110, sizeof(MenuCodes));
		memset(RaceCodes, 110, sizeof(RaceCodes));

		for (int i = 0; i < 14; i++)
			pressed[i] = false;
		ReadConfig();

		controlType = ControlType::Menu;
		if(PatchCalendarCrash) FixCrashOnCalendar();
		if(PatchVirtualMemory && version == 0) FixVirtualMemory();

		if (PatchInput)
		{
			if (version == 1) {
				injector::MakeJMP(0x00401640, GetAxle);
				injector::MakeJMP(0x004015D0, GetButton);
				injector::MakeCALL(0x004632DE, RaceStart);	// Switch control type for race
				injector::MakeCALL(0x004525FC, RaceEndv1);	// Switch control type for race end
				injector::WriteMemoryRaw(0x00452601, VersionOneReturn, sizeof(VersionOneReturn), true);
				injector::MakeCALL(0x005BD47E, SetControlType); // Switch control type for pause menu
			}
			else {
				injector::MakeJMP(0x00401640, GetAxle);
				injector::MakeJMP(0x004015D0, GetButton);
				injector::MakeCALL(0x0046175E, RaceStart);	// Switch control type for race
				injector::MakeCALL(0x00450B3B, RaceEnd);	// Switch control type for race end
				injector::MakeCALL(0x005BB0FE, SetControlType); // Switch control type for pause menu
			}
			
		}

		CreateThread(0, 0, Background, NULL, 0, NULL);
	}
	return TRUE;
}
