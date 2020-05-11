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
#include "ImageWin.h"

ImageWin* ImageWin::CreateImageWin(const char* name, int x, int y, int width, int height)
{
    SDL_Window* hWnd = SDL_CreateWindow(name, x, y, width, height, SDL_WINDOW_INPUT_GRABBED|SDL_WINDOW_RESIZABLE);
    if (hWnd == NULL){
	    return NULL;
    }
    SDL_Renderer* hRenderer = SDL_CreateRenderer(hWnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (hRenderer == NULL){
	    SDL_DestroyWindow(hWnd);
        return NULL;
    }
	ImageWin* w = new ImageWin();
    w->mhWnd = hWnd;
    w->mhRenderer = hRenderer;
    return w;
}
ImageWin::ImageWin():
    mhWnd(NULL), mhRenderer(NULL), mhTexture(NULL), mpSource(NULL), mpRgba(NULL) 
{
    mZoomFactor = 1.0;
    mpImage = NULL;
    mValidate = false;
}

ImageWin::~ImageWin()
{
    if (mhTexture)
        SDL_DestroyTexture(mhTexture);
    if(mhRenderer) 
        SDL_DestroyRenderer(mhRenderer);
    if(mhWnd) 
        SDL_DestroyWindow(mhWnd);
    if(mpSource) {
        if(mpRgba == mpSource)
            mpRgba = NULL;
        free(mpSource);
    }
    if(mpRgba){
        free(mpRgba);
    }
}
void ImageWin::freeImage()
{
    if (mpImage) {
        if (mpRgba == mpImage->data)
            mpRgba = NULL;
        DistroyImage(mpImage);
        mpImage = NULL;
    }
    if(mpRgba){
        free(mpRgba);
        mpRgba = NULL;
    }
}
bool ImageWin::setImage(ImageFormat* pImage)
{
    freeImage();
    mpImage = (ImageFormat*) malloc (sizeof(ImageFormat));
    mpImage = pImage;
 
    if (pImage->colorspace == AV_PIX_FMT_RGBA) {
        mpRgba = mpImage->data;
    } else {
 
        mpRgba = ConvertImageToRgb32(mpImage);
    }
    Uint32 rmask, gmask, bmask, amask;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
 
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(mpRgba, pImage->width, pImage->height,
            32, 4*pImage->width, rmask, gmask, bmask, amask);
    if (surf == NULL) {
        SDL_Log("Creating surface failed: %s", SDL_GetError()); 
        return false;    
    }
    if (mhTexture)
        SDL_DestroyTexture(mhTexture);
    mhTexture = SDL_CreateTextureFromSurface(mhRenderer, surf);
    SDL_FreeSurface(surf);

    /* reset display area */
    mRcDisplay.x = mRcDisplay.y = 0;
    mRcDisplay.w = int( (double)mpImage->width*mZoomFactor);
    mRcDisplay.h = int( (double)mpImage->height*mZoomFactor);
    mValidate = true;
    return (mhTexture != NULL);
}
bool ImageWin::open(const char* path)
{

    return (mhTexture != NULL);
}
void ImageWin::update()
{
    //SDL_RaiseWindow(mhWnd);

    if(mValidate) {
        draw();
        mValidate = false;
    }
}

void ImageWin::moveImage(int dx, int dy)
{
    mRcDisplay.x += dx;
    mRcDisplay.y += dy;
    mValidate = true;
}

void ImageWin::scaleImage(double scale)
{
    if (mpImage && scale > 0.01) {
        mZoomFactor = mZoomFactor * scale;
        mRcDisplay.w = mpImage->width * mZoomFactor;
        mRcDisplay.h = mpImage->height * mZoomFactor;;
        mValidate = true;
    }
}

#define min(a,b) ((a<b)?a:b)
void ImageWin::draw()
{
    SDL_RenderClear(mhRenderer);
	    //Draw the texture
    SDL_Rect SrcR;      //clip region in source
    SDL_Rect DestR;     //display region in window
    /* calculate display area */
    int W, H;
    SDL_GetWindowSize(mhWnd, &W, &H);

    /*************************************/
    bool bDrawImage = false;
    do {
        if (!mhTexture || !mpImage)
            break;
        if (mRcDisplay.x > W || mRcDisplay.y > H)
            break;
        if (mRcDisplay.x < 0) {
            if (mRcDisplay.x + mRcDisplay.w < 0)
                break;
            DestR.x = 0;
            DestR.w = (W < mRcDisplay.x + mRcDisplay.w)?W:(mRcDisplay.x + mRcDisplay.w);
            SrcR.x = - mRcDisplay.x/mZoomFactor;
        } else {
            DestR.x = mRcDisplay.x;
            DestR.w = ((mRcDisplay.x + mRcDisplay.w)>W)?(W-mRcDisplay.x):mRcDisplay.w;
            SrcR.x = 0;
        }
        SrcR.w = (int)((double)DestR.w/mZoomFactor);
        //check vertical
        if (mRcDisplay.y < 0) {
            if (mRcDisplay.y + mRcDisplay.h < 0)
                break;
            DestR.y = 0;
            DestR.h = (H < mRcDisplay.y + mRcDisplay.h)? H: (mRcDisplay.y + mRcDisplay.h);
            SrcR.y = - (int)((float)mRcDisplay.y/mZoomFactor);
        } else {
            DestR.y = mRcDisplay.y;
            DestR.h = ((mRcDisplay.y + mRcDisplay.h)>H)?(H-mRcDisplay.y):mRcDisplay.h;
            SrcR.y = 0;
        }
        SrcR.h = (int)((double)DestR.h/mZoomFactor);

        bDrawImage = true;
    } while(0);
 
    if (bDrawImage) {

	    SDL_RenderCopy(mhRenderer, mhTexture, &SrcR, &DestR);
	    //Update the screen

    } else {
        SDL_SetRenderDrawColor(mhRenderer, 100, 100, 100, 255);
        SDL_RenderFillRect(mhRenderer, NULL);
    }
    SDL_RenderPresent(mhRenderer);
}
