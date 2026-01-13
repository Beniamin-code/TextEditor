#include "Painter.hpp"

static ID2D1Factory *D2DFactory() {
	static ID2D1Factory *factory = 0;

	if(!factory) {
		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&factory
		);

		if(hr != S_OK) throw L"Cannot create Direct2D Factory!";
	}

	return factory;
}

static IDWriteFactory *DWriteFactory() {
	static IDWriteFactory *factory = 0;

	if(!factory) {
		HRESULT hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			(IUnknown **) &factory
		);

		if(hr != S_OK) throw L"Cannot create DirectWrite Factory!";
	}

	return factory;
}

Painter::Painter(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);

	HRESULT hr = D2DFactory()->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(rc.right, rc.bottom)
		),
		&m_RenderTarget
	);

	if(hr != S_OK) throw L"Cannot create Render Target!";

	hr = m_RenderTarget->CreateSolidColorBrush(D2D1::ColorF::ColorF(0), &m_Brush);
	if(hr != S_OK) throw L"Cannot create Brush!";
}

Painter::~Painter() {
	if(m_Brush) m_Brush->Release();
	if(m_RenderTarget) m_RenderTarget->Release();
}

void Painter::Begin() const {
	m_RenderTarget->BeginDraw();
}

void Painter::End() const {
	m_RenderTarget->EndDraw();
}

void Painter::SetDpi(int dpi) {
	m_RenderTarget->SetDpi((float) dpi, (float) dpi);
}

void Painter::Resize(int width, int height) const {
	m_RenderTarget->Resize(D2D1_SIZE_U { (unsigned) width, (unsigned) height });
}

void Painter::Clear(int color) const {
	m_RenderTarget->Clear(D2D1::ColorF(color));
}

void Painter::SetColor(int color) const {
	m_Brush->SetColor(D2D1::ColorF(color));
}

void Painter::FillRect(float x, float y, float w, float h, float r) const {
	D2D1_ROUNDED_RECT rect {
		D2D1_RECT_F { x, y, x + w, y + h },
		r, r
	};

	m_RenderTarget->FillRoundedRectangle(&rect, m_Brush);
}

void Painter::PutText(const Font &font, const wchar_t *text, float x, float y) const {
	m_RenderTarget->DrawTextW(
		text,
		(int) wcslen(text),
		font.m_TextFormat,
		D2D1::RectF(x, y, 1e+6, 1e+6),
		m_Brush,
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
	);
}

void Painter::MeasureText(const Font &font, const wchar_t *text, float &outWidth, float &outHeight) const {
	if(!text || !*text) { outWidth = 0; outHeight = 0; return; }

	IDWriteTextLayout *layout = nullptr;
	HRESULT hr = DWriteFactory()->CreateTextLayout(
		text,
		(UINT32) wcslen(text),
		font.m_TextFormat,
		10000.0f,
		10000.0f,
		&layout
	);

	if(hr != S_OK || !layout) {
		outWidth = 0;
		outHeight = 0;
		return;
	}

	DWRITE_TEXT_METRICS metrics;
	hr = layout->GetMetrics(&metrics);
	if(hr == S_OK) {
		outWidth = metrics.widthIncludingTrailingWhitespace;
		outHeight = metrics.height;
	} else {
		outWidth = 0;
		outHeight = 0;
	}

	layout->Release();
}

Font::Font(const wchar_t *name, int size, int weight, bool italic) {
	DWRITE_FONT_STYLE style = italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

	HRESULT hr = DWriteFactory()->CreateTextFormat(
		name, 0,
		(DWRITE_FONT_WEIGHT) weight,
		style,
		DWRITE_FONT_STRETCH_NORMAL,
		(float) size, L"en-US",
		&m_TextFormat
	);

	if(hr != S_OK) throw L"Cannot create Font!";
}

Font::~Font() {
	if(m_TextFormat)
		m_TextFormat->Release();
}