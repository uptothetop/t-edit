#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include "GapBuffer.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

template <class T> void SafeRelease(T **ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow {
    HWND m_hwnd;
    ID2D1Factory *m_pDirect2dFactory;
    ID2D1HwndRenderTarget *m_pRenderTarget;
    ID2D1SolidColorBrush *m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush *m_pCornflowerBlueBrush;
    IDWriteFactory *m_pDWriteFactory;
    IDWriteTextFormat *m_pTextFormat;

    GapBuffer m_buffer;

    HRESULT CreateGraphicsResources() {
        HRESULT hr = S_OK;
        if (!m_pRenderTarget) {
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

            D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
            
            hr = m_pDirect2dFactory->CreateHwndRenderTarget(
                props,
                D2D1::HwndRenderTargetProperties(m_hwnd, size),
                &m_pRenderTarget);

            if (FAILED(hr)) {
                // Fallback to software rendering (useful for Wine or weak VMs)
                props.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
                hr = m_pDirect2dFactory->CreateHwndRenderTarget(
                    props,
                    D2D1::HwndRenderTargetProperties(m_hwnd, size),
                    &m_pRenderTarget);
            }

            if (SUCCEEDED(hr)) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                    &m_pLightSlateGrayBrush);
            }
            if (SUCCEEDED(hr)) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                    &m_pCornflowerBlueBrush);
            }
        }
        return hr;
    }

    void DiscardGraphicsResources() {
        SafeRelease(&m_pRenderTarget);
        SafeRelease(&m_pLightSlateGrayBrush);
        SafeRelease(&m_pCornflowerBlueBrush);
    }

    void OnPaint() {
        HRESULT hr = CreateGraphicsResources();
        if (SUCCEEDED(hr)) {
            PAINTSTRUCT ps;
            BeginPaint(m_hwnd, &ps);

            m_pRenderTarget->BeginDraw();
            m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

            RECT rc;
            GetClientRect(m_hwnd, &rc);

            std::string text = m_buffer.get_text();
            
            // Convert to UTF-16
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), NULL, 0);
            std::wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), &wstrTo[0], size_needed);

            D2D1_RECT_F layoutRect = D2D1::RectF(
                static_cast<FLOAT>(rc.left) + 10,
                static_cast<FLOAT>(rc.top) + 10,
                static_cast<FLOAT>(rc.right) - 10,
                static_cast<FLOAT>(rc.bottom) - 10
            );

            if (!wstrTo.empty()) {
                m_pRenderTarget->DrawText(
                    wstrTo.c_str(),
                    wstrTo.length(),
                    m_pTextFormat,
                    layoutRect,
                    m_pLightSlateGrayBrush
                );
            }

            // Draw cursor (simple representation)
            size_t cursorPos = m_buffer.cursor_pos();
            int subStrSize = MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)cursorPos, NULL, 0);
            std::wstring subStr(subStrSize, 0);
            MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)cursorPos, &subStr[0], subStrSize);

            // Approximate cursor position (DirectWrite provides better APIs for hit testing, this is naive)
            // But sufficient for very first draft. We will need IDWriteTextLayout to do it perfectly.
            
            hr = m_pRenderTarget->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET) {
                hr = S_OK;
                DiscardGraphicsResources();
            }
            EndPaint(m_hwnd, &ps);
        }
    }

    void Resize() {
        if (m_pRenderTarget) {
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
            m_pRenderTarget->Resize(size);
        }
    }

public:
    MainWindow() : m_hwnd(NULL), m_pDirect2dFactory(NULL), m_pRenderTarget(NULL),
                   m_pLightSlateGrayBrush(NULL), m_pCornflowerBlueBrush(NULL),
                   m_pDWriteFactory(NULL), m_pTextFormat(NULL) {
        m_buffer.insert_char('H');
        m_buffer.insert_char('i');
        m_buffer.insert_char('!');
    }

    ~MainWindow() {
        SafeRelease(&m_pDirect2dFactory);
        SafeRelease(&m_pDWriteFactory);
        SafeRelease(&m_pTextFormat);
        DiscardGraphicsResources();
    }

    HRESULT Initialize() {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
        if (SUCCEEDED(hr)) {
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
            );
        }
        if (SUCCEEDED(hr)) {
            hr = m_pDWriteFactory->CreateTextFormat(
                L"Consolas",
                NULL,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                20.0f,
                L"en-us",
                &m_pTextFormat
            );
        }

        WNDCLASS wc = {0};
        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = L"TEditWindowClass";
        wc.hCursor       = LoadCursor(NULL, IDC_IBEAM);
        
        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(
            0,
            L"TEditWindowClass",
            L"t-edit",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            NULL, NULL, GetModuleHandle(NULL), this
        );

        return m_hwnd ? S_OK : E_FAIL;
    }

    void Show() {
        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        MainWindow *pThis = NULL;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (MainWindow*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->m_hwnd = hwnd;
        } else {
            pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis) {
            switch (uMsg) {
                case WM_SIZE:
                    pThis->Resize();
                    return 0;
                case WM_PAINT:
                    pThis->OnPaint();
                    return 0;
                case WM_CHAR: {
                    if (wParam == '\b') {
                        // handled in WM_KEYDOWN
                    } else if (wParam == '\r') {
                        pThis->m_buffer.insert_char('\n');
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (wParam >= 32) {
                        // Needs proper UTF-8 handling ideally, but simple char for now
                        pThis->m_buffer.insert_char((char)wParam);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                }
                case WM_KEYDOWN: {
                    switch (wParam) {
                        case VK_LEFT:
                            pThis->m_buffer.move_cursor(-1);
                            InvalidateRect(hwnd, NULL, FALSE);
                            break;
                        case VK_RIGHT:
                            pThis->m_buffer.move_cursor(1);
                            InvalidateRect(hwnd, NULL, FALSE);
                            break;
                        case VK_BACK:
                            pThis->m_buffer.delete_char();
                            InvalidateRect(hwnd, NULL, FALSE);
                            break;
                        case VK_DELETE:
                            // we need delete_forward in GapBuffer, or just shift right and delete_char
                            break;
                    }
                    return 0;
                }
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow) {
    MainWindow win;
    if (SUCCEEDED(win.Initialize())) {
        win.Show();
        MSG msg = {0};
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
