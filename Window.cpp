#include "Window.hpp"

#include <dwmapi.h>

Window::Window(const wchar_t *title, WindowSettings settings) : m_Settings(settings) {
	HINSTANCE hInst = GetModuleHandleW(0);

	m_Hwnd = CreateWindowExW(
		0, WndClass(), title, WS_OVERLAPPEDWINDOW,
		settings.m_X, settings.m_Y, settings.m_Width, settings.m_Height,
		0, 0, hInst, 0
	);

	if(!m_Hwnd) throw L"Cannot create window!";

	SetWindowLongPtrW(m_Hwnd, GWLP_USERDATA, (LONG_PTR) this);

	BOOL dark = TRUE;
	DwmSetWindowAttribute(
		m_Hwnd,
		DWMWA_USE_IMMERSIVE_DARK_MODE,
		&dark,
		sizeof(dark)
	);

	COLORREF color = settings.m_Background;
	DwmSetWindowAttribute(m_Hwnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));

	m_Painter = new Painter(m_Hwnd);

	RECT rc;
	GetClientRect(m_Hwnd, &rc);
	PostMessageW(m_Hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

	ShowWindow(m_Hwnd, SW_SHOW);
}

Window::~Window() {
	delete m_Painter;
	DestroyWindow(m_Hwnd);
}

void Window::Redraw() {
	InvalidateRect(m_Hwnd, 0, FALSE);
}

bool Window::IsOpen() const {
	return IsWindowVisible(m_Hwnd);
}

LPCWSTR Window::WndClass() {
	static ATOM atom = 0;

	if(!atom) {
		WNDCLASSEXW wc = { sizeof(wc) };

		wc.lpfnWndProc = Window::WndProc;
		wc.hInstance = GetModuleHandleW(0);
		wc.lpszClassName = L"WINDOW";
		wc.hCursor = LoadCursor(0, IDC_ARROW);

		atom = RegisterClassExW(&wc);
		if(!atom) throw L"Cannot create Window Class!";
	}

	return MAKEINTATOM(atom);
}

LRESULT Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	Window *window = (Window *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(!window) return DefWindowProc(hwnd, msg, wp, lp);

	switch(msg) {
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);

			window->m_Painter->Begin();
			window->m_Painter->Clear(window->m_Settings.m_Background);
			window->Handle(PaintEvent(*window->m_Painter));
			window->m_Painter->End();

			EndPaint(hwnd, &ps);

			return 0;
		}

		case WM_ERASEBKGND:
		{
			return 1;
		}

		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lp);
			int y = HIWORD(lp);

			window->Handle(MouseMoveEvent(x, y));

			return 0;
		}

		case WM_SIZE:
		{
			UINT w = LOWORD(lp);
			UINT h = HIWORD(lp);

			window->m_Painter->Resize(w, h);
			window->Handle(ResizeEvent(w, h));
			window->Redraw();

			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lp;
			mmi->ptMinTrackSize.x = window->m_Settings.m_MinWidth;
			mmi->ptMinTrackSize.y = window->m_Settings.m_MinHeight;
			return 0;
		}

		case WM_KEYUP:
		case WM_KEYDOWN:
		{
			auto e = KeyInputEvent();
			e.Key = (int) wp;
			e.IsDown = !(lp >> 31);

			int scan = (lp >> 16) & 0xFF;

			BYTE state[256];
			GetKeyboardState(state);

			int n = ToUnicodeEx(
				e.Key, scan, state, &e.Char,
				1, 0, GetKeyboardLayout(0)
			);

			if(n == 0) e.Char = 0;

			e.WithAlt = state[VK_MENU] & 0x80;
			e.WithCaps = state[VK_CAPITAL] & 0x01;
			e.WithCtrl = state[VK_CONTROL] & 0x80;
			e.WithShift = state[VK_SHIFT] & 0x80;

			window->Handle(e);
			return 0;
		}

		case WM_CLOSE:
		{
			CloseEvent event;
			window->Handle(event);
			if(event.CanClose)
				ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

void Window::HandleEvents() {
	MSG msg;

	if(GetMessageW(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void Window::OpenDialog(const wchar_t *title, const wchar_t *msg, DialogType type) {
	MessageBoxW(0, msg, title, (int) type | MB_OK);
}

WindowSettings &WindowSettings::Position(int x, int y) {
	m_X = x;
	m_Y = y;
	return *this;
}

WindowSettings &WindowSettings::Size(int width, int height) {
	m_Width = width;
	m_Height = height;
	return *this;
}

WindowSettings &WindowSettings::MinSize(int width, int height) {
	m_MinWidth = width;
	m_MinHeight = height;
	return *this;
}

WindowSettings &WindowSettings::Background(int color) {
	m_Background = color;
	return *this;
}