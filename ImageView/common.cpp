#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "common.h"

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
void Rgb24ToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, AVPixelFormat fmt)
{
    //YVYU - format
    int nBps = width*4;
    unsigned char* pY1;
    unsigned char* pY2;
    unsigned char* pY3;

    switch (fmt) {
        case AV_PIX_FMT_BGR24:
        pY3 = pYuv;
        pY2 = pY3+1; 
        pY1 = pY2+1; 
        break;

        case AV_PIX_FMT_RGB24:
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


void Yuy420pToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
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
/* convert string to ImageFormat code */
AVPixelFormat GetFormateByName(const char* name)
{
    if (strcasecmp (name, "yuyv") == 0)
        return AV_PIX_FMT_YUYV422;
    if (strcasecmp (name, "yvyu") == 0)
        return AV_PIX_FMT_YVYU422;
    if (strcasecmp (name, "uyvy") == 0)
        return AV_PIX_FMT_UYVY422;
    if (strcasecmp (name, "i422") == 0)
        return AV_PIX_FMT_YUV422P;
    if (strcasecmp (name, "i420") == 0)
        return AV_PIX_FMT_YUV420P;
    if (strcasecmp (name, "rgba") == 0)
        return AV_PIX_FMT_RGBA;
    if (strcasecmp (name, "rgb") == 0)
        return AV_PIX_FMT_BGR24;
    if (strcasecmp (name, "rgb") == 0)
        return AV_PIX_FMT_BGR24;
    return AV_PIX_FMT_NONE;
}
void* ConvertImageToRgb32(ImageFormat* pImage)
{
    unsigned char* pRgb = NULL;
    if (pImage->colorspace == AV_PIX_FMT_RGBA)
        return pImage->data;

    pRgb = (unsigned char*) malloc(pImage->width* 4* pImage->height);
    if (!pRgb)
        return NULL;
    switch(pImage->colorspace)
    {
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
            Rgb24ToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
                    pImage->height, pRgb, pImage->colorspace);
            break;
         case AV_PIX_FMT_YUV420P:
            Yuy420pToRgb32((unsigned char*)pImage->data, pImage->width, pImage->stride,
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
                        pImg->colorspace = AV_PIX_FMT_YUV444P;
                        pImg->bitsPerPixel = 24;
                        pImg->stride = ((pImg->width+3)>>2)<<2;
                        pImg->length = pImg->stride * pImg->height * 3;
                   } else if (strcmp(p2, "422") == 0) {
                        pImg->colorspace = AV_PIX_FMT_YUV422P;
                        pImg->bitsPerPixel = 16;
                        pImg->stride = ((pImg->width+3)>>2)<<2;
                        pImg->length = pImg->stride * pImg->height * 2;
                   } else  { //420
                        pImg->colorspace = AV_PIX_FMT_YUV420P;
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
        AVPixelFormat fmt = AV_PIX_FMT_NONE;

        if (strcmp(ext, ".yuv") == 0)
            fmt = AV_PIX_FMT_YUYV422;
        else if(strcmp(ext, ".rgba") == 0){
           fmt = AV_PIX_FMT_RGBA;  
        }
        else if (strcmp(ext, ".420p") == 0)
            fmt = AV_PIX_FMT_YUV420P;

        else if(strcmp(ext, ".rgb") == 0)
            fmt = AV_PIX_FMT_RGB24;
        else if(strcmp(ext, ".bgr") == 0)
            fmt = AV_PIX_FMT_BGR24;
        if (fmt != AV_PIX_FMT_NONE)
            return CreateImageFile(filename, width, height, fmt);
    }
    return NULL;
}
ImageFormat* CreateImageFile(const char* name, int width, int height, AVPixelFormat fmt)
{
    ImageFormat* pImg = (ImageFormat*) malloc(sizeof(ImageFormat));
    memset(pImg, 0, sizeof(ImageFormat));
    pImg->colorspace = fmt;
    pImg->width = width;
    pImg->height = height;
    pImg->fps = 1;
    switch(fmt) {
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

