#include "Window.hpp"

#include <dwmapi.h>

Window::Window(const wchar_t *title, WindowSettings settings) {
	m_Background = settings.m_Background;

	HINSTANCE hInst = GetModuleHandleW(0);
	m_Handle = CreateWindowExW(
		0, WndClass(), title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, hInst, (void *) this
	);

	if(!m_Handle) throw L"Cannot create window!";

	BOOL dark = TRUE;
	DwmSetWindowAttribute(
		m_Handle,
		DWMWA_USE_IMMERSIVE_DARK_MODE,
		&dark,
		sizeof(dark)
	);

	COLORREF color =
		((settings.m_Background & 0x00FF00)) |
		((settings.m_Background & 0x0000FF) << 16) |
		((settings.m_Background & 0xFF0000) >> 16);

	DwmSetWindowAttribute(m_Handle, DWMWA_CAPTION_COLOR, &color, sizeof(color));

	m_Painter = new Painter(m_Handle);

	{
		RECT clientRect;
		GetClientRect(m_Handle, &clientRect);

		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;
		PostMessageW(m_Handle, WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, height));
	}

	ShowWindow(m_Handle, SW_SHOW);
}

Window::~Window() {
	SetWindowLongPtrW(m_Handle, GWLP_USERDATA, 0);

	delete m_Painter;
	DestroyWindow(m_Handle);
}

void Window::Redraw() {
	InvalidateRect(m_Handle, 0, FALSE);
}

bool Window::IsOpen() const {
	return IsWindowVisible(m_Handle);
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
	if(msg == WM_NCCREATE) {
		CREATESTRUCT *create = (CREATESTRUCT *) lp;
		Window *window = (Window *) create->lpCreateParams;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) window);
		return DefWindowProcW(hwnd, msg, wp, lp);
	}

	Window *window = (Window *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if(!window) return DefWindowProcW(hwnd, msg, wp, lp);

	switch(msg) {
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);

			window->m_Painter->Begin();
			window->m_Painter->Clear(window->m_Background);
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
			float x = LOWORD(lp);
			float y = HIWORD(lp);

			window->Handle(MouseMoveEvent(x, y));

			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lp);
			int y = HIWORD(lp);
			window->Handle(MouseClickEvent(x, y));
			return 0;
		}

		case WM_SIZE:
		{
			int width = LOWORD(lp);
			int height = HIWORD(lp);

			window->m_Painter->Resize(width, height);
			window->Handle(ResizeEvent(width, height));
			window->Redraw();

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
			if(!GetKeyboardState(state))
				throw L"Cannot get Keyboard state!";

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

	if(GetMessageW(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void Window::OpenDialog(const wchar_t *title, const wchar_t *msg, DialogType type) {
	MessageBoxW(0, msg, title, (int) type | MB_OK);
}

WindowSettings &WindowSettings::Size(int width, int height) {
	m_Width = width;
	m_Height = height;
	return *this;
}

WindowSettings &WindowSettings::Background(int color) {
	m_Background = color;
	return *this;
}