#include "clock_window.h"
#include "../resources/resource.h"
#include "../resources/app_strings.h"
#include <windowsx.h>

ClockWindow::ClockWindow(HINSTANCE hInstance, SettingsManager* settingsMgr)
    : m_hInstance(hInstance)
    , m_hwnd(NULL)
    , m_settingsMgr(settingsMgr)
    , m_renderer(nullptr)
    , m_visible(false)
    , m_dragging(false)
    , m_memDC(NULL)
    , m_memBitmap(NULL)
    , m_oldBitmap(NULL) {
    m_dragStart.x = 0;
    m_dragStart.y = 0;
    m_windowRect.left = 0;
    m_windowRect.top = 0;
    m_windowRect.right = 0;
    m_windowRect.bottom = 0;
}

ClockWindow::~ClockWindow() {
    if (m_renderer) {
        delete m_renderer;
    }
    
    if (m_memDC) {
        if (m_oldBitmap) {
            SelectObject(m_memDC, m_oldBitmap);
        }
        if (m_memBitmap) {
            DeleteObject(m_memBitmap);
        }
        DeleteDC(m_memDC);
    }
}

bool ClockWindow::Create() {
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = APP_WINDOW_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    wc.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    
    if (!RegisterClassEx(&wc)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }
    
    // Get settings
    const ClockSettings& settings = m_settingsMgr->GetSettings();
    
    // Determine window position
    int x = settings.windowX;
    int y = settings.windowY;
    int width = settings.windowWidth;
    int height = settings.windowHeight;
    
    // If position is default, center on screen
    if (x == CW_USEDEFAULT || y == CW_USEDEFAULT) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        x = (screenWidth - width) / 2;
        y = (screenHeight - height) / 2;
    }
    
    // Create layered window
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    if (settings.alwaysOnTop) {
        exStyle |= WS_EX_TOPMOST;
    }
    
    m_hwnd = CreateWindowEx(
        exStyle,
        APP_WINDOW_CLASS,
        APP_NAME,
        WS_POPUP,  // No WS_SIZEBOX to avoid border
        x, y, width, height,
        NULL,
        NULL,
        m_hInstance,
        this  // Pass this pointer to WM_CREATE
    );
    
    if (m_hwnd == NULL) {
        return false;
    }
    
    // Don't use SetLayeredWindowAttributes - we'll use UpdateLayeredWindow for per-pixel alpha
    // This allows separate background and text transparency
    
    return true;
}

LRESULT CALLBACK ClockWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ClockWindow* pThis = nullptr;
    
    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<ClockWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<ClockWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (pThis) {
        return pThis->HandleMessage(msg, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT ClockWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            OnCreate();
            return 0;
            
        case WM_DESTROY:
            OnDestroy();
            return 0;
            
        case WM_PAINT:
            OnPaint();
            return 0;
            
        case WM_TIMER:
            if (wParam == TIMER_UPDATE) {
                OnTimer();
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_MOUSEMOVE:
            OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_LBUTTONUP:
            OnLButtonUp();
            return 0;
            
        case WM_RBUTTONUP:
            OnRButtonUp(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_NCHITTEST: {
            // Custom hit testing for borderless resize
            LRESULT hit = DefWindowProc(m_hwnd, msg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(m_hwnd, &pt);
                
                RECT rect;
                GetClientRect(m_hwnd, &rect);
                
                const int RESIZE_BORDER = 8;
                bool onLeft = pt.x < RESIZE_BORDER;
                bool onRight = pt.x > rect.right - RESIZE_BORDER;
                bool onTop = pt.y < RESIZE_BORDER;
                bool onBottom = pt.y > rect.bottom - RESIZE_BORDER;
                
                if (onLeft && onTop) return HTTOPLEFT;
                if (onRight && onTop) return HTTOPRIGHT;
                if (onLeft && onBottom) return HTBOTTOMLEFT;
                if (onRight && onBottom) return HTBOTTOMRIGHT;
                if (onLeft) return HTLEFT;
                if (onRight) return HTRIGHT;
                if (onTop) return HTTOP;
                if (onBottom) return HTBOTTOM;
            }
            return hit;
        }
            
        case WM_ERASEBKGND:
            return 1; // Prevent flicker
    }
    
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

void ClockWindow::OnCreate() {
    // Create renderer
    m_renderer = new ClockRenderer();
    if (!m_renderer->Initialize()) {
        MessageBox(m_hwnd, L"Failed to initialize renderer!", APP_NAME, MB_ICONERROR | MB_OK);
    }
    
    // Create memory DC for double buffering
    HDC hdc = GetDC(m_hwnd);
    m_memDC = CreateCompatibleDC(hdc);
    ReleaseDC(m_hwnd, hdc);
}

void ClockWindow::OnDestroy() {
    StopTimer();
    SaveWindowPosition();
}

void ClockWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    
    if (m_renderer) {
        RECT rect;
        GetClientRect(m_hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        // Ensure memory bitmap is correct size
        if (m_memBitmap == NULL ||
            rect.right != m_windowRect.right ||
            rect.bottom != m_windowRect.bottom) {
            
            if (m_memBitmap) {
                SelectObject(m_memDC, m_oldBitmap);
                DeleteObject(m_memBitmap);
            }
            
            // Create 32-bit bitmap for per-pixel alpha
            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = width;
            bmi.bmiHeader.biHeight = -height; // Top-down DIB
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            
            void* pBits = NULL;
            m_memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
            m_oldBitmap = (HBITMAP)SelectObject(m_memDC, m_memBitmap);
            m_windowRect = rect;
        }
        
        // Render to memory DC with per-pixel alpha
        const ClockSettings& settings = m_settingsMgr->GetSettings();
        m_renderer->Render(m_memDC, &rect, settings);
        
        // Use UpdateLayeredWindow for per-pixel alpha blending
        POINT ptSrc = {0, 0};
        POINT ptDst = {rect.left, rect.top};
        SIZE sizeWnd = {width, height};
        BLENDFUNCTION blend = {};
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255; // Don't multiply - use per-pixel alpha only
        blend.AlphaFormat = AC_SRC_ALPHA; // Use per-pixel alpha from bitmap
        
        // Get window position
        RECT windowRect;
        GetWindowRect(m_hwnd, &windowRect);
        ptDst.x = windowRect.left;
        ptDst.y = windowRect.top;
        
        ::UpdateLayeredWindow(m_hwnd, hdc, &ptDst, &sizeWnd, m_memDC, &ptSrc, 0, &blend, ULW_ALPHA);
    }
    
    EndPaint(m_hwnd, &ps);
}

void ClockWindow::OnTimer() {
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void ClockWindow::OnLButtonDown(int x, int y) {
    SetCapture(m_hwnd);
    GetCursorPos(&m_dragStart);
    GetWindowRect(m_hwnd, &m_windowRect);
    m_dragging = true;
}

void ClockWindow::OnMouseMove(int x, int y) {
    if (m_dragging) {
        POINT current;
        GetCursorPos(&current);
        
        int dx = current.x - m_dragStart.x;
        int dy = current.y - m_dragStart.y;
        
        SetWindowPos(m_hwnd, NULL,
                    m_windowRect.left + dx,
                    m_windowRect.top + dy,
                    0, 0,
                    SWP_NOSIZE | SWP_NOZORDER);
    }
}

void ClockWindow::OnLButtonUp() {
    if (m_dragging) {
        ReleaseCapture();
        m_dragging = false;
        SaveWindowPosition();
    }
}

void ClockWindow::OnRButtonUp(int x, int y) {
    // Context menu will be handled by tray icon
    // This is here for future direct window context menu if needed
}

void ClockWindow::OnSize(int width, int height) {
    // Enforce minimum size
    if (width < 100) width = 100;
    if (height < 50) height = 50;
    
    // Save new size
    ClockSettings& settings = m_settingsMgr->GetSettings();
    settings.windowWidth = width;
    settings.windowHeight = height;
    m_settingsMgr->SaveSettings();
    
    // Force redraw
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void ClockWindow::Show() {
    if (!m_visible) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        m_visible = true;
        StartTimer();
        
        // Force initial render for layered window
        InvalidateRect(m_hwnd, NULL, FALSE);
        UpdateWindow(m_hwnd);
        
        // Update settings
        ClockSettings& settings = m_settingsMgr->GetSettings();
        settings.clockVisible = true;
        m_settingsMgr->SaveSettings();
    }
}

void ClockWindow::Hide() {
    if (m_visible) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
        StopTimer();
        
        // Update settings
        ClockSettings& settings = m_settingsMgr->GetSettings();
        settings.clockVisible = false;
        m_settingsMgr->SaveSettings();
    }
}

void ClockWindow::UpdateDisplay() {
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void ClockWindow::ApplySettings() {
    const ClockSettings& settings = m_settingsMgr->GetSettings();
    
    // Update always on top
    HWND insertAfter = settings.alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(m_hwnd, insertAfter, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    // Restart timer with appropriate interval
    if (m_visible) {
        StopTimer();
        StartTimer();
    }
    
    // Force redraw (transparency is applied in OnPaint via UpdateLayeredWindow)
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void ClockWindow::StartTimer() {
    const ClockSettings& settings = m_settingsMgr->GetSettings();
    UINT interval = settings.showSeconds ? 1000 : 60000;
    SetTimer(m_hwnd, TIMER_UPDATE, interval, NULL);
}

void ClockWindow::StopTimer() {
    KillTimer(m_hwnd, TIMER_UPDATE);
}

void ClockWindow::SaveWindowPosition() {
    RECT rect;
    GetWindowRect(m_hwnd, &rect);
    
    ClockSettings& settings = m_settingsMgr->GetSettings();
    settings.windowX = rect.left;
    settings.windowY = rect.top;
    settings.windowWidth = rect.right - rect.left;
    settings.windowHeight = rect.bottom - rect.top;
    
    m_settingsMgr->SaveSettings();
}

void ClockWindow::ValidateWindowPosition() {
    // This is called during settings load to ensure window is on screen
    // Implementation is in SettingsManager::LoadSettings()
}

void ClockWindow::UpdateLayeredWindow() {
    // For future use if we need more advanced layered window updates
}

// Made with Bob
