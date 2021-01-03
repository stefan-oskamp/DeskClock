
#include <windows.h>
#include <uxtheme.h>
#include "DeskClock.h"

#define RECTWIDTH(x)   ((x).right - (x).left + 1)
#define RECTHEIGHT(x)  ((x).bottom - (x).top + 1)

extern ULONG        g_cDllRef;
extern HINSTANCE    g_hInst;

extern CLSID CLSID_DeskClock;

static const WCHAR g_szDeskClockClass[] = L"DeskClockClass";

//////////////////
// static
DWORD WINAPI CDeskClock::ClockThread(void* This)
{
    CDeskClock* self = (CDeskClock*)This;
    while (self->m_running) {
        self->ClockTick();
        Sleep(1000);
    }
    return 0L;
}

CDeskClock::CDeskClock() :
    m_cRef(1), m_pSite(NULL), m_fHasFocus(FALSE), m_fIsDirty(FALSE), m_dwBandID(0), m_hwnd(NULL), m_hwndParent(NULL)
{
    m_hInstance = g_hInst;

    // Initialize GDI+.
    GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

    WCHAR moduleName[_MAX_PATH];
    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR fileName[_MAX_FNAME];
    WCHAR extension[_MAX_EXT];
    ::GetModuleFileName(m_hInstance, moduleName, _MAX_PATH);
    _wsplitpath_s (moduleName, drive, dir, fileName, extension);
    m_installDir = CString(drive);
    m_installDir += CString(dir);
    m_clockThread = NULL;
    m_clockThreadID = -1L;
    m_running = false;
    m_width = 0;
    m_height = 0;
    m_clockX = 0;
    m_clockY = 0;
    m_clockSize = 0;
    GetLocalTime(&m_currentTime);
    m_backgroundImage = new Image(m_installDir + CString ("SiemensClock-1024x1024.png"));
    // Start tickin':
    m_running = true;
    m_clockThread = CreateThread(NULL, 0, ClockThread, this, 0, &m_clockThreadID);

    InterlockedIncrement(&g_cDllRef);
}

CDeskClock::~CDeskClock()
{
    if (m_pSite) {
        m_pSite->Release();
    }
    if (m_clockThread != NULL) {
        m_running = false;
        TerminateThread(m_clockThread, 0);
        m_clockThread = NULL;
        m_clockThreadID = -1L;
    }
    GdiplusShutdown(m_gdiplusToken);
    InterlockedDecrement(&g_cDllRef);
}

//
// IUnknown
//
STDMETHODIMP CDeskClock::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(IID_IUnknown, riid)       ||
        IsEqualIID(IID_IOleWindow, riid)     ||
        IsEqualIID(IID_IDockingWindow, riid) ||
        IsEqualIID(IID_IDeskBand, riid)      ||
        IsEqualIID(IID_IDeskBand2, riid))
    {
        *ppv = static_cast<IOleWindow *>(this);
    }
    else if (IsEqualIID(IID_IPersist, riid) ||
             IsEqualIID(IID_IPersistStream, riid))
    {
        *ppv = static_cast<IPersist *>(this);
    }
    else if (IsEqualIID(IID_IObjectWithSite, riid))
    {
        *ppv = static_cast<IObjectWithSite *>(this);
    }
    else if (IsEqualIID(IID_IInputObject, riid))
    {
        *ppv = static_cast<IInputObject *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppv = NULL;
    }

    if (*ppv) {
        AddRef();
    }

    return hr;
}

STDMETHODIMP_(ULONG) CDeskClock::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CDeskClock::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef) {
        delete this;
    }

    return cRef;
}

//
// IOleWindow
//
STDMETHODIMP CDeskClock::GetWindow(HWND *phwnd)
{
    *phwnd = m_hwnd;
    return S_OK;
}

STDMETHODIMP CDeskClock::ContextSensitiveHelp(BOOL)
{
    return E_NOTIMPL;
}

//
// IDockingWindow
//
STDMETHODIMP CDeskClock::ShowDW(BOOL fShow)
{
    if (m_hwnd) {
        ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    }

    return S_OK;
}

STDMETHODIMP CDeskClock::CloseDW(DWORD)
{
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }

    return S_OK;
}

STDMETHODIMP CDeskClock::ResizeBorderDW(const RECT *, IUnknown *, BOOL)
{
    return E_NOTIMPL;
}

//
// IDeskBand
//
STDMETHODIMP CDeskClock::GetBandInfo(DWORD dwBandID, DWORD, DESKBANDINFO *pdbi)
{
    HRESULT hr = E_INVALIDARG;

    if (pdbi) {
        m_dwBandID = dwBandID;

        if (pdbi->dwMask & DBIM_MINSIZE) {
            pdbi->ptMinSize.x = 32;
            pdbi->ptMinSize.y = 32;
        }

        if (pdbi->dwMask & DBIM_MAXSIZE) {
            pdbi->ptMaxSize.y = -1;
        }

        if (pdbi->dwMask & DBIM_INTEGRAL) {
            pdbi->ptIntegral.y = 1;
        }

        if (pdbi->dwMask & DBIM_ACTUAL) {
            pdbi->ptActual.x = 64;
            pdbi->ptActual.y = 64;
        }

        if (pdbi->dwMask & DBIM_TITLE) {
            // Don't show title by removing this flag.
            pdbi->dwMask &= ~DBIM_TITLE;
        }

        if (pdbi->dwMask & DBIM_MODEFLAGS) {
            pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;
        }

        if (pdbi->dwMask & DBIM_BKCOLOR) {
            // Use the default background color by removing this flag.
            pdbi->dwMask &= ~DBIM_BKCOLOR;
        }

        hr = S_OK;
    }

    return hr;
}

//
// IDeskBand2
//
STDMETHODIMP CDeskClock::CanRenderComposited(BOOL *pfCanRenderComposited)
{
    *pfCanRenderComposited = TRUE;

    return S_OK;
}

STDMETHODIMP CDeskClock::SetCompositionState(BOOL fCompositionEnabled)
{
    m_fCompositionEnabled = fCompositionEnabled;

    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);

    return S_OK;
}

STDMETHODIMP CDeskClock::GetCompositionState(BOOL *pfCompositionEnabled)
{
    *pfCompositionEnabled = m_fCompositionEnabled;

    return S_OK;
}

//
// IPersist
//
STDMETHODIMP CDeskClock::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_DeskClock;
    return S_OK;
}

//
// IPersistStream
//
STDMETHODIMP CDeskClock::IsDirty()
{
    return m_fIsDirty ? S_OK : S_FALSE;
}

STDMETHODIMP CDeskClock::Load(IStream * /*pStm*/)
{
    return S_OK;
}

STDMETHODIMP CDeskClock::Save(IStream * /*pStm*/, BOOL fClearDirty)
{
    if (fClearDirty) {
        m_fIsDirty = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDeskClock::GetSizeMax(ULARGE_INTEGER * /*pcbSize*/)
{
    return E_NOTIMPL;
}

//
// IObjectWithSite
//
STDMETHODIMP CDeskClock::SetSite(IUnknown *pUnkSite)
{
    HRESULT hr = S_OK;

    m_hwndParent = NULL;

    if (m_pSite) {
        m_pSite->Release();
    }

    if (pUnkSite)
    {
        IOleWindow *pow;
        hr = pUnkSite->QueryInterface(IID_IOleWindow, reinterpret_cast<void **>(&pow));
        if (SUCCEEDED(hr)) {
            hr = pow->GetWindow(&m_hwndParent);
            if (SUCCEEDED(hr)) {
                WNDCLASSW wc = { 0 };
                wc.style         = CS_HREDRAW | CS_VREDRAW;
                wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
                wc.hInstance     = g_hInst;
                wc.lpfnWndProc   = WndProc;
                wc.lpszClassName = g_szDeskClockClass;
                wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0));

                RegisterClassW(&wc);

                CreateWindowExW(0,
                                g_szDeskClockClass,
                                NULL,
                                WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                0,
                                0,
                                0,
                                0,
                                m_hwndParent,
                                NULL,
                                g_hInst,
                                this);

                if (!m_hwnd) {
                    hr = E_FAIL;
                }
            }

            pow->Release();
        }

        hr = pUnkSite->QueryInterface(IID_IInputObjectSite, reinterpret_cast<void **>(&m_pSite));
    }

    return hr;
}

STDMETHODIMP CDeskClock::GetSite(REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    if (m_pSite) {
        hr =  m_pSite->QueryInterface(riid, ppv);
    }
    else {
        *ppv = NULL;
    }

    return hr;
}

//
// IInputObject
//
STDMETHODIMP CDeskClock::UIActivateIO(BOOL fActivate, MSG *)
{
    if (fActivate) {
        SetFocus(m_hwnd);
    }

    return S_OK;
}

STDMETHODIMP CDeskClock::HasFocusIO()
{
    return m_fHasFocus ? S_OK : S_FALSE;
}

STDMETHODIMP CDeskClock::TranslateAcceleratorIO(MSG *)
{
    return S_FALSE;
};

void CDeskClock::OnFocus(const BOOL fFocus)
{
    m_fHasFocus = fFocus;

    if (m_pSite) {
        m_pSite->OnFocusChangeIS(static_cast<IOleWindow*>(this), m_fHasFocus);
    }
}

void CDeskClock::OnPaint(const HDC hdcIn)
{
    HDC hdc = hdcIn;
    PAINTSTRUCT ps;
    static WCHAR szContent[] = L"Desk Clock";
    static WCHAR szContentGlass[] = L"DeskClock (Glass)";

    GetLocalTime(&m_currentTime);

    if (!hdc) {
        hdc = BeginPaint(m_hwnd, &ps);
    }

    if (hdc) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        SIZE size;

        if (m_fCompositionEnabled) {
            HTHEME hTheme = OpenThemeData(NULL, L"BUTTON");
            if (hTheme) {
                HDC hdcPaint = NULL;
                HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);

                DrawThemeParentBackground(m_hwnd, hdcPaint, &rc);

                DrawBackground(hdcPaint, rc);
                Draw(hdcPaint, rc);

                EndBufferedPaint(hBufferedPaint, TRUE);

                CloseThemeData(hTheme);
            }
        }
        else {
            SetBkColor(hdc, RGB(255, 255, 0));
            GetTextExtentPointW(hdc, szContent, ARRAYSIZE(szContent), &size);
            TextOutW(hdc,
                     (RECTWIDTH(rc) - size.cx) / 2,
                     (RECTHEIGHT(rc) - size.cy) / 2,
                     szContent,
                     ARRAYSIZE(szContent));
        }
    }

    if (!hdcIn) {
        EndPaint(m_hwnd, &ps);
    }
}

LRESULT CALLBACK CDeskClock::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    CDeskClock *pDeskBand = reinterpret_cast<CDeskClock *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
        pDeskBand = reinterpret_cast<CDeskClock *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        pDeskBand->m_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDeskBand));
        break;

    case WM_SETFOCUS:
        pDeskBand->OnFocus(TRUE);
        break;

    case WM_KILLFOCUS:
        pDeskBand->OnFocus(FALSE);
        break;

    case WM_PAINT:
        pDeskBand->OnPaint(NULL);
        break;

    case WM_PRINTCLIENT:
        pDeskBand->OnPaint(reinterpret_cast<HDC>(wParam));
        break;

    case WM_ERASEBKGND:
        if (pDeskBand->m_fCompositionEnabled) {
            lResult = 1;
        }
        break;
    }

    if (uMsg != WM_ERASEBKGND) {
        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lResult;
}


void CDeskClock::DrawBackground(HDC dc, RECT rc)
{
    Graphics graphics(dc);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);

    unsigned int width = RECTWIDTH(rc);
    unsigned int height = RECTHEIGHT(rc);

    m_width = width;
    m_height = height;
    m_clockSize = min(m_width, m_height) - 2;
    m_center = Point((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);
    m_clockX = (m_width - m_clockSize) / 2;
    m_clockY = (m_height - m_clockSize) / 2;

    // Draw the clock background:
    if (m_backgroundImage) {
        graphics.DrawImage(m_backgroundImage, m_clockX, m_clockY, m_clockSize, m_clockSize);
    }

} // End of DrawBackground ().


void CDeskClock::Draw(HDC dc, RECT rc)
{
    if (!this) return;

    Graphics graphics(dc);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);

    Color        black(255, 0, 0, 0);
    Color        red(255, 220, 0, 0);

    Pen          hoursPen(black);
    SolidBrush   hoursBrush(black);
    Pen          minutesPen(black);
    SolidBrush   minutesBrush(black);
    Pen          secondsPen(red);
    SolidBrush   secondsBrush(red);
    GraphicsPath hoursHand(FillModeAlternate), minutesHand(FillModeAlternate), secondsHand(FillModeAlternate);
    GraphicsPath *leftHalf = NULL;
    float        clockRadius = m_clockSize / 2.0f;
    float        scale = clockRadius * 0.65f;
    Matrix       mirror; mirror.Scale(-1.0f, 1.0f);

    /* Draw the hours hand:
     */
    hoursHand.AddBezier(0.000f, 0.230f, 0.030f, 0.230f, 0.030f, 0.230f, 0.060f, 0.220f);
    hoursHand.AddBezier(0.060f, 0.220f, 0.050f, 0.190f, 0.040f, 0.090f, 0.040f, 0.050f);
    hoursHand.AddBezier(0.040f, 0.050f, 0.080f, 0.030f, 0.080f, -0.030f, 0.040f, -0.050f);
    hoursHand.AddLine(0.040f, -0.050f, 0.040f, -0.360f);
    hoursHand.AddBezier(0.040f, -0.360f, 0.120f, -0.380f, 0.132f, -0.460f, 0.100f, -0.520f);
    hoursHand.AddLine(0.100f, -0.520f, 0.000f, -0.680f);

    leftHalf = hoursHand.Clone();
    leftHalf->Transform(&mirror);
    hoursHand.AddPath(leftHalf, true);
    hoursHand.CloseFigure();
    delete leftHalf;

    Matrix hoursTransformation;
    hoursTransformation.Translate((REAL)m_center.X, (REAL)m_center.Y);
    hoursTransformation.Scale(scale, scale);
    hoursTransformation.Rotate(((float)m_currentTime.wHour + (float)m_currentTime.wMinute / 60.0f) * 30.0f);
    hoursHand.Transform(&hoursTransformation);

    graphics.DrawPath(&hoursPen, &hoursHand);
    graphics.FillPath(&hoursBrush, &hoursHand);

    /* Draw the minutes hand:
     */
    minutesHand.AddBezier(0.000f, 0.230f, 0.030f, 0.230f, 0.030f, 0.230f, 0.060f, 0.220f);
    minutesHand.AddBezier(0.060f, 0.220f, 0.050f, 0.190f, 0.040f, 0.090f, 0.040f, 0.050f);
    minutesHand.AddBezier(0.040f, 0.050f, 0.080f, 0.030f, 0.080f, -0.030f, 0.040f, -0.050f);
    minutesHand.AddLine(0.040f, -0.050f, 0.060f, -0.480f);
    minutesHand.AddLine(0.060f, -0.480f, 0.000f, -1.000f);  // Minute hand extends to -1.0 * scale

    leftHalf = minutesHand.Clone();
    leftHalf->Transform(&mirror);
    minutesHand.AddPath(leftHalf, true);
    minutesHand.CloseFigure();
    delete leftHalf;

    Matrix minutesTransformation;
    minutesTransformation.Translate((REAL)m_center.X, (REAL)m_center.Y);
    minutesTransformation.Scale(scale, scale);
    minutesTransformation.Rotate(m_currentTime.wMinute * 6.0f);
    minutesHand.Transform(&minutesTransformation);

    graphics.DrawPath(&minutesPen, &minutesHand);
    graphics.FillPath(&minutesBrush, &minutesHand);

    secondsHand.AddEllipse(-0.040f, -0.040f, 0.080f, 0.080f);
    secondsHand.AddLine(0.000f, -0.020f, 0.000f, -1.100f);
    minutesHand.CloseFigure();

    Matrix secondsTransformation;
    secondsTransformation.Translate((REAL)m_center.X, (REAL)m_center.Y);
    secondsTransformation.Scale(scale, scale);
    secondsTransformation.Rotate(m_currentTime.wSecond * 6.0f);
    secondsHand.Transform(&secondsTransformation);

    graphics.DrawPath(&secondsPen, &secondsHand);
    graphics.FillPath(&secondsBrush, &secondsHand);
}


void CDeskClock::ClockTick(void)
{
    GetLocalTime(&m_currentTime);
    // Redraw the clock:
    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);
}
