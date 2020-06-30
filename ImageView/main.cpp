#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "common.h"
#include "ProcessEvents.h"
#include "ImageWin.h"


static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tDisplay image file\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s [-w WIDTH] [-h HEIGHT] [-f FORMAT] FILE\n\n", name);
    printf("DESCRIPTION\n");
    printf("\tDisplay the raw image FILE in a window.\n");
    printf("\tIf the filename is the format of name_widthxheight.ext. It shows the image in widthxheight and ext format.\n\n");
    printf("\t-w WIDTH\n");
    printf("\t\t Width of the image, in pixel.\n\n");
    printf("\t-h HEIGHT\n");
    printf("\t\t Height of the image, in pixel.\n\n");
    printf("\t-s FPS\n");
    printf("\t\t display frequency, frames per second.\n\n");
    printf("\t-f FORMAT\n");
    printf("\t\t Format of the image.\n");
    printf("\t\t - \"yuyv\"\tYUYV 422 8bit packet mode\n");
    printf("\t\t - \"yvyu\"\tYVYU 422 8bit packet mode\n");
    printf("\t\t - \"uyvy\"\tUYVY 422 8bit packet mode\n");
    printf("\t\t - \"i422\"\tYUV 422 8bit plant mode\n");  
    printf("\t\t - \"i420\"\tYUV 420 8bit plant mode\n");  
    printf("\t\t - \"nv12\"\tY-UV 420 half-plant mode\n\n");  

    printf("\t\t - \"rgba\"\tRGBA 32 bits packet mode\n");
    printf("\t\t - \"rgb\"\tRGB 24 bits packet mode\n");
    printf("\t\t - \"bgr\"\tRGB 24 bits packet mode\n");
    printf("\n\n");
}

void SdlInfo()
{
    SDL_version a;
    SDL_VERSION(&a);
    printf("SDL compiled version - %d.%d.%d\n", a.major, a.minor, a.patch);
}

ImageWin* gWin[2] = { NULL, NULL};

int main(int argc, char* argv[])
{
    int ch;
    int width = 0;;
    int height = 0;
    AVPixelFormat fmt = AV_PIX_FMT_NONE;
    char* image1Filename = NULL;
    Uint32 tStart;
    Uint32 tDelay = 30;
    
    SdlInfo();

    while ((ch = getopt(argc, argv, "w:h:f:s:?")) != -1)
    {
        switch (ch) {
        case 'w':
            width = atoi (optarg);
            break;
        case 'h':
            height = atoi (optarg);
            break;
        case 'f':
            fmt = GetFormateByName(optarg);
            printf("fmt=%s %d\n", optarg, fmt);
            break;
        default:
            usage(argv[0]);
            exit(-1);
        }   
    }

    if(optind<argc)
        image1Filename = argv[optind];

    if ( image1Filename == 0) {
        usage(argv[0]);
        exit(-1);
    }
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "SDL_Init Error: %s", SDL_GetError());
		return 1;
	}

    ImageWin* gIw1 = ImageWin::CreateWinByFile(image1Filename, width, height, fmt);
    if (!gIw1){
 	    SDL_Quit();
        return -1;
    }
    gIw1->update(true);
    gWin[0] = gIw1;

    UiCommand cmd = COMMAND_NONE;

//    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
    tStart = SDL_GetTicks();
    while ((cmd = GetEventMessage()) != COMMAND_EXIT ) {
        if (cmd != COMMAND_NONE) {
            ProcessCommand(cmd);
            if (gWin[0]) gWin[0]->update();
            if (gWin[1]) gWin[1]->update();
        }
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        int tDif = tDelay - (SDL_GetTicks() - tStart);
        if(tDif < 0) tDif= 0;
        tStart = SDL_GetTicks();
        SDL_Delay(tDif);        
    }
 
    printf("Exit program!\n");
    for(size_t i=0; i< sizeof(gWin)/sizeof(gWin[0]); i++) {
        if (gWin[i])
            delete gWin[i];
    }	

    SDL_Quit();
	return 0;
}
