#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "common.h"
#include "ProcessEvents.h"
#include "ImageWin.h"
#include "inifile/inifile.h"

static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tDisplay frame buffer image\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s \n\n", name);
    printf("DESCRIPTION\n");
    printf("\tDisplay the frame buffer in a window.\n");
    printf("\tSet configurations in %s.conf\n", name);
	printf("\tColor format code:\n");
	PrintPixelFormat("\t\t");
    printf("\n\n");
}

void SdlInfo()
{
    SDL_version a;
    SDL_VERSION(&a);
    printf("SDL compiled version - %d.%d.%d\n", a.major, a.minor, a.patch);
}

ImageWin* gWin = NULL;
typedef enum _RAW_FORMAT {
	RF_NONE = 0,
	RF_BIN = 1,
	RF_HEX = 2,
	RF_HEX2 = 3,
}RAW_FORMAT;

typedef struct _fbwSysConf {
	int frameWidth;
	int frameHeight;
	int frameFormat;
	int srcX;
	int srcY;
	int srcWidth;
	int srcHeight;
	int srcFormat;
	char srcFile[256];
	RAW_FORMAT raw;
	char* plana[3]; /* filename of 3 plans, channela*/
	char* planb[3];
	int delay; //flash delay
}fbwSysConf;
fbwSysConf sysConf = { 
/*frame buffer info */
	.frameWidth = 640,
	.frameHeight = 480,
	.frameFormat = 5,
	.srcX = 0,
	.srcY = 0,
	.srcWidth = 640,
	.srcHeight = 480,
	.srcFormat = 0,
    .srcFile = {0},
	.raw = RF_NONE,
	.plana = {0,0,0},
	.planb = {0,0,0},

	.delay = 10000
};

unsigned char char2byte(char data)
{
    if(data >= '0' && data <= '9')
        return data - '0';
    if(data >= 'a' && data <= 'f')
        return data - 'a' + 10;
    if (data >= 'A' && data <= 'F')
        return data - 'A' +10;
    return 0;
}
#define HEX_2_BYTE(lo,hi) ((char2byte(hi)<<4)+ char2byte(lo))
void Hex2Bin32(char* source, unsigned char* data)
{
	if (strlen(source) <8) 
		return;
	data[3] = HEX_2_BYTE(source[1], source[0]);
	data[2] = HEX_2_BYTE(source[3], source[2]);
	data[1] = HEX_2_BYTE(source[5], source[4]);
	data[0] = HEX_2_BYTE(source[7], source[6]);	
}
void Hex2Bin16(char* source, unsigned char* data)
{
	if (strlen(source) <4) 
		return;
	data[1] = HEX_2_BYTE(source[1], source[0]);
	data[0] = HEX_2_BYTE(source[3], source[2]);

}
void* CreateBufferFromHex(int* pLen)
{
	int len = GetImageBufferLength(sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.srcFormat);
	unsigned char* pb = (unsigned char *)pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
		FILE* fp = fopen(sysConf.plana[i], "rb");
		unsigned char* p = pb;
		if(fp) {
			char line[256];
			//unsigned char data[4];
			int processed = 0;
			while(fgets(line, sizeof(line), fp))
			{
				if (line[0]=='@')
					continue;
				Hex2Bin32(line, (unsigned char*) p);
				
				p+=4;
				processed += 4;
				if (processed >= n)
					break;
			}
			fclose(fp);
			        printf("--- pp = %d\n", processed);

		} else {
			fprintf(stderr, "Failed to open file %s\n", sysConf.plana[i]);
		}
		pb += n;
	}
	return pBuffer;
}
void* CreateBufferFromHex2(int* pLen)
{
	int len = GetImageBufferLength(sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.srcFormat);
	unsigned char* pb = (unsigned char *)pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
		FILE* fpa = fopen(sysConf.plana[i], "rb");
		FILE* fpb = fopen(sysConf.planb[i], "rb");
		unsigned char* p = pb;		
 	
		if(fpa && fpb) {
 		
			char linea[64];
			char lineb[64];			
			//unsigned char data[4];
			int processed = 0;
			
			while (fgets(linea, sizeof(linea), fpa) && fgets(lineb, sizeof(lineb), fpb))  
			{
				if (linea[0]=='@' || lineb[0] == '@')
					continue;
		
				Hex2Bin16(linea, p);
				p+=2;
				Hex2Bin16(lineb, p);
				p+=2;
				processed += 4;
				if (processed >= n)
					break;
			}
 		
			fclose(fpa);
			fclose(fpb);
		}else {
			fprintf(stderr, "Failed to open %s or %s.\n", sysConf.plana[i], sysConf.planb[i]);
		}
		pb += n;
	}
	return pBuffer;
}
void* CreateBufferFromBin(int* pLen)
{
	int len = GetImageBufferLength(sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.srcFormat);
	void* p = pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	printf("plan=%d n=%d\n", i, n);
		FILE* fp = fopen(sysConf.plana[i], "rb");
		if(fp) {
			int r = fread(p, 1, n, fp);
			p = ((char*)p) + n;
		}
		fclose(fp);
	}
	return pBuffer;
}
void ReloadImage()
{
	ImageFormat* pImage = NULL;
	void* buffer;
	int length = 0;
	if(sysConf.raw == RF_BIN) {
		//combine files to one buffer
		buffer = CreateBufferFromBin(&length);
		if (length > 0)
			pImage = CreateImageBuffer(buffer, length, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	} else if(sysConf.raw == RF_HEX) {
		buffer = CreateBufferFromHex(&length);
		if (length > 0)
			pImage = CreateImageBuffer(buffer, length, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
 
	}else if(sysConf.raw == RF_HEX2) {
 		buffer = CreateBufferFromHex2(&length);
		if (length > 0)
			pImage = CreateImageBuffer(buffer, length, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
 
	}
	else if(sysConf.srcFile[0]) {
		pImage = CreateImageFile(sysConf.srcFile, sysConf.srcWidth, sysConf.srcHeight, sysConf.srcFormat);
	}
	if (pImage) {
		//gWin->setImage(pImage);
		gWin->putImage(sysConf.srcX, sysConf.srcY, pImage);
		 DistroyImage(pImage);
	}
	

}
void FreeConfigure()
{
	for (int i=0;i<3; i++) {
		if (sysConf.plana[i] ) free(sysConf.plana[i]);
        if (sysConf.planb[i] ) free(sysConf.planb[i]);
	}
	memset(&sysConf, 0, sizeof(sysConf));
}
int ReadConfigure(char* file)
{
	char* conf = (char*) malloc(strlen(file)+5);
	strcpy(conf, file);
	strcat(conf,".conf");
	printf("Read configure file %s...\n", conf);
	void* h = openIniFile(conf, true);
	if (h) {
		char value[32];
		sysConf.frameWidth = GetProfileInt("display", "width", sysConf.frameWidth, h);
		sysConf.frameHeight = GetProfileInt("display", "height", sysConf.frameHeight, h);
		if ( GetProfileString("display", "format",  value, sizeof(value), "RGBA", h)) {
			sysConf.frameFormat = GetPixelFormat(value);
		}
		//layer profile
		sysConf.srcWidth = GetProfileInt("layer0", "width", sysConf.srcWidth, h);
        sysConf.srcHeight = GetProfileInt("layer0", "height", sysConf.srcHeight, h);
        sysConf.srcX = GetProfileInt("layer0", "x", sysConf.srcX, h);
        sysConf.srcY = GetProfileInt("layer0", "y", sysConf.srcY, h);
        if ( GetProfileString("layer0", "format",  value, sizeof(value), "I420", h)) {
            sysConf.srcFormat = GetPixelFormat(value);
        }
		 GetProfileString("layer0", "file",  sysConf.srcFile, sizeof(sysConf.srcFile), "", h);
		//plan info
		sysConf.raw = (RAW_FORMAT)GetProfileInt("layer0", "raw", sysConf.srcY, h);
		char plan[256];
		char key[32]="plan1a";
		switch (sysConf.raw) {
			case 3: //2 channels
				key[4] = '1'; key[5]='b';
				for( int i = 0; i < 3; i++) {
				 	if (GetProfileString("layer0", key,  plan, sizeof(plan), "", h))
					 	sysConf.planb[i] = strdup(plan);
					key[4] ++;
				}
			case 1:
			case 2:
				//single channel
				key[4] = '1'; key[5]='a';
				for( int i = 0; i < 3; i++) {			
				 	if (GetProfileString("layer0", key,  plan, sizeof(plan), "", h))
					 	sysConf.plana[i] = strdup(plan);
					key[4] ++;
				}
				break;
			default:
				break;
		}

		 closeIniFile(h);
	}
	free(conf);
	return 0;
}
int main(int argc, char* argv[])
{
    Uint32 tStart;
    
    SdlInfo();
	if( ReadConfigure(argv[0]))
	{
		usage(argv[0]);
		exit(1);
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "SDL_Init Error: %s", SDL_GetError());
		return 1;
	}

    gWin = ImageWin::Create(sysConf.frameWidth, sysConf.frameHeight, sysConf.frameFormat, 0xff440000);

    if (!gWin){
 	    SDL_Quit();
        return -1;
    }
    gWin->update(true);

    UiCommand cmd = COMMAND_NONE;

//    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
    tStart = SDL_GetTicks();
	int reload = 0;
    while ((cmd = GetEventMessage()) != COMMAND_EXIT ) {
        if (cmd != COMMAND_NONE) {
            ProcessCommand(cmd);
            if (gWin) gWin->update();
        }
		if(reload%1000 == 0 ) {
			ReloadImage();
			if (gWin) gWin->update();
		}
		reload++;
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        int tDif = 10 - (SDL_GetTicks() - tStart);
        if(tDif < 0) tDif= 0;
        tStart = SDL_GetTicks();
        SDL_Delay(tDif);        
    }
	FreeConfigure();
    printf("Exit program!\n");
    if (gWin) {
    	delete gWin;
    }	

    SDL_Quit();
	return 0;
}
