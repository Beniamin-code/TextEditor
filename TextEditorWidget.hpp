#pragma once

#include "Widget.hpp"
#include "PieceTable.hpp"
#include "Window.hpp"
#include "Painter.hpp"

#include <string>

struct TextEditorWidget : Widget {
	TextEditorWidget(Font *font);

	void Resize(const Constraint &parentCon) override;
	void Paint(const Painter &painter) override;

	void OnKey(const KeyInputEvent &e);
	void OnMouseMove(const MouseMoveEvent &e);
	void OnMouseClick(int x, int y);

	bool LoadFromFile(const std::wstring &path);
	bool SaveToFile(const std::wstring &path);

protected:
	PieceTable m_Doc { "" };

	Font *m_Font = nullptr;

	size_t m_Caret = 0;
	size_t m_SelectionStart = SIZE_MAX;
	size_t m_SelectionEnd = SIZE_MAX;

	size_t m_FirstVisibleLine = 0;
	float m_LineHeight = 18.0f;
	int m_LeftPadding = 8;
};