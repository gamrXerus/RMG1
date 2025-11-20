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

    // Open all available gamepads
    openAllGamepads();

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

    // Close all gamepads
    for (auto it = gamepads.begin(); it != gamepads.end(); ++it)
    {
        if (it.value() != nullptr)
        {
            SDL_CloseGamepad(it.value());
        }
    }
    gamepads.clear();
    gamepadStates.clear();

    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    initialized = false;
}

void GamepadHandler::openAllGamepads(void)
{
    // Get list of gamepads
    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);

    if (joysticks != nullptr && count > 0)
    {
        // Open all gamepads
        for (int i = 0; i < count; i++)
        {
            SDL_JoystickID id = joysticks[i];

            // Only open if not already opened
            if (!gamepads.contains(id))
            {
                SDL_Gamepad* pad = SDL_OpenGamepad(id);
                if (pad != nullptr)
                {
                    gamepads[id] = pad;
                    gamepadStates[id] = GamepadState();
                }
            }
        }
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
            // A gamepad was connected - open all gamepads to include the new one
            openAllGamepads();
        }
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            // A gamepad was disconnected
            SDL_JoystickID removedId = event.gdevice.which;
            if (gamepads.contains(removedId))
            {
                SDL_CloseGamepad(gamepads[removedId]);
                gamepads.remove(removedId);
                gamepadStates.remove(removedId);
            }
        }
    }

    // Poll all gamepads
    for (auto it = gamepads.begin(); it != gamepads.end(); ++it)
    {
        if (it.value() != nullptr)
        {
            pollGamepad(it.value(), it.key());
        }
    }
}

void GamepadHandler::pollGamepad(SDL_Gamepad* gamepad, SDL_JoystickID id)
{
    if (gamepad == nullptr || !gamepadStates.contains(id))
    {
        return;
    }

    GamepadState& state = gamepadStates[id];

    // D-pad Up
    bool dpadUp = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    if (dpadUp && !state.lastDpadUp)
    {
        emit DpadUpPressed();
    }
    state.lastDpadUp = dpadUp;

    // D-pad Down
    bool dpadDown = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    if (dpadDown && !state.lastDpadDown)
    {
        emit DpadDownPressed();
    }
    state.lastDpadDown = dpadDown;

    // D-pad Left
    bool dpadLeft = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    if (dpadLeft && !state.lastDpadLeft)
    {
        emit DpadLeftPressed();
    }
    state.lastDpadLeft = dpadLeft;

    // D-pad Right
    bool dpadRight = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    if (dpadRight && !state.lastDpadRight)
    {
        emit DpadRightPressed();
    }
    state.lastDpadRight = dpadRight;

    // Cross button (X on PS4, A on Xbox)
    bool buttonCross = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    if (buttonCross && !state.lastButtonCross)
    {
        emit ButtonCrossPressed();
    }
    state.lastButtonCross = buttonCross;

    // Guide button (PlayStation button on PS4, Xbox button on Xbox)
    bool buttonGuide = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    if (buttonGuide && !state.lastButtonGuide)
    {
        emit ButtonGuidePressed();
    }
    state.lastButtonGuide = buttonGuide;
}
