#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

struct Painter;

struct Font {
	friend Painter;

	Font(const wchar_t *name, int size, int weight = 400, bool italic = false);
	~Font();

private:
	IDWriteTextFormat *m_TextFormat = 0;
};

struct Painter {
	Painter(HWND hwnd);
	~Painter();

	void Begin() const;
	void End() const;

	void Resize(int width, int height) const;
	void Clear(int color) const;

	void SetColor(int color) const;

	void FillRect(float x, float y, float w, float h, float r = 0) const;

	void PutText(const Font &font, const wchar_t *text, float x, float y) const;

private:
	ID2D1HwndRenderTarget *m_RenderTarget = 0;
	ID2D1SolidColorBrush *m_Brush = 0;
};
