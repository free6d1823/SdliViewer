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
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "common.h"
#include "colorconv.h"

PixelFormatTable sPixelFormatTable[] = {
	{AV_PIX_FMT_YUV420P, "I420", "YUV420 plannar, I420", Yuv420p_Rgb32},
	{AV_PIX_FMT_NV12, "NV12", "YUV420 semi-plannar UV NV12", Nv12_Rgb32},
	{AV_PIX_FMT_NV21, "NV21", "YUV420 semi-plannar VU NV21", Nv21_Rgb32},
	{AV_PIX_FMT_YUYV422, "YUYV", "YUYV422 YUYV packet", Yuyv422_Rgb32},
	{AV_PIX_FMT_UYVY422, "UYVY", "UYVY420 UYVY packet", Uyvy422_Rgb32},
	{AV_PIX_FMT_RGBA, "RGBA", "packed RGBA 8:8:8:8, 32bpp", Rgba_Rgb32},
	{AV_PIX_FMT_BGRA, "BGRA", "packed BGRA 8:8:8:8, 32bpp", Bgra_Rgb32},
	{AV_PIX_FMT_RGB24, "RGB24", "RGB24", Rgb24_Rgb32},
	{AV_PIX_FMT_BGR24, "BGR24", "BGR24", Bgr24_Rgb32},
	{AV_PIX_FMT_YVYU422, "YVYU", "YVYU422 YVYU packet", Yvyu422_Rgb32},
    {AV_PIX_FMT_YUV444P, "YUV444", "YUV 8-bits 3 plannar mode", Yuv444p_Rgb32},
    {AV_PIX_FMT_GBRP, "RGB444", "RGB 8-bits 3 plannar mode", Rgb444_Rgb32},
    {AV_PIX_FMT_GRAY8, "GRAY8", "Gray 8-bit one plan", Gray8_Rgb32},
};

void* ConvertImageToRgb32(ImageFormat* pImage)
{
    unsigned char* pRgb = NULL;

    pRgb = (unsigned char*) malloc(pImage->width* 4* pImage->height);
    if (!pRgb) {
        return NULL;
	}
	if(sPixelFormatTable[pImage->colorCode].fnConv)
		sPixelFormatTable[pImage->colorCode].fnConv((unsigned char*)pImage->data,
				pImage->width, pImage->stride, pImage->height, pRgb);

    return pRgb;
}
static ImageFormat* LoadY4MFile(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
            return NULL;
    }

    char line[256];
    bool bParserBody = false;
    ImageFormat* pImg = (ImageFormat*) malloc(sizeof(ImageFormat));
    memset(pImg, 0, sizeof(ImageFormat));
    while(fgets(line, sizeof(line), fp) != NULL){
        char* p1 = strtok(line, " ");
        if(0!= strcmp(p1, "YUV4MPEG2")) {
             break;
        }
        while((p1 = strtok(NULL, " \n")) != NULL){
            switch(p1[0]){
            case 'W':
               pImg->width = atoi(p1+1);
               break;
            case 'H':
               pImg->height = atoi(p1+1);

               break;
            case 'I':
               if (p1[1] == 'p')
                   pImg->interlace = 0;
               else if(p1[1] == 't')
                   pImg->interlace = 1;
               else if(p1[1] == 'b')
                   pImg->interlace = 2;
               break;
            case 'F': //Fdd:nn
                {
                   char* p2 = strchr(p1, ':');
                   if (p2!= NULL){
                       *p2 = 0;
                       int dd = atoi(p1+1);
                       int nn = atoi(p2+1);
                       if (nn>0)
                           pImg->fps = (float)dd/(float)nn;
                   }
                   break;
                }
            case 'C':
                {
                   char* p2 = p1+1;
                   if ( strcmp(p2, "444") == 0) {
                        pImg->colorCode = GetIndexByAVPixelFormat(AV_PIX_FMT_YUV444P);
                        pImg->bitsPerPixel = 24;
                        pImg->stride = ((pImg->width+3)>>2)<<2;
                        pImg->length = pImg->stride * pImg->height * 3;
                   } else if (strcmp(p2, "422") == 0) {
                        pImg->colorCode = GetIndexByAVPixelFormat(AV_PIX_FMT_YUV422P);
                        pImg->bitsPerPixel = 16;
                        pImg->stride = ((pImg->width+3)>>2)<<2;
                        pImg->length = pImg->stride * pImg->height * 2;
                   } else  { //420
                        pImg->colorCode = GetIndexByAVPixelFormat(AV_PIX_FMT_YUV420P);
                        pImg->bitsPerPixel = 12;
                        pImg->stride = ((pImg->width+3)>>2)<<2;
                        pImg->length = pImg->stride * pImg->height * 3/2;
                   }
                   break;
                }
            default:
               break;

           }
           //check next param
       }
       //parser until find FRAME data
       if(fgets(line, sizeof(line), fp) == NULL) {
           break;
       }
       if (strstr(line, "FRAME"))
           bParserBody = true;
       break;

    }

    if (bParserBody) {
        pImg->data = malloc(pImg->length);
        fread(pImg->data, 1, pImg->length, fp);
        fclose(fp);
        return pImg;
    }
    fclose(fp);
    free (pImg);
    return NULL;
}
/* Parser file name for image infomation */
ImageFormat* CreateImageFileByName(const char* filename)
{
    //check ext name
    char ext[8];
    char* p1 = strrchr((char*) filename, '.');
    if (p1)
        strncpy(ext, p1, 7);
    else
        return NULL;
    if (strcmp(ext, ".y4m") == 0)
        return LoadY4MFile(filename);
    else {
        //check widthxheight
        int width, height;
        char value[32];
        char* p2 = strrchr((char*)filename, '_');
        if(!p2)
            return NULL;
        long long length = p1-p2-1;
        memcpy(value, p2+1, (size_t)length);
        value[length] = 0;
        p1 = strrchr(value, 'x');
        if(!p1)
            return NULL;
        *p1 = 0;
        p2 = p1+1;
        width = atoi(value);
        height = atoi(p2);

        if (width <= 0 || height <=0) {
            return NULL;
        }
		int colorIndex = -1;
        if (strcmp(ext, ".yuyv") == 0)
            colorIndex = GetIndexByAVPixelFormat(AV_PIX_FMT_YUYV422);
        else if(strcmp(ext, ".rgba") == 0){
           colorIndex = GetIndexByAVPixelFormat(AV_PIX_FMT_RGBA);
        }
        else if (strcmp(ext, ".yuv") == 0)
            colorIndex = GetIndexByAVPixelFormat(AV_PIX_FMT_YUV420P);

        else if(strcmp(ext, ".rgb") == 0)
            colorIndex = GetIndexByAVPixelFormat(AV_PIX_FMT_RGB24);
        else if(strcmp(ext, ".bgr") == 0)
            colorIndex = GetIndexByAVPixelFormat(AV_PIX_FMT_BGR24);
        if (colorIndex >=0)
            return CreateImageFile(filename, width, height, colorIndex);
    }
    return NULL;
}
int GetImagePlanNumbers(int colorIndex)
{
	int plan = 0;
	switch(sPixelFormatTable[colorIndex].fmt) {
    case AV_PIX_FMT_YVYU422:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_UYVY422:
    case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
    case AV_PIX_FMT_GRAY8:
        plan = 1;
        break;
    case AV_PIX_FMT_GBRP:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV411P:
    	plan = 3;
    	break;
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
        plan = 2;
        break;
    }
    return plan;
}
int GetImagePlanLength(int plan, int width, int height, int colorIndex)
{
	switch(sPixelFormatTable[colorIndex].fmt) {
    case AV_PIX_FMT_YVYU422:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_UYVY422:
    	return width*2*height;
    case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
    	return width*4*height;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:

    case AV_PIX_FMT_YUV444P:
        return ((((width*3+3)>>2)<<2) * height);
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV411P:
    	if (plan == 0)
    		return width*height;
    	else
    		return width*height/4;

    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
    	if (plan == 0)
    		return width*height;
    	else
    		return width*height/2;
    case AV_PIX_FMT_GBRP: //RGB444
	case AV_PIX_FMT_GRAY8:
    	return width*height;
    }
	return 0;
}

int GetImageBufferLength(int width, int height, int colorIndex)
{
	int length = 0;
	switch(sPixelFormatTable[colorIndex].fmt) {
    case AV_PIX_FMT_YVYU422:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_UYVY422:
        length = width*2 * height;
        break;
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
        length = width * height * 3/2;
        break;
    case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
        length = width*4 * height;
        break;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_GBRP:
    case AV_PIX_FMT_YUV444P:
		length = (((width*3+3)>>2)<<2) * height;
        break;
    case AV_PIX_FMT_GRAY8:
    	length = width*height;
    	break;
    }
	printf("colorIndex = %d, length = %d\n", colorIndex, length);
    return length;
}

ImageFormat* CreateImageFile(const char* name, int width, int height, int colorIndex)
{
	int length = GetImageBufferLength(width, height, colorIndex);
	if (length <=0 ) {
		fprintf(stderr, "Not supported color format.\n");
		return NULL;
	}

    FILE* fp = fopen(name, "rb");
    if (!fp) {
      	fprintf(stderr, "Failed to open file %s\n", name);
      	return NULL;
    }
    void* buffer = malloc(length);
	fread(buffer, 1, length, fp);
	fclose(fp);

    return CreateImageBuffer(buffer, length, width, height, colorIndex);

}

ImageFormat* CreateImageBuffer(void* buf, int length, int width, int height, int colorIndex)
{
    ImageFormat* pImg = (ImageFormat*) malloc(sizeof(ImageFormat));
    memset(pImg, 0, sizeof(ImageFormat));
    pImg->colorCode = colorIndex;
    pImg->width = width;
    pImg->height = height;
    pImg->fps = 1;

    switch(sPixelFormatTable[colorIndex].fmt) {
    case AV_PIX_FMT_YVYU422:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_UYVY422:
        //422 Packet mode
        pImg->stride = width*2; //be multiple of 4
        pImg->bitsPerPixel = 16;
        pImg->length = pImg->stride * pImg->height;
        break;
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
        pImg->stride = width; //be multiple of 4
        pImg->bitsPerPixel = 12;
        pImg->length = pImg->stride * pImg->height * 3/2;
        break;
    case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
        pImg->stride = width*4; //be multiple of 4
        pImg->bitsPerPixel = 32;
        pImg->length = pImg->stride * pImg->height;
        break;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
        pImg->stride = ((width*3+3)>>2)<<2; //be multiple of 4
        pImg->bitsPerPixel = 24;
        pImg->length = pImg->stride * pImg->height;
        break;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_GBRP:
        pImg->stride = width; //be multiple of 4
        pImg->bitsPerPixel = 24;
        pImg->length = pImg->stride * pImg->height;
        break;
 	case AV_PIX_FMT_GRAY8:
        pImg->stride = width; //be multiple of 4
        pImg->bitsPerPixel = 8;
        pImg->length = pImg->stride * pImg->height;
        break;
    }
    if (pImg->length > length) {
    	fprintf(stderr, "No enough buffer length!\n");

    } else {
    	pImg->data = buf;
    }

    return pImg;

}

ImageFormat* CreateImage(int width, int height, int colorIndex, int defColor)
{
	int i, j;
	int* line = NULL;
    ImageFormat* pImg = (ImageFormat*) malloc(sizeof(ImageFormat));
    memset(pImg, 0, sizeof(ImageFormat));
    pImg->colorCode = colorIndex;
    pImg->width = width;
    pImg->height = height;
    pImg->fps = 1;
    switch(sPixelFormatTable[colorIndex].fmt) {

    case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
        pImg->stride = width*4; //be multiple of 4
        pImg->bitsPerPixel = 32;
        pImg->length = pImg->stride * pImg->height;
        pImg->data = malloc(pImg->length);
        line = (int*) pImg->data;
		for (i=0;i<height; i++) {
			for (j=0; j< width; j++)
				line[j] = defColor;
            line +=  pImg->stride/sizeof(int);

		}
        break;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
    default:
    	fprintf(stderr, "Not implemented!\n");
    	free(pImg);
    	return NULL;
    }

    return pImg;

}
bool DistroyImage(ImageFormat* pImage)
{
    if (pImage->data)
        free(pImage->data);
    free (pImage);
    return true;
}

AVPixelFormat GetAVPixelFormat(int color)
{
	if (color <0 || color >= (int)(sizeof(sPixelFormatTable)/sizeof(PixelFormatTable)))
		return AV_PIX_FMT_NONE;
	return sPixelFormatTable[color].fmt;
}
int GetIndexByAVPixelFormat(AVPixelFormat fmt)
{
	for (unsigned int i=0; i<sizeof(sPixelFormatTable)/sizeof(PixelFormatTable); i++ ) {
		if ( sPixelFormatTable[i].fmt  == fmt)
			return (int)i;
	}
	return -1;
}
int GetPixelFormat(char* key)
{
	for (unsigned int i=0; i<sizeof(sPixelFormatTable)/sizeof(PixelFormatTable); i++ ) {
		if (strncasecmp(sPixelFormatTable[i].key, key, sizeof(sPixelFormatTable[i].key)) == 0)
			return (int)i;
	}
	return -1;
}
void PrintPixelFormat(const char* indent)
{
	for (unsigned int i=0; i<sizeof(sPixelFormatTable)/sizeof(PixelFormatTable); i++ ) {
		printf("%s%s\t%s\n", indent, sPixelFormatTable[i].key, sPixelFormatTable[i].desc);
	}
}


