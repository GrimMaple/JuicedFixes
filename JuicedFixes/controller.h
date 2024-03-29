#pragma once

#include <array>
#include <atomic>
#include <thread>

#include <Windows.h>
#include <Xinput.h>

enum ControllerButtons : int
{
    Y = 0,
    X = 1,
    A = 2,
    B = 3,
    DPadDown = 4,
    DPadUp = 5,
    DPadLeft = 6,
    DPadRight = 7,
    LeftShoulder = 8,
    RightShoulder = 9,
    LeftThumb = 10,
    RightThumb = 11,
    Start = 12,
    BackButton = 13,
    LeftTrigger = 14,
    RightTrigger = 15,
    LeftThumbX = 16,
    LeftThumbY = 17,
    RightThumbX = 18,
    RightThumbY = 19,
    End = 20
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
    bool getSinglePress(int controllerId, ControllerButtons btn) const;

private:
    void RefreshControllers();
    struct ControllerInfo
    {
        bool connected;
        std::array<bool, 14> buttons;
        std::array<float, 6> axles;
        XINPUT_STATE state;

        void setButton(ControllerButtons btn, bool state)
        {
            if (buttonsCache[btn] != state)
            {
                buttons[btn] = state;
            }
            buttonsCache[btn] = state;
        }

    private:
        /// <summary>
        /// Cached buttons for "singlePress" mechanism
        /// </summary>
        std::array<bool, 14> buttonsCache;
    };

    int timeout = 0;
    std::atomic_bool exiting = false;

    mutable std::array<ControllerInfo, 4> controllers;
};
