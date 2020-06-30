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

class ImageWin;
extern ImageWin* gWin[2];/* global windows */

#endif //COMMON_H
