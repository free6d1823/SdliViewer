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
#define MAX_PLANS	3
#define MAX_LAYERS	2
typedef struct _layerConf {
	int x;
	int y;
	int width;
	int height;
	int format;
	RAW_FORMAT raw;
	char* plana[MAX_PLANS];
	char* planb[MAX_PLANS];
}LayerConf;

typedef struct _fbwSysConf {
	int frameWidth;
	int frameHeight;
	int frameFormat;
	int layers;
	LayerConf layer[MAX_LAYERS];
	int delay; //flash delay
}fbwSysConf;
fbwSysConf sysConf = {0};

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
void* CreateBufferFromHex(int k, int* pLen)
{
	int len = GetImageBufferLength(sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.layer[k].format);
	unsigned char* pb = (unsigned char *)pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
		FILE* fp = fopen(sysConf.layer[k].plana[i], "rb");
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
		} else {
			fprintf(stderr, "Failed to open file %s\n", sysConf.layer[k].plana[i]);
		}
		pb += n;
	}
	return pBuffer;
}
void* CreateBufferFromHex2(int k, int* pLen)
{
	int len = GetImageBufferLength(sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.layer[k].format);
	unsigned char* pb = (unsigned char *)pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
		if (sysConf.layer[k].plana[i] && sysConf.layer[k].plana[i][0]!=0
			&& sysConf.layer[k].planb[i] && sysConf.layer[k].planb[i][0]!=0){	

			FILE* fpa = fopen(sysConf.layer[k].plana[i], "rb");
			FILE* fpb = fopen(sysConf.layer[k].planb[i], "rb");
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
				fprintf(stderr, "Failed to open %s or %s.\n", sysConf.layer[k].plana[i], sysConf.layer[k].planb[i]);
			}
		}
		pb += n;
	}
	return pBuffer;
}
void* CreateBufferFromBin(int k, int* pLen)
{
	int len = GetImageBufferLength(sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
	void* pBuffer = malloc(len);
	*pLen = len;
	int plan = GetImagePlanNumbers(sysConf.layer[k].format);
	void* p = pBuffer;
	for(int i=0; i< plan; i++) {
		int n = GetImagePlanLength(i, sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
		if (sysConf.layer[k].plana[i] && sysConf.layer[k].plana[i][0]!=0){	
		printf("open %s\n", sysConf.layer[k].plana[i]);
			FILE* fp = fopen(sysConf.layer[k].plana[i], "rb");
			if(fp) {
				fread(p, 1, n, fp);
			}
			fclose(fp);
		}
		p = ((char*)p) + n;
	}
	return pBuffer;
}
void ReloadImage()
{
	ImageFormat* pImage = NULL;
	void* buffer;
	int length = 0;
	for (int k=0; k < sysConf.layers; k++) {
		length = 0;
		pImage = NULL;
		if (sysConf.layer[k].raw == RF_NONE) {
			pImage = CreateImageFile(sysConf.layer[k].plana[0], 
				sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
		
		} else {
			if(sysConf.layer[k].raw == RF_BIN) {
				buffer = CreateBufferFromBin(k, &length);
			} else if(sysConf.layer[k].raw == RF_HEX) {
				buffer = CreateBufferFromHex(k, &length);
			}else if(sysConf.layer[k].raw == RF_HEX2) {
		 		buffer = CreateBufferFromHex2(k, &length);
			}
			if (length > 0) {
				pImage = CreateImageBuffer(buffer, length, 
					sysConf.layer[k].width, sysConf.layer[k].height, sysConf.layer[k].format);
			}			
		}

		if (pImage) {
			//gWin->setImage(pImage);

			gWin->putImage(sysConf.layer[k].x, sysConf.layer[k].y, pImage);
			DistroyImage(pImage);
		}
	}
	gWin->update();

}
void FreeConfigure()
{
	for(int k=0; k< MAX_LAYERS; k++) {
		for (int i=0;i<MAX_PLANS; i++) {
			if (sysConf.layer[k].plana[i] ) free(sysConf.layer[k].plana[i]);
		    if (sysConf.layer[k].planb[i] ) free(sysConf.layer[k].planb[i]);
		}
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
		sysConf.frameWidth = GetProfileInt("display", "width", 1280, h);
		sysConf.frameHeight = GetProfileInt("display", "height", 720, h);
		if ( GetProfileString("display", "format",  value, sizeof(value), "RGBA", h)) {
			sysConf.frameFormat = GetPixelFormat(value);
		}
		sysConf.delay =  GetProfileInt("display", "delay", 10000, h);
		if (sysConf.delay < 100) sysConf.delay = 100; //avoid zero
		sysConf.layers = GetProfileInt("display", "layers", 0, h);
		char section[32];
		for (int i=0; i<sysConf.layers; i++) {
			sprintf(section, "layer%d", i);
			sysConf.layer[i].x =  GetProfileInt(section, "x", 0, h);
			sysConf.layer[i].y =  GetProfileInt(section, "y", 0, h);
			sysConf.layer[i].width =  GetProfileInt(section, "width", 0, h);			
			sysConf.layer[i].height =  GetProfileInt(section, "height", 0, h);
		    if ( GetProfileString(section, "format",  value, sizeof(value), "I420", h)) {
		        sysConf.layer[i].format = GetPixelFormat(value);
		    }
		    sysConf.layer[i].raw =  (RAW_FORMAT) GetProfileInt(section, "raw", 0, h);
		    char filename[256];
			char key[32]="plan1a";
			int plans = GetImagePlanNumbers(sysConf.layer[i].format);
			switch(sysConf.layer[i].raw) {
				case RF_NONE:
				if (GetProfileString(section, key,  filename, sizeof(filename), "", h))
					sysConf.layer[i].plana[0] = strdup(filename);
				if (!sysConf.layer[i].plana[0] || sysConf.layer[i].plana[0][0]==0)
				 	printf("conf error: wrong in [%s] %s=%s!\n", section, key, filename);
				else
				 	printf("conf error: wrong in [%s] %s=%s!\n", section, key, filename);	
				break;
				case RF_HEX2: //two channel
				key[4] = '1'; key[5]='b';
				for( int k = 0; k < plans; k++) {
				 	if (GetProfileString(section, key,  filename, sizeof(filename), "", h))
					 	sysConf.layer[i].planb[k] = strdup(filename);
					 if (!sysConf.layer[i].planb[k] || sysConf.layer[i].planb[k][0]==0)
					 	printf("conf error: wrong in [%s] %s=%s!\n", section, key, filename);
					key[4] ++;
				}				
				case RF_BIN: //one channel
				case RF_HEX:
				key[4] = '1'; key[5]='a';
				for( int k = 0; k < plans; k++) {
				 	if (GetProfileString(section, key,  filename, sizeof(filename), "", h))
					 	sysConf.layer[i].plana[k] = strdup(filename);
					 if (!sysConf.layer[i].plana[k] || sysConf.layer[i].plana[k][0]==0)
					 	printf("conf error: wrong in [%s] %s=%s!\n", section, key, filename);
					key[4] ++;
				}
				break;
			}
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

    gWin = ImageWin::Create(sysConf.frameWidth, sysConf.frameHeight, sysConf.frameFormat, 0xff880000);

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
		if(reload%sysConf.delay == 0 ) {
			ReloadImage();
			if (gWin) gWin->update();
		}
		reload++;
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        int tDif = 30 - (SDL_GetTicks() - tStart);
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
