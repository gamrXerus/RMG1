/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2025 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "GamepadHandler.hpp"

using namespace UserInterface;

GamepadHandler::GamepadHandler(QObject *parent) : QObject(parent)
{
}

GamepadHandler::~GamepadHandler(void)
{
    Shutdown();
}

bool GamepadHandler::Init(void)
{
    if (initialized)
    {
        return true;
    }

    // Initialize SDL gamepad subsystem
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
    {
        return false;
    }

    // Open the first available gamepad
    openFirstGamepad();

    // Create timer to poll gamepad state
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &GamepadHandler::on_PollTimer_Timeout);
    pollTimer->start(16); // Poll at ~60Hz

    initialized = true;
    return true;
}

void GamepadHandler::Shutdown(void)
{
    if (!initialized)
    {
        return;
    }

    if (pollTimer != nullptr)
    {
        pollTimer->stop();
        delete pollTimer;
        pollTimer = nullptr;
    }

    if (gamepad != nullptr)
    {
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    initialized = false;
}

void GamepadHandler::openFirstGamepad(void)
{
    // Close any existing gamepad
    if (gamepad != nullptr)
    {
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
    }

    // Get list of gamepads
    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);

    if (joysticks != nullptr && count > 0)
    {
        // Open the first gamepad
        gamepad = SDL_OpenGamepad(joysticks[0]);
        SDL_free(joysticks);
    }
}

void GamepadHandler::on_PollTimer_Timeout(void)
{
    // Process SDL events to handle gamepad connections/disconnections
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            // A gamepad was connected
            if (gamepad == nullptr)
            {
                openFirstGamepad();
            }
        }
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            // A gamepad was disconnected
            if (gamepad != nullptr && event.gdevice.which == SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad)))
            {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
                // Try to open another gamepad if available
                openFirstGamepad();
            }
        }
    }

    // Poll gamepad state
    pollGamepad();
}

void GamepadHandler::pollGamepad(void)
{
    if (gamepad == nullptr)
    {
        return;
    }

    // D-pad Up
    bool dpadUp = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    if (dpadUp && !lastDpadUp)
    {
        emit DpadUpPressed();
    }
    lastDpadUp = dpadUp;

    // D-pad Down
    bool dpadDown = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    if (dpadDown && !lastDpadDown)
    {
        emit DpadDownPressed();
    }
    lastDpadDown = dpadDown;

    // D-pad Left
    bool dpadLeft = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    if (dpadLeft && !lastDpadLeft)
    {
        emit DpadLeftPressed();
    }
    lastDpadLeft = dpadLeft;

    // D-pad Right
    bool dpadRight = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    if (dpadRight && !lastDpadRight)
    {
        emit DpadRightPressed();
    }
    lastDpadRight = dpadRight;

    // Cross button (X on PS4, A on Xbox)
    bool buttonCross = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    if (buttonCross && !lastButtonCross)
    {
        emit ButtonCrossPressed();
    }
    lastButtonCross = buttonCross;

    // Guide button (PlayStation button on PS4, Xbox button on Xbox)
    bool buttonGuide = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    if (buttonGuide && !lastButtonGuide)
    {
        emit ButtonGuidePressed();
    }
    lastButtonGuide = buttonGuide;
}
