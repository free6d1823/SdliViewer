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
#include "common.h"
#include "ImageWin.h"
#include "ProcessEvents.h"

 
int gPosX = 0;
int gPosY = 0;
double gScale = 1.0; 
static UiCommand CheckWindowEvents(SDL_Event* event)
{
    UiCommand cmd = COMMAND_NONE;
       switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_HIDDEN:
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_MOVED:
        case SDL_WINDOWEVENT_RESIZED:
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESTORED:
            if(gWin[0]) gWin[0]->update(true);
            if(gWin[1]) gWin[1]->update(true);
            break;
        case SDL_WINDOWEVENT_ENTER:
//            SDL_Log("Mouse entered window %d",
//                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
//            SDL_Log("Mouse left window %d", event->window.windowID);
//            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
//            SDL_Log("Window %d gained keyboard focus",
//                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
//            SDL_Log("Window %d lost keyboard focus",
//                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            {
                SDL_Window* pWin = SDL_GetWindowFromID(event->window.windowID); 
                if(gWin[0] && pWin == gWin[0]->getWindow())
                    cmd = COMMAND_EXIT;
                if(gWin[1] && pWin == gWin[1]->getWindow()) {
                    gWin[1] = NULL;
                    SDL_DestroyWindow(pWin);
                }
            }
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
            break;
        }
    return cmd;
}

typedef struct _CommandTable {
    const char* key;
    const char* description;
}CommandTable;

CommandTable command_table[] = {
    {"q", "Quit"},
    {"X", "Quit"},
    {"?", "list command manual"},
    {"in ", "open input file"},
    {"ref ", "open reference file"},
    {"dif ", "show difference of two file"},
    {"+", "zoom in"},
    {"-", "zoom out"},
    {"<arrow>", "move picture"}
};

static UiCommand ProcessKeyCommand(SDL_Keysym key)
{
    UiCommand cmd = COMMAND_NONE;

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
        case 0x3d://"+" SDLK_PLUS=0x2b
            if (key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                gScale = 2;
            else
                 gScale = 1.1;
            cmd = COMMAND_SCALE;
            break;
        case 0x2f: //? SDLK_QUESTION=0x3f
        case SDLK_QUESTION:
            cmd = COMMAND_HELP;
            break;
        default:
            break;
    }
    //printf("Key command =%x, 0x%x, 0x%x SDLK_QUESTION=%x\n", cmd, key.sym, key.mod,SDLK_QUESTION);
    return cmd;
}

UiCommand GetEventMessage()
{
    UiCommand cmd = COMMAND_NONE;   
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
            cmd = CheckWindowEvents(&e);
            break;
	    case SDL_MOUSEBUTTONDOWN:
	    default:
		    break;
	    }	

    }
    return cmd;
}
void ProcessCommand(UiCommand cmd)
{
    switch (cmd)
    {
        case COMMAND_MOVE:
            if(gWin[0]) gWin[0]->moveImage(gPosX, gPosY);
            if(gWin[1]) gWin[1]->moveImage(gPosX, gPosY);

            gPosX = gPosY = 0;
            break;    
        case COMMAND_SCALE:
            if(gWin[0]) gWin[0]->scaleImage(gScale);
            if(gWin[1]) gWin[1]->scaleImage(gScale);

            gScale = 1.0;
            break;
        case COMMAND_HELP:
            for (size_t i=0; i< sizeof(command_table)/sizeof(command_table[0]); i++ ) {
                printf("%s\t%s\n", command_table[i].key, command_table[i].description);
            }    
        default:
            break;
       
    }
}

