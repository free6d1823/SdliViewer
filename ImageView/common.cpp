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


void Yuv420p_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Nv12_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Nv21_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Yuyv422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Yvyu422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Uyvy422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Rgba_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Bgra_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Rgb24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Bgr24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);

PixelFormatTable sPixelFormatTable[] = {
	{AV_PIX_FMT_YUV420P, "0", "YUV420 plannar, I420", Yuv420p_Rgb32},
	{AV_PIX_FMT_NV12, "1", "YUV420 semi-plannar UV NV12", Nv12_Rgb32},
	{AV_PIX_FMT_NV21, "2", "YUV420 semi-plannar VU NV21", Nv21_Rgb32},
	{AV_PIX_FMT_YUYV422, "3", "YUYV422 YUYV packet", Yuyv422_Rgb32},
	{AV_PIX_FMT_UYVY422, "4", "UYVY420 UYVY packet", Uyvy422_Rgb32},
	{AV_PIX_FMT_RGBA, "11", "packed RGBA 8:8:8:8, 32bpp", Rgba_Rgb32},
	{AV_PIX_FMT_BGRA, "12", "packed BGRA 8:8:8:8, 32bpp", Bgra_Rgb32},
	{AV_PIX_FMT_RGB24, "21", "RGB24", Rgb24_Rgb32},
	{AV_PIX_FMT_BGR24, "22", "BGR24", Bgr24_Rgb32},
	{AV_PIX_FMT_YVYU422, "23", "YVYU422 YVYU packet", Yvyu422_Rgb32}

};


#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)


// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)
//////
/// \brief Yuy422 packet mode to RGBA 32
/// \param pYuv
/// \param width
/// \param stride
/// \param height
/// \param pRgb     output RGB32 buffer
/// \param fmt AV_PIX_FMT_UYVY422,    AV_PIX_FMT_YVYU,  AV_PIX_FMT_YUYV
///
static void YuyvToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, AVPixelFormat fmt)
{
    //YVYU - format
    int nBps = width*4;
    unsigned char* pY1;

    unsigned char* pV;
    unsigned char* pU;

    switch (fmt) {
        case AV_PIX_FMT_UYVY422:
        pU = pYuv;
        pY1 = pU +1;
        pV =  pU+2;
        break;

        case AV_PIX_FMT_YVYU422:
        pY1 = pYuv;
        pV = pY1+1; pU = pV+2;
        break;

        case AV_PIX_FMT_YUYV422:
        default:
        pY1 = pYuv;
        pU = pY1+1; 
        pV = pU+2;            

        break;
    }


    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;
    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j+=2)
        {
            y1 = pY1[2*j];
            u = pU[2*j];
            v = pV[2*j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y1 = pY1[2*j+2];
            pLine1[j*4+4] = YUV2B(y1, u, v);//b
            pLine1[j*4+5] = YUV2G(y1, u, v);//g
            pLine1[j*4+6] = YUV2R(y1, u, v);//r
            pLine1[j*4+7] = 0xff;
        }
        pY1 += stride;
        pV += stride;
        pU += stride;
        pLine1 += nBps;

    }
}
void Rgb24ToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, int fmt)
{
    //YVYU - format
    int nBps = width*4;
    unsigned char* pY1;
    unsigned char* pY2;
    unsigned char* pY3;

    switch (fmt) {
        case 1: //BGR
        pY3 = pYuv;
        pY2 = pY3+1; 
        pY1 = pY2+1; 
        break;

        case 0: //RGB
        default:
        pY1 = pYuv;
        pY2 = pY1+1; 
        pY3 = pY2+1;            

        break;
    }

    unsigned char* pLine1 = pRgb;
    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j++)
        {
            pLine1[j*4] = pY1[3*j]; 
            pLine1[j*4+1] = pY2[3*j]; 
            pLine1[j*4+2] = pY3[3*j]; 
            pLine1[j*4+3] = 0xff;

        }
        pY1 += stride;
        pY2 += stride;
        pY3 += stride; 
        pLine1 += nBps;

    }
}


void Yuv420pToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
{
    //YVU420 - format 4Y:1V:1U
    int nBps = width*4;
    unsigned char* pY1 = pYuv;
    unsigned char* pY2 = pYuv+stride;

    unsigned char* pV;
    unsigned char* pU;
 
    if (uFirst) {
        pU = pY1+stride*height; pV = pU+width*height/4;
    } else {
        pV = pY1+stride*height; pU = pV+stride*height/4;
    }


    unsigned char* pLine1 = pRgb;
    unsigned char* pLine2 = pRgb+nBps;

    unsigned char y1,y2,u,v;
    for (int i=0; i<height; i+=2)
    {
        for (int j=0; j<width; j++)
        {
            y1 = pY1[j];
            u = pU[j/2];
            v = pV[j/2];
            pLine1[j*4] = YUV2R(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2B(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y2 = pY2[j];

            pLine2[j*4] = YUV2R(y2, u, v);//b
            pLine2[j*4+1] = YUV2G(y2, u, v);//g
            pLine2[j*4+2] = YUV2B(y2, u, v);//r
            pLine2[j*4+3] = 0xff;

            j++;

            y1 = pY1[j];
            pLine1[j*4] = YUV2R(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2B(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y2 = pY2[j];

            pLine2[j*4] = YUV2R(y2, u, v);//b
            pLine2[j*4+1] = YUV2G(y2, u, v);//g
            pLine2[j*4+2] = YUV2B(y2, u, v);//r
            pLine2[j*4+3] = 0xff;

        }
        pY1 = pY2 + stride;
        pY2 = pY1 + stride;
        pV += stride/2;
        pU += stride/2;
        pLine1 = pLine2 + nBps;
        pLine2 = pLine1 + nBps;

    }
}

void Yuy422pToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
{
    //YVU420 - format 4Y:1V:1U
    int nBps = width*4;
    unsigned char* pY1 = pYuv;

    unsigned char* pV;
    unsigned char* pU;
 
    if (uFirst) {
        pU = pY1+stride*height; pV = pU+width*height/2;
    } else {
        pV = pY1+stride*height; pU = pV+stride*height/2;
    }


    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;
    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j++)
        {
            y1 = pY1[j];
            u = pU[j/2];
            v = pV[j/2];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;


            j++;

            y1 = pY1[j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;


        }
        pY1 += stride;
        pV += stride/2;
        pU += stride/2;
        pLine1 += nBps;
    }
}

void* ConvertImageToRgb32(ImageFormat* pImage)
{
    unsigned char* pRgb = NULL;
    if (GetAVPixelFormat(pImage->colorspace) == AV_PIX_FMT_RGBA)
        return pImage->data;

    pRgb = (unsigned char*) malloc(pImage->width* 4* pImage->height);
    if (!pRgb)
        return NULL;
	if(sPixelFormatTable[pImage->colorCode].fnConv)
		sPixelFormatTable[pImage->colorCode].fnConv((unsigned char*)pImage->data, 
				pImage->width, pImage->stride, pImage->height, pRgb);
/*
    switch(pImage->colorspace)
    {
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
            Rgb24ToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
                    pImage->height, pRgb, pImage->colorspace);
            break;
         case AV_PIX_FMT_YUV420P:
            Yuv420pToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
                    pImage->height, pRgb, true);
            break;
         case AV_PIX_FMT_YUV422P:
            Yuy422pToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
                    pImage->height, pRgb, true);
            break;
        case AV_PIX_FMT_YVYU422: //Y0 Cr Y1 Cb
        case AV_PIX_FMT_YUYV422:
        case AV_PIX_FMT_UYVY422: //Cb Y0 Cr Y1 *
            YuyvToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
                    pImage->height, pRgb, pImage->colorspace);
            break;
        default:
        break;
    }
*/
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
printf("filenae=%s, ext=%s,\n", filename, ext);
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
ImageFormat* CreateImageFile(const char* name, int width, int height, int colorIndex)
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
        pImg->stride = width; //be multiple of 4
        pImg->bitsPerPixel = 12;
        pImg->length = pImg->stride * pImg->height * 3/2;
        break;
    case AV_PIX_FMT_RGBA:
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
    }
    
    pImg->data = malloc(pImg->length);
    FILE* fp = fopen(name, "rb");
    fread(pImg->data, 1, pImg->length, fp);
    fclose(fp);
    return pImg;

}
bool DistroyImage(ImageFormat* pImage)
{
    if (pImage->data)
        free(pImage->data);
    free (pImage);
    return true;
}
/* pixel format */

void Yuv420p_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	Yuv420pToRgb32(pYuv, width, stride, height, pRgb, true);
}
void Nv12_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
}
void Nv21_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
}
void Yuyv422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	YuyvToRgb32(pYuv, width, stride, height, pRgb, AV_PIX_FMT_YUYV422);

}
void Yvyu422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	YuyvToRgb32(pYuv, width, stride, height, pRgb, AV_PIX_FMT_YVYU422);
}
void Uyvy422_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	YuyvToRgb32(pYuv, width, stride, height, pRgb, AV_PIX_FMT_UYVY422);
}
void Rgba_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	memcpy(pRgb, pYuv, stride* width);
}
void Bgra_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
}
void Rgb24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	Rgb24ToRgb32(pYuv, width, stride, height, pRgb, 0);
}
void Bgr24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	Rgb24ToRgb32(pYuv, width, stride, height, pRgb, 1);
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
		if (memcmp(sPixelFormatTable[i].key, key, sizeof(sPixelFormatTable[i].key)) == 0)
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


