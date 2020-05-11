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
#include "ProcessEvents.h"

int gPosX = 0;
int gPosY = 0;
double gScale = 1.0; 
static void ShowWindowEvents(SDL_Event* event)
{
       switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_Log("Window %d exposed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            SDL_Log("Window %d moved to %d,%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("Window %d resized to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            SDL_Log("Mouse entered window %d",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
//            SDL_Log("Mouse left window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
//            SDL_Log("Window %d gained keyboard focus",
//                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
//            SDL_Log("Window %d lost keyboard focus",
//                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event->window.windowID);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            SDL_Log("Window %d is offered a focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            SDL_Log("Window %d has a special hit test", event->window.windowID);
            break;
#endif
        default:
            SDL_Log("Window %d got unknown event %d",
                    event->window.windowID, event->window.event);
            break;
        }
}
#define CMD_QUIT    "q"
#define CMD_QUIT2    "x"

typedef struct _CommandTable {
    const char* key;
    const char* description;
}CommandTable;

CommandTable command_table[] = {
    {"q", "Quit"},
    {"X", "Quit"},
    {"man", "list command manual"},
    {"in ", "open input file"},
    {"ref ", "open reference file"},
    {"dif ", "show difference of two file"},
    {"+", "zoom in"},
    {"-", "zoom out"},
    {"^", "move up"}
};

static KeyCommand ProcessKeyCommand(SDL_Keysym key)
{
    KeyCommand cmd = COMMAND_NONE;
printf("key=%x mod=%d, SDL_SCANCODE_X=%d\n", key.sym, key.mod, SDL_SCANCODE_X);
    switch((key.sym )) {
        case SDLK_x:
        case SDLK_q:

            cmd = COMMAND_EXIT;
            break;
        case SDLK_DOWN:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gPosY = -10;
            else
                gPosY = -1;
            cmd = COMMAND_MOVE;
            break;
        case SDLK_UP:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gPosY = 10;
            else
                gPosY = 1;
            cmd = COMMAND_MOVE;
            break;
        case SDLK_RIGHT:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gPosX = -10;
            else
                gPosX = -1;
            cmd = COMMAND_MOVE;
            break;
        case SDLK_LEFT:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gPosX = 10;
            else
                gPosX = 1;
            cmd = COMMAND_MOVE;
            break;
        case SDLK_MINUS:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gScale = 0.5;
            else
                 gScale = 0.1;
            cmd = COMMAND_SCALE;
            break;
        case SDLK_PLUS:
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gScale = 2;
            else
                 gScale = 1.1;
            cmd = COMMAND_SCALE;
            break;
        default:
            break;
    }
    return cmd;
}
KeyCommand GetEventMessage()
{
#define MAX_COMMAND_LENGTH  9
static char sCommad[256];
static int keyIndex = 0;
    KeyCommand cmd = COMMAND_NONE;    
    SDL_Event e;
    if (SDL_PollEvent(&e)) {

	    switch(e.type) {
        case SDL_MOUSEMOTION: //0x400
            break;
	    case SDL_QUIT:      //0x100
		    return COMMAND_EXIT;
	    case SDL_KEYDOWN:
            return ProcessKeyCommand(e.key.keysym);
        case SDL_WINDOWEVENT: //0x200
            //ShowWindowEvents(&e);
            break;
	    case SDL_MOUSEBUTTONDOWN:
    //		return 0;
	    default:
		    break;
	    }	
    }	
	return cmd;

}
