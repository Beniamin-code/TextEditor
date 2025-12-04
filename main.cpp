#include <iostream>
#include <Windows.h>
#include <d2d1.h>
#include <dwmapi.h>

ID2D1Factory* factory;
ID2D1HwndRenderTarget* rt;
ID2D1SolidColorBrush* brush;

// undo, redo
// syntax highlghting
// zoom
// search, replace
// copy/paste
// linie, coloana

LRESULT WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	if (m == WM_PAINT) {
		PAINTSTRUCT ps;
		BeginPaint(h, &ps);

		rt->BeginDraw();
		rt->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		D2D1_RECT_F r{ 50,50,150,150 };
		rt->FillRectangle(&r, brush);

		rt->EndDraw();
		EndPaint(h, &ps);
		return 0;
	}

	if (m == WM_SIZE) {
		if (rt) {
			UINT w = LOWORD(l);
			UINT h = HIWORD(l);
			rt->Resize(D2D1_SIZE_U{ w, h });
			float r = (float)w / h;
			brush->SetColor(D2D1::ColorF(r, 1 / r, 0, 1));
		}
		InvalidateRect(h, nullptr, FALSE);
		return DefWindowProc(h, m, w, l);
	}

	if (m == WM_CLOSE) {
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(h, m, w, l);
}

int main() {
	WNDCLASSW wc = { };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandleW(0);
	wc.lpszClassName = L"TextEditor";
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Text Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, 0, 0, wc.hInstance, 0);

	BOOL dark = TRUE;
	DwmSetWindowAttribute(
		hwnd,
		DWMWA_USE_IMMERSIVE_DARK_MODE,
		&dark,
		sizeof(dark)
	);

	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

	RECT rc;
	GetClientRect(hwnd, &rc);

	factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd, D2D1::SizeU(rc.right, rc.bottom)),
		&rt
	);

	rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &brush);

	ShowWindow(hwnd, SW_SHOW);
	
	MSG msg;
	while (GetMessageW(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}