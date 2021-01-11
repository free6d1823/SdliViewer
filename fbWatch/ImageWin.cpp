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

ImageWin* ImageWin::CreateImageWin(const char* name)
{
    int flags = SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE;
    SDL_Window* hWnd = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, flags);
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
ImageWin* ImageWin::CreateWinByFile(const char* name, int width, int height, int color)
{
    ImageFormat* pImg = NULL;
	AVPixelFormat fmt = GetAVPixelFormat(color);

    if (AV_PIX_FMT_NONE == fmt) {
        pImg = CreateImageFileByName(name);
    } else {
        pImg = CreateImageFile(name, width, height, color);
    }

    if (!pImg) {
        fprintf(stderr, "Failed to load file %s!\n", name);
        return NULL;
    }
    ImageWin* pWin = CreateImageWin(name);
    if (pWin)
        pWin->setImage(pImg);
    else
        DistroyImage(pImg);
    return pWin;
}
ImageWin* ImageWin::Create(int width, int height, int colorIndex, int defColor)
{
	char name [256];
	sprintf(name, "%dx%d", width, height);
    int flags = SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE;
    SDL_Window* hWnd = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
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

	ImageFormat* pImage = CreateImage(width, height, colorIndex, defColor);
	if (pImage)
			w->setImage(pImage);
	return w;

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
/*copy pImage to ImageWin->mpImage, free pImage by caller */
bool ImageWin::putImage(int x, int y, ImageFormat* pImage)
{
	if (!mpImage || !mpRgba)
		return false;
	unsigned char* pSource = (unsigned char*) ConvertImageToRgb32(pImage);
	if (!pSource)
		return false;
	/* TODO: force start point . 0 */
	if (x <0) x=0;
	if (y <0) y=0;

	unsigned char* pDest = (unsigned char*)mpRgba + x*4 + y*mpImage->stride;
	int ye = y + pImage->height;
	if (ye > mpImage->height) ye = mpImage->height;
	int xe = x + pImage->width;
	if (xe > mpImage->width) xe = mpImage->width;
	unsigned char* pt = pDest;
	unsigned char* ps = pSource;

	xe = (xe-x)*4;
	int desStride = mpImage->width*4;
	int srcStride = pImage->width*4;

	for (int i= y; i<ye; i++ ) {
		memcpy(pt, ps, xe);
		pt += desStride;
		ps += srcStride;
	}
	free(pSource);
	//update surface
	Uint32 rmask, gmask, bmask, amask;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(mpRgba, mpImage->width, mpImage->height,
            32, 4*mpImage->width, rmask, gmask, bmask, amask);
    if (surf == NULL) {
        SDL_Log("Creating surface failed: %s", SDL_GetError());
        return false;
    }
    if (mhTexture)
        SDL_DestroyTexture(mhTexture);
    mhTexture = SDL_CreateTextureFromSurface(mhRenderer, surf);
    SDL_FreeSurface(surf);

    mValidate = true;

    return (mhTexture != NULL);
}

bool ImageWin::setImage(ImageFormat* pImage)
{
    freeImage();
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

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(mpRgba, mpImage->width, mpImage->height,
            32, 4*mpImage->width, rmask, gmask, bmask, amask);
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

    SDL_SetWindowSize(mhWnd,  mRcDisplay.w, mRcDisplay.h);
    SDL_ShowWindow(mhWnd);

    return (mhTexture != NULL);
}
bool ImageWin::open(const char* path)
{

    return (mhTexture != NULL);
}
void ImageWin::update(bool bForce)
{
    //SDL_RaiseWindow(mhWnd);

    if(bForce || mValidate) {
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

void ImageWin::scaleImage(double dScale)
{
    if (mpImage && dScale > 0.01) {
        mZoomFactor = mZoomFactor * dScale;
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
