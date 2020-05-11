#ifndef IMAGEVWIN_H
#define IMAGEVWIN_H
#include <SDL2/SDL.h>
#include "common.h"

class ImageWin
{
public:
    static ImageWin* CreateImageWin(const char* name, int x, int y, int width, int height);
	ImageWin();
	~ImageWin();
    SDL_Window* getWindow() {return mhWnd;}
    bool open(const char* path);
    bool setImage(ImageFormat* pImage);
    void moveImage(int dx, int dy);
    void scaleImage(double scale);
	void draw();
    void update();
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
