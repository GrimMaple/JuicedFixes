#include "controller.h"

#include <atomic>

#include <Winbase.h>
#include "settings.h"

namespace
{
    float TriggerToFloat(int value)
    {
        return static_cast<float>(value) / 255.0f;
    }

    float ThumbToFloat(int value)
    {
        return fmaxf(-1, static_cast<float>(value) / 32767.0);
    }

    float NormDeadzone(float value)
    {
        const float deadzone = Deadzone();
        if (fabs(value) < deadzone)
            return 0.0f;
        return (value > 0 ? (value - deadzone) : (value + deadzone)) * (1.0 / (1.0 - deadzone));
    }

    int JoyBtnToXInputBtn(ControllerButtons joyBtn)
    {
        switch (joyBtn)
        {
        case ControllerButtons::A:
            return  XINPUT_GAMEPAD_A;
        case ControllerButtons::B:
            return XINPUT_GAMEPAD_B;
        case ControllerButtons::Y:
            return XINPUT_GAMEPAD_Y;
        case ControllerButtons::X:
            return XINPUT_GAMEPAD_X;
        case ControllerButtons::DPadDown:
            return XINPUT_GAMEPAD_DPAD_DOWN;
        case ControllerButtons::DPadLeft:
            return XINPUT_GAMEPAD_DPAD_LEFT;
        case ControllerButtons::DPadRight:
            return XINPUT_GAMEPAD_DPAD_RIGHT;
        case ControllerButtons::DPadUp:
            return XINPUT_GAMEPAD_DPAD_UP;
        case ControllerButtons::Start:
            return XINPUT_GAMEPAD_START;
        case ControllerButtons::BackButton:
            return XINPUT_GAMEPAD_BACK;
        case ControllerButtons::LeftShoulder:
            return XINPUT_GAMEPAD_LEFT_SHOULDER;
        case ControllerButtons::RightShoulder:
            return XINPUT_GAMEPAD_RIGHT_SHOULDER;
        case ControllerButtons::LeftThumb:
            return XINPUT_GAMEPAD_LEFT_THUMB;
        case ControllerButtons::RightThumb:
            return XINPUT_GAMEPAD_RIGHT_THUMB;
        }
        return 0;
    }
}

void Controllers::RefreshControllers()
{
    XINPUT_STATE state;
    for (int i = 0; i < 4; i++)
    {
        if (controllers[i].connected)
            continue;
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
        {
            controllers[i].connected = true;
            controllers[i].state = state;
        }
    }
}

void Controllers::Tick(int passed)
{
    passed += timeout;
    if (timeout > 1000)
    {
        RefreshControllers();
        timeout -= 1000;
    }
    XINPUT_STATE state;
    for (int i = 0; i < 4; i++)
    {
        if (!controllers[i].connected)
            continue;

        if (XInputGetState(i, &state) != ERROR_SUCCESS)
        {
            controllers[i].connected = FALSE;
            continue;
        }

        float deadzone = 0.2;
        controllers[i].axles[0] = TriggerToFloat(state.Gamepad.bLeftTrigger);
        controllers[i].axles[1] = TriggerToFloat(state.Gamepad.bRightTrigger);
        controllers[i].axles[2] = NormDeadzone(ThumbToFloat(state.Gamepad.sThumbLX));
        controllers[i].axles[3] = NormDeadzone(ThumbToFloat(state.Gamepad.sThumbLY));
        controllers[i].axles[4] = NormDeadzone(ThumbToFloat(state.Gamepad.sThumbRX));
        controllers[i].axles[5] = NormDeadzone(ThumbToFloat(state.Gamepad.sThumbRY));

        controllers[i].state = state;
    }
}

float Controllers::getValue(int controllerId, ControllerButtons btn) const
{
    if (!controllers[controllerId].connected || btn >= ControllerButtons::End)
        return 0;

    controllers[controllerId].setButton(btn, (controllers[controllerId].state.Gamepad.wButtons & JoyBtnToXInputBtn(btn)) != 0);

    if (btn >= ControllerButtons::LeftTrigger)
    {
        return controllers[controllerId].axles[btn - ControllerButtons::LeftTrigger];
    }
    return controllers[controllerId].buttons[btn] ? 1.0f : 0.0f;
}

bool Controllers::getPressed(int controllerId, ControllerButtons btn) const
{
    return getValue(controllerId, btn) > 0.5f;
}

bool Controllers::getSinglePress(int controllerId, ControllerButtons btn) const
{
    bool ret = getPressed(controllerId, btn);
    controllers[controllerId].buttons[btn] = false;
    return ret;
}
