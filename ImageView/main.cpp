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
    SDL_version b;

    SDL_VERSION(&a);
    SDL_GetVersion(&b);
    printf("SDL compiled version - %d.%d.%d\n", a.major, a.minor, a.patch);
    printf("SDL   linked version - %d.%d.%d\n", b.major, b.minor, b.patch);
}

ImageWin* gIw1 = NULL;
ImageWin* gIw2 = NULL;
void ProcessCommand(KeyCommand cmd)
{
    switch (cmd)
    {
        case COMMAND_MOVE:
            gIw1->moveImage(gPosX, gPosY);
            gPosX = gPosY = 0;
            break;    
        case COMMAND_SCALE:
            gIw1->scaleImage(gScale);
            gScale = 1.0;
            break;    
        default:
            break;
       
    }
}
int main(int argc, char* argv[])
{
    int ch;
    int width = 0;;
    int height = 0;
    AVPixelFormat fmt = AV_PIX_FMT_NONE;
    char* image1Filename = NULL;

    SdlInfo();

    while ((ch = getopt(argc, argv, "w:h:f:?")) != -1)
    {
        switch (ch) {
        case 'w':
            width = atoi (optarg);
            break;
        case 'h':
            width = atoi (optarg);
            break;
        case 'f':
            fmt = GetFormateByName(optarg);
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

    ImageFormat* pImg = CreateImageFileByName(image1Filename);
    if (!pImg) {
        if (AV_PIX_FMT_NONE == fmt)
            fprintf(stderr, "Failed to load file %s!\n", image1Filename);
        else {
            pImg = CreateImageFile(image1Filename, width, height, fmt);
        }        
    }
    if (!pImg ) {
        usage(argv[0]);
        exit(-1);
    }
    
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "SDL_Init Error: %s", SDL_GetError());
		return 1;
	}
    int dWidth = pImg->width;
    int dHeight = pImg->height;
    if (dWidth>1024) {
        dWidth /= 2;
        dHeight /=2;
    }
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    gIw1 = ImageWin::CreateImageWin("Hello", 10, 10, dm.w/2, dm.w/2);
    if (!gIw1|| !gIw1->setImage(pImg)){
 	    SDL_Quit();
        return -1;
    }
    
    KeyCommand cmd = COMMAND_NONE;

gPosX = 100;
gPosY = 100;
cmd = COMMAND_MOVE;

int i=1000;
    while ((cmd = GetEventMessage()) != COMMAND_EXIT ) {
//    for (int i=0; i< 100; i++) {    
        if (cmd == COMMAND_NONE)
            SDL_Delay(10);
        else
            ProcessCommand(cmd);
        SDL_Delay(100);
//gScale = 1.01;
//cmd = COMMAND_SCALE;
i--;
        gIw1->update();
printf("--- %d ---cmd=%d\n", i, cmd);
if (i==0)
    break;
    }

    printf("Exit program!\n");
    if (gIw1)
        delete gIw1;
	SDL_Quit();
	return 0;
}
