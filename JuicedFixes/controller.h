#pragma once

#include <array>
#include <atomic>
#include <thread>

#include <Windows.h>
#include <Xinput.h>

enum ControllerButtons : int
{
    Y = 0,
    X,
    A,
    B,
    DPadDown,
    DPadUp,
    DPadLeft,
    DPadRight,
    LeftShoulder,
    RightShoulder,
    LeftThumb,
    RightThumb,
    Start,
    BackButton,
    LeftTrigger,
    RightTrigger,
    LeftThumbX,
    LeftThumbY,
    RightThumbX,
    RightThumbY,
    End
};

class Controllers
{
public:
    Controllers()
    {
        RefreshControllers();
    }
    Controllers(const Controllers& other) = delete;
    Controllers(const Controllers&& other) = delete;

    void Tick(int passed);

    float getValue(int controllerId, ControllerButtons btn) const;
    bool getPressed(int controllerId, ControllerButtons btn) const;

private:
    void RefreshControllers();
    struct ControllerInfo
    {
        bool connected;
        std::array<bool, 14> buttons;
        std::array<float, 6> axles;
        XINPUT_STATE state;

    };

    int timeout = 0;
    std::atomic_bool exiting = false;

    mutable std::array<ControllerInfo, 4> controllers;
};
