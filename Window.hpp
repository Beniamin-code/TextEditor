#pragma once

#include <Windows.h>

#include "Painter.hpp"

struct Window;

struct PaintEvent {
	PaintEvent(Painter &painter) : Painter(painter) { }

	Painter &Painter;
};

struct ResizeEvent {
	ResizeEvent(int width, int height)
		: Width(width), Height(height) { }

	int Width, Height;
};

struct CloseEvent {
	CloseEvent() { }

	bool CanClose = true;
};

struct KeyInputEvent {
	KeyInputEvent() { }

	wchar_t Char = 0;
	int Key = 0;

	bool IsDown = false;
	bool WithShift = false;
	bool WithCaps = false;
	bool WithCtrl = false;
	bool WithAlt = false;
};

struct MouseMoveEvent {
	MouseMoveEvent(float x, float y) : X(x), Y(y) { }

	float X, Y;
};

struct WindowSettings {
	friend Window;

	WindowSettings &Size(int width, int height);
	WindowSettings &Background(int color);

private:
	int m_X = -1, m_Y = -1;
	int m_Width = -1, m_Height = -1;
	int m_Background = 0;
};

enum class DialogType {
	Error = MB_ICONERROR,
	Warning = MB_ICONEXCLAMATION,
	Info = MB_ICONINFORMATION
};

struct Window {
	Window(const wchar_t *title, WindowSettings ws);
	~Window();

	void Redraw();

	bool IsOpen() const;

	virtual void Handle(const PaintEvent &e) { }
	virtual void Handle(const ResizeEvent &e) { }
	virtual void Handle(const KeyInputEvent &e) { }
	virtual void Handle(const MouseMoveEvent &e) { }
	virtual void Handle(CloseEvent &e) { }

	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	Window(Window &&) = delete;
	Window &operator=(Window &&) = delete;

	static void HandleEvents();
	static void OpenDialog(const wchar_t *title, const wchar_t *msg, DialogType type);

private:
	static LPCWSTR WndClass();
	static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	int m_Background;
	HWND m_Handle = 0;
	Painter *m_Painter = 0;
};