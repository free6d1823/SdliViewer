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
