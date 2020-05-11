#ifndef COMMON_H
#define COMMON_H

#include <pixfmt.h>
typedef struct _ImageFormat {
    AVPixelFormat colorspace; //AV_PIX_FMT_RGB24, AV_PIX_FMT_RGBA, AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P
    int width;
    int height;
    float fps;
    int interlace;//0 progressive, 1 top field first, 2 bottom field first
    int stride;
    int bitsPerPixel;
    void* data;
    long long length;
    ////
}ImageFormat;

ImageFormat* CreateImageFileByName(const char* filename);
ImageFormat* CreateImageFile(const char* name, int width, int height, AVPixelFormat fmt);
bool DistroyImage(ImageFormat* pImage);
void* ConvertImageToRgb32(ImageFormat* pImage);
AVPixelFormat GetFormateByName(const char* name);
#endif //COMMON_H
