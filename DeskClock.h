#pragma once

#include <windows.h>
#include <shlobj.h> // for IDeskband2, IObjectWithSite, IPesistStream, and IInputObject
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;


#include <map>
#include <atlstr.h>
#include <atlwin.h>

using namespace std;

#include "resource.h"


class CDeskClock: public IDeskBand2,
                  public IPersistStream,
                  public IObjectWithSite,
                  public IInputObject
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IOleWindow
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL);

    // IDockingWindow
    STDMETHODIMP ShowDW(BOOL fShow);
    STDMETHODIMP CloseDW(DWORD);
    STDMETHODIMP ResizeBorderDW(const RECT *, IUnknown *, BOOL);

    // IDeskBand (needed for all deskbands)
    STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD, DESKBANDINFO *pdbi);

    // IDeskBand2 (needed for glass deskband)
    STDMETHODIMP CanRenderComposited(BOOL *pfCanRenderComposited);
    STDMETHODIMP SetCompositionState(BOOL fCompositionEnabled);
    STDMETHODIMP GetCompositionState(BOOL *pfCompositionEnabled);

    // IPersist
    STDMETHODIMP GetClassID(CLSID *pclsid);

    // IPersistStream
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStream *pStm);
    STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

    // IObjectWithSite
    STDMETHODIMP SetSite(IUnknown *pUnkSite);
    STDMETHODIMP GetSite(REFIID riid, void **ppv);

    // IInputObject
    STDMETHODIMP UIActivateIO(BOOL fActivate, MSG *);
    STDMETHODIMP HasFocusIO();
    STDMETHODIMP TranslateAcceleratorIO(MSG *);

    CDeskClock();

protected:
    ~CDeskClock();
    void SetTime(SYSTEMTIME* time);
    void Draw(HDC dc, RECT rc);
    void DrawBackground(HDC dc, RECT rc);

    HANDLE           m_clockThread;
    DWORD            m_clockThreadID;
    bool             m_running;
    HINSTANCE        m_hInstance;
    CString          m_installDir;

    static DWORD WINAPI ClockThread(void* This);
    void                ClockTick(void);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnFocus(const BOOL fFocus);
    void OnPaint(const HDC hdcIn);

private:
    LONG                m_cRef;                 // ref count of deskband
    IInputObjectSite*   m_pSite;                // parent site that contains deskband
    BOOL                m_fHasFocus;            // whether deskband window currently has focus
    BOOL                m_fIsDirty;             // whether deskband setting has changed
    BOOL                m_fCompositionEnabled;  // whether glass is currently enabled in deskband
    DWORD               m_dwBandID;             // ID of deskband
    HWND                m_hwnd;                 // main window of deskband
    HWND                m_hwndParent;           // parent window of deskband
    Image*              m_backgroundImage;
    unsigned int        m_width, m_height;
    Point               m_center;
    int                 m_clockX, m_clockY;
    unsigned int        m_clockSize;
    SYSTEMTIME          m_currentTime;
    GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR           m_gdiplusToken;
};

