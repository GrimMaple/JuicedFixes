#pragma once


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

enum AxlesRace
{
	Steering = 0,
	Throttle = 1,
	Brake = 2,
	Handbrake = 3,
	Reverse = 4,
	LookAround = 5,
	LookAroundY = 6
};

enum AxlesMenu
{
	LookLeft = 2,
	LookDown = 3,
	LookUp = 4,
	LookRight = 5
};

enum MenuButtons
{
	/* Missing stuff:
	3 -- has no idea what it does
	9 -- shows drivers names?
	13 -- IDK

	*/
	Left = 0,//	ALSO
	LeftDigital = 1,
	Right = 2, // ALSO CHANGE VIEW
	RightDigital = 3,
	Up = 4, // ALSO HORN
	UpDigital = 5,
	Down = 6, // ALSO GEAR DOWN
	DownDigital = 7,
	Accept = 8, //	--ACCEPT
	Menu9 = 9,
	PageDown = 10, //	--PGDN(option)
	Tab = 11, //	--TAB(option)
	Back = 12, //	--BACK
	Back2 = 13,
	Menu14 = 14,
	Menu15 = 15
	//Pause = 1,
	//LookBack = 5,
	//GearUp = 7,
};

enum RaceButtons
{
	Race0 = 0,
	Pause = 1,
	ChangeView = 2,
	Nitro = 3,
	Horn = 4,
	LookBack = 5,
	GearDown = 6,
	GearUp = 7,
	Skip = 8,
	Race9 = 9,
	Race10 = 10,
	Race11 = 11,
	Race12 = 12,
	Race13 = 13,
	Race14 = 14,
	Race15 = 15
};

enum ControlType
{
	Menu = 1,
	Race = 0
};

class IInput
{
public:
	void SetControlType(ControlType type)
	{
		controlType = type;
	}
protected:
	bool SinglePress(int keyCode);

private:
	ControlType controlType;
};