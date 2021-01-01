
#include <windows.h>

#define _USE_MATH_DEFINES 1
#include <math.h>

#include "SiemensClock.h"


//Image **CSiemensClock::m__backgroundImages = NULL;

map<int, Image*> CSiemensClock::m_backgroundImages;


CSiemensClock::CSiemensClock (const CString& installDir)
{
    m_width            = 0;
    m_height           = 0;
    m_clockX           = 0;
    m_clockY           = 0;
    m_clockWidth       = 0;
    m_clockHeight      = 0;
    m_cachedBackground = NULL;
    m_memBitmap        = NULL;
    m_memGraphics      = NULL;
    m_backgroundImage  = NULL;
    m_hTheme           = NULL;

	// Did we already read the background images?
    if (m_backgroundImages.empty ()) 
    {
        //static WCHAR    path[2000];
        WIN32_FIND_DATA FindFileData;
        HANDLE          hFind;
		CString         fileName;

		// Read all background image files ("SiemensClock-*.png"):
        hFind = FindFirstFile (installDir + CString ("SiemensClock-*.png"),
                               &FindFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                fileName = CString (installDir) + FindFileData.cFileName;
                //TRACE (_T ("Found image \"%s\"."), fileName);
                //MultiByteToWideChar (CP_UTF8, 0, LPCWCH(fileName), fileName.GetLength() + 1,
                //                     path, sizeof (path));
                Image *backgroundImage = new Image (LPCWCH(fileName));
				// Put each image into a map that uses the number of pixels as a key:
                m_backgroundImages[backgroundImage->GetWidth () *
                                   backgroundImage->GetHeight ()] =
                    backgroundImage;
            } while (FindNextFile (hFind, &FindFileData) != 0);
        } // End if (first file found)
    } // End if (Background image files not yet read).
} // End constructor.

CSiemensClock::~CSiemensClock (void)
{
    if (m_cachedBackground) {
        delete m_cachedBackground;
        m_cachedBackground = NULL;
    }
    if (m_memGraphics) {
        delete m_memGraphics;
        m_memGraphics = NULL;
    }
    if (m_memBitmap) {
        delete m_memBitmap;
        m_memBitmap = NULL;
    }
}

void CSiemensClock::SetTime (SYSTEMTIME *time) {
    memcpy (&m_currentTime, time, sizeof (SYSTEMTIME));
}

void CSiemensClock::DrawBackground (HWND wnd)
{
    RECT rc;
    GetClientRect(wnd, &rc);

    int TBPart = 0;

    unsigned int width  = rc.right  - rc.left + 1;
    unsigned int height = rc.bottom - rc.top  + 1;

    // Check for changed window size:
    if (width != m_width || height != m_height) {

        m_width  = width;
        m_height = height;
        m_center = Point ((rc.left + rc.right) / 2,
                          (rc.top + rc.bottom) / 2);

        // Look for the largest image that fits into the window:
		// Search the first image that is certainly too large,
		// i.e. that has more pixels than the window client area.
        map<int, Image*>::iterator iter =
            m_backgroundImages.upper_bound ((m_width - 2) * (m_height - 2));
		// Start with the next smaller image:
        if (iter == m_backgroundImages.end () && 
            iter != m_backgroundImages.begin ()) {
            iter--;
        }
		// Search the largest image that fits:
        while (iter != m_backgroundImages.begin () && 
               (iter->second->GetWidth ()  > m_width - 2 ||
                iter->second->GetHeight () > m_height - 2)) {
            iter--;
        }
        m_backgroundImage = iter->second;

        /* Determine clock width, height, and position, from the
         * size of the background image and the space around it.
         * (Default values for the position are to center the clock.)
         */
        m_clockWidth  = m_backgroundImage->GetWidth ();
        m_clockHeight = m_backgroundImage->GetHeight ();
        m_clockX      = (m_width  - m_clockWidth)  / 2;
        m_clockY      = (m_height - m_clockHeight) / 2;

        //TRACE (_T ("Image size: %d x %d."), m_clockWidth, m_clockHeight);

        if (m_memGraphics) {
            delete m_memGraphics;
            m_memGraphics = NULL;
        }

        /* Find the taskbar position: 
         */
        HWND taskbarWnd = FindWindow (LPCWSTR("Shell_TrayWnd"), NULL);

        if (wnd != NULL) {
            APPBARDATA abd;

            abd.cbSize = sizeof (APPBARDATA);
            abd.hWnd = taskbarWnd;

            SHAppBarMessage (ABM_GETTASKBARPOS, &abd);

            if (abd.rc.top == abd.rc.left && abd.rc.bottom > abd.rc.right) {
                TBPart = TBP_BACKGROUNDLEFT;
            }
            else if (abd.rc.top == abd.rc.left && abd.rc.bottom < abd.rc.right) { 
                TBPart = TBP_BACKGROUNDTOP;
            }
            else if (abd.rc.top > abd.rc.left)  {
                TBPart = TBP_BACKGROUNDBOTTOM;
            }
            else {
                TBPart = TBP_BACKGROUNDRIGHT;
            }
        } 

        /* Align the clock to the window title (top or left):
         */
        if (TBPart == TBP_BACKGROUNDRIGHT || TBPart == TBP_BACKGROUNDLEFT) {
            m_clockY = 0;
            m_center.Y = m_clockHeight / 2;
        }
        else if (TBPart == TBP_BACKGROUNDBOTTOM || TBPart == TBP_BACKGROUNDTOP) {
            m_clockX = 0;
            m_center.X = m_clockWidth / 2;
        }
    } // End if (Window size changed).


    if (!m_memGraphics) {
        if (m_memBitmap) delete m_memBitmap;
        m_memBitmap = new Bitmap (m_width, m_height);
        m_memGraphics = Graphics::FromImage (m_memBitmap);
        m_memGraphics->SetSmoothingMode (SmoothingModeHighQuality);
        if (m_hTheme) {
            CloseThemeData (m_hTheme);
            m_hTheme = NULL;
        }
        if (m_cachedBackground) {
            delete m_cachedBackground;
            m_cachedBackground = NULL;
        }
    }

    if (!m_hTheme) {
        // Get the theme handle if it exists for the task bar:
        m_hTheme = OpenThemeData(wnd, L"TaskBar");
    }

    if (!m_cachedBackground) {
        /* Draw the window background for the first time
         * and then cache it.
         */
        if (m_hTheme) {  // A theme is active.
            // Draw the themed task bar background:
            HDC      dc = m_memGraphics->GetHDC ();
            HRESULT  hr = DrawThemeBackground (m_hTheme, dc, 
                                               //TBP_BACKGROUNDRIGHT, 0,
                                               TBPart, 0,
                                               &rc, NULL);
            m_memGraphics->ReleaseHDC (dc);
        }
        else {  // No theme.
            // Use plain 3D background color:
            DWORD threedface = GetSysColor (COLOR_3DFACE);
            m_memGraphics->Clear (Color (GetRValue (threedface), 
                                         GetGValue (threedface), 
                                         GetBValue (threedface)));
            //m_memGraphics->Clear (Color (0, 0, 0, 0));
        }
        // Draw the clock background:
        m_memGraphics->DrawImage (m_backgroundImage, m_clockX, m_clockY);

        // Cache the window background:
        m_cachedBackground = new CachedBitmap (m_memBitmap, m_memGraphics);
    }
    else {
        /* Draw the cached window background:
         */
        m_memGraphics->DrawCachedBitmap (m_cachedBackground, 0, 0);
    }

} // End of DrawBackground ().


void CSiemensClock::Draw (HWND wnd)
{
    if (!this) return;

    HDC dc = GetDC (wnd);

    Graphics graphics (dc);
    graphics.SetSmoothingMode (SmoothingModeHighQuality);

    /* Draw the background image into the offscreen memory graphics:
     * ((Re-)initialises the offscreen memory graphics as needed.)
     */
    DrawBackground (wnd);

    Color        black (255, 0, 0, 0);
    Color        red (255, 220, 0, 0);

    Pen          hoursPen (black);
    SolidBrush   hoursBrush (black);
    Pen          minutesPen (black);
    SolidBrush   minutesBrush (black);
    Pen          secondsPen (red);
    SolidBrush   secondsBrush (red);
    GraphicsPath hoursHand, minutesHand, secondsHand, *leftHalf;
    float        clockRadius = min (m_clockWidth, m_clockHeight) / 2.0f;
    float        scale = clockRadius * 0.65f;
    Matrix       mirror; mirror.Scale (-1.0f, 1.0f);

    /* Draw the hours hand:
     */
    hoursHand.AddBezier (0.000f,  0.230f, 0.030f,  0.230f,  0.030f,  0.230f,  0.060f,  0.220f);
    hoursHand.AddBezier (0.060f,  0.220f, 0.050f,  0.190f,  0.040f,  0.090f,  0.040f,  0.050f);
    hoursHand.AddBezier (0.040f,  0.050f, 0.080f,  0.030f,  0.080f, -0.030f,  0.040f, -0.050f);
    hoursHand.AddLine   (0.040f, -0.050f, 0.040f, -0.360f);
    hoursHand.AddBezier (0.040f, -0.360f, 0.120f, -0.380f,  0.132f, -0.460f,  0.100f, -0.520f);
    hoursHand.AddLine   (0.100f, -0.520f, 0.000f, -0.680f);
    //hoursHand.AddLine   (0.000f, -0.680f, 0.000f, -0.020f);
    //hoursHand.AddBezier (0.000f, -0.020f, 0.030f, -0.020f,  0.030f,  0.020f,  0.000f,  0.020f);
    //hoursHand.AddLine   (0.000f,  0.020f, 0.000f,  0.230f);

    leftHalf = hoursHand.Clone ();
    leftHalf->Transform (&mirror);
    hoursHand.AddPath (leftHalf, true);
    hoursHand.CloseFigure ();
    delete leftHalf;

    Matrix hoursTransformation;
    hoursTransformation.Translate ((REAL) m_center.X, (REAL) m_center.Y);
    hoursTransformation.Scale (scale, scale);
    hoursTransformation.Rotate (((float) m_currentTime.wHour + (float) m_currentTime.wMinute / 60.0f)  * 30.0f);
    hoursHand.Transform (&hoursTransformation);
    
    m_memGraphics->DrawPath (&hoursPen, &hoursHand);    
    m_memGraphics->FillPath (&hoursBrush, &hoursHand);    

    /* Draw the minutes hand:
     */
    minutesHand.AddBezier (0.000f,  0.230f, 0.030f,  0.230f,  0.030f,  0.230f,  0.060f,  0.220f);
    minutesHand.AddBezier (0.060f,  0.220f, 0.050f,  0.190f,  0.040f,  0.090f,  0.040f,  0.050f);
    minutesHand.AddBezier (0.040f,  0.050f, 0.080f,  0.030f,  0.080f, -0.030f,  0.040f, -0.050f);
    minutesHand.AddLine   (0.040f, -0.050f, 0.060f, -0.480f);
    minutesHand.AddLine   (0.060f, -0.480f, 0.000f, -1.000f);  // Minute hand extends to -1.0 * scale
    //minutesHand.AddLine   (0.000f, -1.000f, 0.000f, -0.020f);
    //minutesHand.AddBezier (0.000f, -0.020f, 0.030f, -0.020f,  0.030f,  0.020f,  0.000f,  0.020f);
    //minutesHand.AddLine   (0.000f,  0.020f, 0.000f,  0.230f);

    leftHalf = minutesHand.Clone ();
    leftHalf->Transform (&mirror);
    minutesHand.AddPath (leftHalf, true);
    minutesHand.CloseFigure ();
    delete leftHalf;

    Matrix minutesTransformation;
    minutesTransformation.Translate ((REAL) m_center.X, (REAL) m_center.Y);
    minutesTransformation.Scale (scale, scale);
    minutesTransformation.Rotate (m_currentTime.wMinute * 6.0f);
    minutesHand.Transform (&minutesTransformation);
    
    m_memGraphics->DrawPath (&minutesPen, &minutesHand);    
    m_memGraphics->FillPath (&minutesBrush, &minutesHand);    

    secondsHand.AddEllipse (-0.040f, -0.040f, 0.080f, 0.080f);
    secondsHand.AddLine    (0.000f, -0.020f, 0.000f, -1.100f);
    minutesHand.CloseFigure ();

    Matrix secondsTransformation;
    secondsTransformation.Translate ((REAL) m_center.X, (REAL) m_center.Y);
    secondsTransformation.Scale (scale, scale);
    secondsTransformation.Rotate (m_currentTime.wSecond * 6.0f);
    secondsHand.Transform (&secondsTransformation);
    
    m_memGraphics->DrawPath (&secondsPen, &secondsHand);    
    m_memGraphics->FillPath (&secondsBrush, &secondsHand);    

    /* Draw the offscreen image onto the screen:
     */
    graphics.DrawImage (m_memBitmap, 0, 0);
}
