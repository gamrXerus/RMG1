/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2025 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef GAMEPADHANDLER_HPP
#define GAMEPADHANDLER_HPP

#include <QObject>
#include <QTimer>
#include <QMap>
#include <SDL3/SDL.h>

namespace UserInterface
{
class GamepadHandler : public QObject
{
    Q_OBJECT

  public:
    GamepadHandler(QObject *parent = nullptr);
    ~GamepadHandler(void);

    bool Init(void);
    void Shutdown(void);

  private:
    QTimer* pollTimer = nullptr;
    QMap<SDL_JoystickID, SDL_Gamepad*> gamepads;
    bool initialized = false;

    // Button state tracking to detect press events for each gamepad
    struct GamepadState {
        bool lastDpadUp = false;
        bool lastDpadDown = false;
        bool lastDpadLeft = false;
        bool lastDpadRight = false;
        bool lastButtonCross = false;
        bool lastButtonGuide = false;
    };
    QMap<SDL_JoystickID, GamepadState> gamepadStates;

    void pollGamepad(SDL_Gamepad* gamepad, SDL_JoystickID id);
    void openAllGamepads(void);

  private slots:
    void on_PollTimer_Timeout(void);

  signals:
    void DpadUpPressed(void);
    void DpadDownPressed(void);
    void DpadLeftPressed(void);
    void DpadRightPressed(void);
    void ButtonCrossPressed(void);  // X button on PS4 controller
    void ButtonGuidePressed(void);   // PlayStation button
};
} // namespace UserInterface

#endif // GAMEPADHANDLER_HPP
