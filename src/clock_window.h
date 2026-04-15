#ifndef CLOCK_WINDOW_H
#define CLOCK_WINDOW_H

#include <windows.h>
#include "settings.h"
#include "renderer.h"

class ClockWindow {
public:
    ClockWindow(HINSTANCE hInstance, SettingsManager* settingsMgr);
    ~ClockWindow();
    
    bool Create();
    void Show();
    void Hide();
    void UpdateDisplay();
    void ApplySettings();
    
    HWND GetHandle() const { return m_hwnd; }
    bool IsVisible() const { return m_visible; }
    
private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    
    void OnCreate();
    void OnDestroy();
    void OnPaint();
    void OnTimer();
    void OnLButtonDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnLButtonUp();
    void OnRButtonUp(int x, int y);
    void OnSize(int width, int height);
    
    void StartTimer();
    void StopTimer();
    void SaveWindowPosition();
    void ValidateWindowPosition();
    void UpdateLayeredWindow();
    
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    SettingsManager* m_settingsMgr;
    ClockRenderer* m_renderer;
    
    bool m_visible;
    bool m_dragging;
    POINT m_dragStart;
    RECT m_windowRect;
    
    // For double buffering
    HDC m_memDC;
    HBITMAP m_memBitmap;
    HBITMAP m_oldBitmap;
};

#endif // CLOCK_WINDOW_H

// Made with Bob
