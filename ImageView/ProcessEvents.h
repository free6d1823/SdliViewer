/**************************************************************************
    SdliViewer Project - simple image viewer based on SDL2
    Copyright (C) 2020  chengjyhchang@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
***************************************************************************/
#include <SDL2/SDL.h>

extern int gPosX;
extern int gPosY;
extern double gScale; 

typedef enum _KeyCommand {
    COMMAND_EXIT = -1,
    COMMAND_NONE = 0,
    COMMAND_MOVE = 1,
    COMMAND_SCALE = 2,
    COMMAND_OPENR_REF = 3,
    COMMAND_DIFF = 4, /*show different window */
}KeyCommand;

KeyCommand GetEventMessage();
