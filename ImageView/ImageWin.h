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
#ifndef IMAGEVWIN_H
#define IMAGEVWIN_H
#include <SDL2/SDL.h>
#include "common.h"

class ImageWin
{
public:
    static ImageWin* CreateImageWin(const char* name);   
    static ImageWin* CreateWinByFile(const char* name, int width, int height, int color);
	ImageWin();
	~ImageWin();
    SDL_Window* getWindow() {return mhWnd;}
    bool open(const char* path);
    bool setImage(ImageFormat* pImage);
    void moveImage(int dx, int dy);
    void scaleImage(double dScale);
    double getScaleFactor() { return mZoomFactor;}
	void draw();
    void update(bool bForce=false);
protected:
    void freeImage();
private:
    SDL_Rect mRcDisplay;
protected:
	SDL_Window* mhWnd;
	SDL_Renderer* mhRenderer;
	SDL_Texture* mhTexture;
    /* image source property */
    ImageFormat* mpImage;
    void* mpSource;
    void* mpRgba;
    /* display parameters */
    bool mValidate;
    double mZoomFactor;
};
#endif
