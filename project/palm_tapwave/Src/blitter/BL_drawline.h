/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWLINE_H_
#define __BL_DRAWLINE_H_

/**************************************************************
* DrawLine Helper Functions
**************************************************************/

// ClipLine() constants
#define CL_LEFT		1
#define CL_RIGHT		2
#define CL_TOP		4
#define CL_BOTTOM	8

namespace DrawLineHelper {

	inline
	DWORD GetOutCode(POINT &pt, RECT &clipper)
	{
		DWORD code;

		code = 0;

		if (pt.y < clipper.top)
			code |= CL_TOP;	// TOP
		else if (pt.y > clipper.bottom-1)
			code |= CL_BOTTOM; // BOTTOM

		if (pt.x < clipper.left)
			code |= CL_LEFT;	// LEFT
		else if (pt.x > clipper.right-1)
			code |= CL_RIGHT;	// RIGHT

		return code;
	}

	inline
	BOOL ClipLine(POINT &pt1, POINT &pt2, RECT &clipper)
	{
		// Clip using Cohen-Sutherland
		LONG xmin = clipper.left;
		LONG ymin = clipper.top;
		LONG xmax = clipper.right - 1;
		LONG ymax = clipper.bottom - 1;

		DWORD code1 = GetOutCode( pt1, clipper );
		DWORD code2 = GetOutCode( pt2, clipper );

		// This code first came straight from PocketFrog,
		// but has been substantially tweaked to fix bugs. -ptm
		while (code1 || code2)
		{
			// Trivial rejection
			if (code1 & code2) return FALSE;

			DWORD code = (code1) ? code1 : code2;
			LONG x, y;

			if (code & CL_LEFT)
			{
				// Left edge
				if (pt2.x == pt1.x)
					y = pt1.y;
				else
					y = pt1.y + (pt2.y - pt1.y) * (xmin - pt1.x) / (pt2.x - pt1.x);
				
				x = xmin;
			}
			else if (code & CL_RIGHT)
			{
				// Right edge
				if (pt2.x == pt1.x)
					y = pt1.y;
				else
					y = pt1.y + (pt2.y - pt1.y) * (xmax - pt1.x) / (pt2.x - pt1.x);
				
				x = xmax;
			}
			else if (code & CL_TOP)
			{
				// Top edge
				if (pt2.y == pt1.y)
					x = pt1.x;
				else
					x = pt1.x + (pt2.x - pt1.x) * (ymin - pt1.y) / (pt2.y - pt1.y);
				
				y = ymin;
			}
			else if (code & CL_BOTTOM)
			{
				// Bottom edge
				if (pt2.y == pt1.y)
					x = pt1.x;
				else
					x = pt1.x + (pt2.x - pt1.x) * (ymax - pt1.y) / (pt2.y - pt1.y);
				
				y = ymax;
			}

			if (code == code1)
			{
				pt1.x = x;
				pt1.y = y;
				code1 = GetOutCode(pt1, clipper);
			}
			else
			{
				pt2.x = x;
				pt2.y = y;
				code2 = GetOutCode(pt2, clipper);
			}
		}
		
		return TRUE;
	}

}

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawLine Functions
**************************************************************/

// -------------------- VERT. & HORIZ. LINES -----------------------

#define BL_DrawVLine555\
	#error "Not yet supported"
	
#define BL_DrawHLine555\
	#error "Not yet supported"
	
#define BL_DrawVLineOpacity555\
	#error "Not yet supported"
	
#define BL_DrawHLineOpacity555\
	#error "Not yet supported"

// -------------------- STANDARD LINES -----------------------

#define BL_DrawLine555\
	#error "Not yet supported"
	
#define BL_DrawLineOpacity555\
	#error "Not yet supported"
	
#define BL_DrawWuLine555\
	#error "Not yet supported"
	
#define BL_DrawWuLineOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 DrawLine Functions
**************************************************************/

// -------------------- VERT. & HORIZ. LINES -----------------------

void BL_DrawVLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x, LONG y1, LONG y2,
	COLORREF dwColor);

void BL_DrawHLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1,
	LONG y, LONG x2, COLORREF dwColor);

void BL_DrawVLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x, LONG y1, LONG y2,
	COLORREF dwColor, DWORD dwOpacity);

void BL_DrawHLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1,
	LONG y, LONG x2, COLORREF dwColor, DWORD dwOpacity);
	
// -------------------- STANDARD LINES -----------------------

void BL_DrawLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x1, LONG y1, LONG x2, LONG y2,
	COLORREF dwColor);
	
void BL_DrawLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x1, LONG y1, LONG x2, LONG y2,
	COLORREF dwColor, DWORD dwOpacity);

void BL_DrawWuLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG X0, LONG Y0, LONG X1, LONG Y1,
	COLORREF dwColor);

void BL_DrawWuLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG X0, LONG Y0, LONG X1, LONG Y1,
	COLORREF dwColor, DWORD dwMasterOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWLINE_H_