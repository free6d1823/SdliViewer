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
            SDL_Log("Mouse left window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    event->window.windowID);
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
                 gScale = 0.9;
            cmd = COMMAND_SCALE;
            break;
        case 0x3d://"+"
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gScale = 2;
            else
                 gScale = 1.1;
            cmd = COMMAND_SCALE;
            break;
        default:
            break;
    }
    //printf("Key command =%x, 0x%x, 0x%x SDLK_PLUS=%x\n", cmd, key.sym, key.mod,SDLK_PLUS);
    return cmd;
}
#include <termio.h>

int GET_CHAR()
{
    struct termios tm, tm_old;
    int fd = 0, ch;

    if ( tcgetattr(fd, &tm)<0 )
      return -1;
    tm_old =tm;
    cfmakeraw(&tm);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
      return -1;

    ch = getchar();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
      return -1;
    return ch;
}
KeyCommand GetEventMessage4()
{
    KeyCommand cmd = COMMAND_NONE;    
    const int key = GET_CHAR();
    if (key == 'q')
        cmd = COMMAND_EXIT;
    else if (key == 0x1b) {
        gPosX = -1;
        cmd = COMMAND_MOVE;
    }
    return cmd;
}
KeyCommand GetEventMessage3()
{
    KeyCommand cmd = COMMAND_NONE;    

    char line[32];
 
    while (fgets(line, 32, stdin)) {
        //check hot key
        switch(line[0]) {
            case 0x1b: 
            {
                if( 0 == memcmp(line+1, "[A", 2))
                {
                    gPosY = 1;
                    cmd = COMMAND_MOVE;
                } else if ( 0 == memcmp(line+1, "[B", 2))
                {
                    gPosY = -1;
                    cmd = COMMAND_MOVE;
                } 
                else if ( 0 == memcmp(line+1, "[D", 2)) { //left arrow
                    gPosX = -1;
                    cmd = COMMAND_MOVE;
                } 
                else if ( 0 == memcmp(line+1, "[C", 2)) { //right arrow
                    gPosX = 1;
                    cmd = COMMAND_MOVE;
                }
                break; 
            }
            case '=':
            case '+':
                gScale = 1.1;
                cmd = COMMAND_SCALE;
                break;
            case '-':
            case '_':                         
                gScale = 0.8;
                cmd = COMMAND_SCALE;
                break;
            default:
                break;
        }
        if (cmd != COMMAND_NONE)
            break;
        if ( 0 == memcmp(line, "quit", 4)) {
            cmd = COMMAND_EXIT;
        } else if ( 0 == memcmp(line, "exit", 4)) {
            cmd = COMMAND_EXIT;
        } else {
            printf ("No command - %s, (%x,%x%c,%x%c)\n", line, line[0], line[1],line[1], line[2],line[2]);
        }
        break;
    }
    return cmd;

}
KeyCommand GetEventMessage()
{
    KeyCommand cmd = COMMAND_NONE;   
    SDL_Event e; 
    SDL_PumpEvents();
    int r = SDL_PeepEvents(&e, 1, SDL_GETEVENT,0x100, SDL_TEXTINPUT);
    if (r > 0) {
	    switch(e.type) {
        case SDL_MOUSEMOTION: //0x400
            break;
	    case SDL_QUIT:      //0x100
		    return COMMAND_EXIT;
	    case SDL_KEYDOWN:
            cmd = ProcessKeyCommand(e.key.keysym);
            break; 
        case SDL_WINDOWEVENT: //0x200
            ShowWindowEvents(&e);
            break;
	    case SDL_MOUSEBUTTONDOWN:
	    default:
		    break;
	    }	

    }
    return cmd;
}
KeyCommand GetEventMessage2s()
{
 
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
            ShowWindowEvents(&e);
            break;
	    case SDL_MOUSEBUTTONDOWN:
    		return COMMAND_EXIT;
	    default:
		    break;
	    }	
    }	
	return cmd;

}
