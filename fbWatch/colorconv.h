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
#ifndef _COLOR_CONV_H_
#define _COLOR_CONV_H_

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
void Gray8_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Yuv444p_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);
void Rgb444_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb);

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

void Nv420pToRgb32(unsigned char* pY1, unsigned char* pU, unsigned char* pV, int width, int stride, int height, unsigned char* pRgb)
{
    unsigned char* pY2 = pY1+stride;
    int nBps = width*4;
    unsigned char* pLine1 = pRgb;
    unsigned char* pLine2 = pRgb+nBps;

    unsigned char y1,y2,u,v;
    for (int i=0; i<height; i+=2)
    {
        for (int j=0; j<width; j++) {
            y1 = pY1[j];
            u = pU[j];
            v = pV[j];
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
        pV += stride;
        pU += stride;
        pLine1 = pLine2 + nBps;
        pLine2 = pLine1 + nBps;
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

void Yuv420p_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	Yuv420pToRgb32(pYuv, width, stride, height, pRgb, true);
}
void Nv12_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	unsigned char* pY1 = pYuv;
	unsigned char* pU = pYuv + stride*height;
	unsigned char* pV = pU + 1;
	Nv420pToRgb32(pY1, pU, pV, width, stride, height, pRgb);
}
void Nv21_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	unsigned char* pY1 = pYuv;
	unsigned char* pV = pYuv + stride*height;
	unsigned char* pU = pV + 1;
	Nv420pToRgb32(pY1, pU, pV, width, stride, height, pRgb);
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
	unsigned char* ps = pYuv;
	unsigned char* pt = pRgb;
	for (int i=0; i<height; i++) {
		for (int j=0; j<width; j++) {
			pt[j*4] = ps[j*4+2];	//B
			pt[j*4+1] = ps[j*4+1];	//G
			pt[j*4+2] = ps[j*4];	//R
			pt[j*4+3] = 0xff;		//A
		}
		pt += width*4;
		ps += stride;
	}
}
void Bgra_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
    unsigned char* ps = pYuv;
    unsigned char* pt = pRgb;
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            pt[j*4] = ps[j*4];
            pt[j*4+1] = ps[j*4+1];
            pt[j*4+2] = ps[j*4+2];
            pt[j*4+3] = 0xff;
        }
        pt += width*4;
        ps += stride;
    }

}
void Rgb24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{

	Rgb24ToRgb32(pYuv, width, stride, height, pRgb, 0);

}
void Bgr24_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	Rgb24ToRgb32(pYuv, width, stride, height, pRgb, 1);
}

void Gray8_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	unsigned char* ps = pYuv;
	unsigned char* pt = pRgb;
	for (int i=0; i<height; i++) {
		for (int j=0; j<width; j++) {
			pt[j*4] = ps[j];	//B
			pt[j*4+1] = ps[j];	//G
			pt[j*4+2] = ps[j];	//R
			pt[j*4+3] = 0xff;	//A
		}
		pt += width*4;
		ps += stride;
	}
}
void Yuv444p_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
    int nBps = width*4;
    unsigned char* pY1 = pYuv;

    unsigned char* pV;
    unsigned char* pU;
 
    pU = pY1+stride*height; 
    pV = pU+stride*height;

    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;

    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j++)
        {
            y1 = pY1[j];
            u = pU[j];
            v = pV[j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;
        }
        pY1 += stride;
        pV += stride;
        pU += stride;
        pLine1 += nBps;
    }
}
void Rgb444_Rgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb)
{
	unsigned char* pr = pYuv;
	unsigned char* pg = pYuv+stride*height;
	unsigned char* pb = pg + stride*height;
	
	unsigned char* pt = pRgb;
	for (int i=0; i<height; i++) {
		for (int j=0; j<width; j++) {
			pt[j*4] = pb[j];	//B
			pt[j*4+1] = pg[j];	//G
			pt[j*4+2] = pr[j];	//R
			pt[j*4+3] = 0xff;	//A
		}
		pt += width*4;
		pr += stride;
		pg += stride;
		pb += stride;
	}	
}


#endif //_COLOR_CONV_H_
