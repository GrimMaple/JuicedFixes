#include "IniReader.h"
#include "injector/injector.hpp"

#include "input.h"
#include "xinput.h"

#define JOY_BTN_Y      0
#define JOY_BTN_X      1
#define JOY_BTN_A      2
#define JOY_BTN_B      3
#define JOY_DPAD_DOWN  4
#define JOY_DPAD_UP    5
#define JOY_DPAD_LEFT  6
#define JOY_DPAD_RIGHT 7
#define JOY_BTN_LS     8
#define JOY_BTN_RS     9
#define JOY_BTN_LT     10
#define JOY_BTN_RT     11
#define JOY_BTN_START  12
#define JOY_BTN_BACK   13

#define AXIS_LT       0x01000000
#define AXIS_RT       0x02000000
#define AXIS_LX       0x03000000
#define AXIS_LY       0x04000000
#define AXIS_RX       0x05000000
#define AXIS_RY       0x06000000

#define CHECK_TIMEOUT 1000 // Check controllers every n ms

bool pressed[14];


int JoyBtnToXInputBtn(int joyBtn)
{
	switch (joyBtn)
	{
	case JOY_BTN_A:
		return  XINPUT_GAMEPAD_A;
	case JOY_BTN_B:
		return XINPUT_GAMEPAD_B;
	case JOY_BTN_Y:
		return XINPUT_GAMEPAD_Y;
	case JOY_BTN_X:
		return XINPUT_GAMEPAD_X;
	case JOY_DPAD_DOWN:
		return XINPUT_GAMEPAD_DPAD_DOWN;
	case JOY_DPAD_LEFT:
		return XINPUT_GAMEPAD_DPAD_LEFT;
	case JOY_DPAD_RIGHT:
		return XINPUT_GAMEPAD_DPAD_RIGHT;
	case JOY_DPAD_UP:
		return XINPUT_GAMEPAD_DPAD_UP;
	case JOY_BTN_START:
		return XINPUT_GAMEPAD_START;
	case JOY_BTN_BACK:
		return XINPUT_GAMEPAD_BACK;
	case JOY_BTN_LS:
		return XINPUT_GAMEPAD_LEFT_SHOULDER;
	case JOY_BTN_RS:
		return XINPUT_GAMEPAD_RIGHT_SHOULDER;
	case JOY_BTN_LT:
		return XINPUT_GAMEPAD_LEFT_THUMB;
	case JOY_BTN_RT:
		return XINPUT_GAMEPAD_RIGHT_THUMB;
	}
	return 0;
}


XINPUT_STATE states[4];
bool         connected[4];
int          timeout = 0;

XINPUT_STATE check;

/*
*  Buttons check order
*  Deatils on the exact order are in joystick.h
*/
int checkOrder[] =
{
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK
};

float throttle = 0.0f;
float brake = 0.0f;
float steering = 0.0f;
float LY = 0.0f;
float RY = 0.0f;
float RX = 0.0f;

int controlType = 1;

void CheckControllers(void)
{
	XINPUT_STATE state;
	for (int i = 0; i < 4; i++)
	{
		if (connected[i])
			continue;
		if (XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			connected[i] = TRUE;
			states[i] = state;
		}
	}
}


void SignalHandler(int signal)
{
	printf("Signal %d", signal);
	throw "!Access Violation!";
}

void SetThrottle(float t)
{
	throttle = t;
}

void SetBrake(float t)
{
	brake = t;
}

int GetButtonState(int joyBtn, int joyIdx)
{
	bool actuallyPressed = states[joyIdx].Gamepad.wButtons & JoyBtnToXInputBtn(joyBtn);
	if (controlType == ControlType::Menu && joyBtn > 3)
		return actuallyPressed;
	if (joyBtn == JOY_DPAD_DOWN && actuallyPressed)
		MessageBox(NULL, "IT WAS PRESSED YOU BITCH", "DPAD_DOWN", MB_ICONWARNING);

	if (pressed[joyBtn])
	{
		pressed[joyBtn] = actuallyPressed;
		return false;
	}
	if (actuallyPressed)
	{
		pressed[joyBtn] = true;
		return true;
	}
	pressed[joyBtn] = false;
	return false;

	switch (joyBtn)
	{
	case JOY_BTN_A:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_A & !(check.Gamepad.wButtons & XINPUT_GAMEPAD_A);
	case JOY_BTN_B:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_B;
	case JOY_BTN_Y:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_Y;
	case JOY_BTN_X:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_X;
	case JOY_DPAD_DOWN:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
	case JOY_DPAD_LEFT:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
	case JOY_DPAD_RIGHT:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
	case JOY_DPAD_UP:
		return states[joyIdx].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
	}
}

float NormDeadzone(float value, float deadzone)
{
	if (fabs(value) < deadzone)
		return 0.0f;
	return (value > 0 ? (value - deadzone) : (value + deadzone)) * (1.0 / (1.0 - deadzone));
}

void RaiseEvents(void)
{
	XINPUT_STATE state;
	for (int i = 0; i < 4; i++)
	{
		if (!connected[i])
			continue;

		if (XInputGetState(i, &state) != ERROR_SUCCESS)
		{
			connected[i] = FALSE;
			continue;
		}

		float deadzone = 0.2;

		SetThrottle((float)state.Gamepad.bRightTrigger / 255.0);
		SetBrake((float)state.Gamepad.bLeftTrigger / 255.0);

		float normLX = fmaxf(-1, (float)state.Gamepad.sThumbLX / 32767.0);
		float normLY = fmaxf(-1, (float)state.Gamepad.sThumbLY / 32767.0);
		float normRX = fmaxf(-1, (float)state.Gamepad.sThumbRX / 32767.0);
		float normRY = fmaxf(-1, (float)state.Gamepad.sThumbRY / 35767.0);

		steering = NormDeadzone(normLX, deadzone);
		LY = NormDeadzone(normLY, deadzone);
		RX = NormDeadzone(normRX, deadzone);
		RY = NormDeadzone(normRY, deadzone);

		states[i] = state;
	}
}

void joystick_init(void)
{
	CheckControllers();
}

void joystick_cycle(int passed)
{
	timeout += passed;
	if (timeout > CHECK_TIMEOUT)
	{
		timeout -= CHECK_TIMEOUT;
		CheckControllers();
	}
	RaiseEvents();
}

DWORD WINAPI Background(LPVOID unused)
{

	while (true)
	{
		joystick_cycle(1);
		Sleep(1);
	}
}

char rets[]
{
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
	0xC3,
};

int GetSteering()
{
	float result;
	float deadzone = 0.2;
	float normLX = fmaxf(-1, (float)states[0].Gamepad.sThumbLX / 32767);
	if (fabs(normLX) <= deadzone)
		result = 0;
	else
		result = (normLX > 0 ? (normLX - deadzone) : (normLX + deadzone)) * (1.0 / (1.0 - deadzone));
	return *(int*)(&result);
}

void FixCrashOnCalendar()
{
	char crashFix[] =
	{
		0xB0, 0x00, 0x90
	};
	injector::WriteMemoryRaw(0x004C14CF, crashFix, sizeof(crashFix), true); // Fix crash on calendar
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

	if (a == 1)
	{
		__asm pop ecx
		return throttle;
	}
	else if (a == 0)
	{
		__asm pop ecx
		return steering;
	}
	else if(a == 2)
	{
		__asm pop ecx
		return brake;
	}
	else if (a == 3)
	{
		__asm pop ecx
		return RX;
	}
	else if (a == 4)
	{
		__asm pop ecx
		return RX;
	}
	return 0.0f;
}

void RaceEnd()
{
	SetControlType(nullptr, ControlType::Menu);
}

void RaceStart()
{
	SetControlType(nullptr, ControlType::Race);
}

int ProcessMenuInput(int key)
{
	switch (key)
	{
	case MenuButtons::Accept:
		return GetButtonState(JOY_BTN_A, 0);
	case MenuButtons::Back:
		return GetButtonState(JOY_BTN_B, 0);
	case MenuButtons::Tab:
		return GetButtonState(JOY_BTN_Y, 0);
	case MenuButtons::PageDown:
		return GetButtonState(JOY_BTN_X, 0);
	case MenuButtons::Down:
	case MenuButtons::DownDigital:
		return GetButtonState(JOY_DPAD_DOWN, 0) | LY < 0 ? 1 : 0;
	case MenuButtons::Up:
	case MenuButtons::UpDigital:
		return GetButtonState(JOY_DPAD_UP, 0) | LY > 0 ? 1 : 0;
	case MenuButtons::Left:
	case MenuButtons::LeftDigital:
		return GetButtonState(JOY_DPAD_LEFT, 0) | steering < 0 ? 1 : 0;
	case MenuButtons::Right:
	case MenuButtons::RightDigital:
		return GetButtonState(JOY_DPAD_RIGHT, 0) | steering > 0 ? 1 : 0;
	/*case MenuButtons::LeftDigital:
		return GetButtonState(JOY_DPAD_LEFT, 0);
	case MenuButtons::RightDigital:
		return GetButtonState(JOY_DPAD_RIGHT, 0);
	case MenuButtons::DownDigital:
		return GetButtonState(JOY_DPAD_DOWN, 0);
	case MenuButtons::UpDigital:
		return GetButtonState(JOY_DPAD_UP, 0);*/
	case MenuButtons::Menu15:
		return GetButtonState(JOY_BTN_LS, 0);
	case MenuButtons::Menu14:
		return GetButtonState(JOY_BTN_RS, 0);
	}
	return 0;
}

int ProcessRaceInput(int key)
{
	switch (key)
	{
	case RaceButtons::ChangeView:
		return GetButtonState(JOY_BTN_BACK, 0);
	case RaceButtons::GearUp:
		return GetButtonState(JOY_BTN_A, 0);
	case RaceButtons::GearDown:
		return GetButtonState(JOY_BTN_B, 0);
	case RaceButtons::Horn:
		return GetButtonState(JOY_DPAD_UP, 0);
	case RaceButtons::Pause:
		return GetButtonState(JOY_BTN_START, 0);
	case RaceButtons::LookBack:
		return GetButtonState(JOY_BTN_RT, 0);
	case RaceButtons::Race0:
		return GetButtonState(JOY_BTN_LS, 0);
	case RaceButtons::Race9:
		return GetButtonState(JOY_BTN_RS, 0);

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

bool PatchInput = false;

void ReadConfig()
{
	CIniReader iniReader("fixes.ini");

	PatchInput = iniReader.ReadBoolean("Fixes", "PatchInput", false);
}

int WINAPI DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		for (int i = 0; i < 14; i++)
			pressed[i] = false;
		ReadConfig();
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

		char dis[] =
		{
			0x90, 0x90, 0x90
		};

		char pushEdi[] =
		{
			0x57
		};

		controlType = ControlType::Menu;
		FixCrashOnCalendar();
		FixVirtualMemory();

		if (PatchInput)
		{
			injector::MakeJMP(0x00401640, GetAxle);
			injector::MakeJMP(0x004015D0, GetButton);
			injector::MakeCALL(0x0046175E, RaceStart);	// Switch control type for race
			injector::MakeCALL(0x00450B3B, RaceEnd);	// Switch control type for race end
			injector::MakeCALL(0x005BB0FE, SetControlType); // Switch control type for pause menu
		}

		CreateThread(0, 0, Background, NULL, 0, NULL);
		MessageBoxA(NULL, "XInput support added!", "XI4J", MB_ICONINFORMATION);
	}
	return TRUE;
}