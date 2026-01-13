#include "TextEditorWidget.hpp"

#include <fstream>
#include <string>
#include <cassert>

TextEditorWidget::TextEditorWidget(Font *font)
	: m_Font(font) {
	m_Constraint.Min(100, 100);
	m_Constraint.Max(-1, -1);
	m_Constraint.Width = 400;
	m_Constraint.Height = 300;
}

void TextEditorWidget::Resize(const Constraint &parentCon) {
	Widget::Resize(parentCon);
	// Keep same position; size already set by parent constraints
}

void TextEditorWidget::Paint(const Painter &painter) {
	// Determine line height using font metrics
	float w, h;
	painter.MeasureText(*m_Font, L"Mg", w, h);
	if(h > 1.0f) m_LineHeight = h;

	// Background for editor
	painter.SetColor(0xFFFFFF);
	painter.FillRect(m_X, m_Y, m_Width, m_Height);

	// Clip region: simple approach - don't set D2D clip, just avoid drawing outside
	int linesVisible = (int) ((float) m_Height / m_LineHeight);
	size_t totalLines = m_Doc.GetLineCount();

	for(int i = 0; i < linesVisible; ++i) {
		size_t lineIndex = m_FirstVisibleLine + i;
		if(lineIndex >= totalLines) break;

		std::string lineBytes = m_Doc.GetLine(lineIndex);
		// convert to wchar_t (assume ASCII/UTF-8 simple cases)
		std::wstring wline;
		wline.reserve(lineBytes.size());
		for(char c : lineBytes) wline.push_back((unsigned char) c);

		float tx = (float) (m_X + m_LeftPadding);
		float ty = (float) (m_Y + i * m_LineHeight + 2);

		painter.SetColor(0x000000);
		painter.PutText(*m_Font, wline.c_str(), tx, ty);
	}

	// Draw caret
	size_t caretLine = m_Doc.GetLineNumberAtIndex(m_Caret);
	if(caretLine >= m_FirstVisibleLine && caretLine < m_FirstVisibleLine + (size_t) linesVisible) {
		size_t lineStart = m_Doc.GetLineStartIndex(caretLine);
		size_t column = m_Caret - lineStart;

		std::string lineBytes = m_Doc.GetLine(caretLine);
		std::wstring wline;
		for(char c : lineBytes) wline.push_back((unsigned char) c);

		std::wstring before;
		for(size_t i = 0; i < column && i < wline.size(); ++i) before.push_back(wline[i]);

		float cx = (float) (m_X + m_LeftPadding);
		if(!before.empty()) {
			float bw, bh;
			painter.MeasureText(*m_Font, before.c_str(), bw, bh);
			cx += bw;
		}
		float cy = (float) (m_Y + (int) (caretLine - m_FirstVisibleLine) * m_LineHeight + 2);

		// caret as a thin rectangle
		painter.SetColor(0x000000);
		painter.FillRect(cx, cy, 1.5f, m_LineHeight - 4);
	}
}

void TextEditorWidget::OnKey(const KeyInputEvent &e) {
	if(!e.IsDown) return;

	// Printable characters
	if(e.Char >= 0x20 && e.Char < 0x80) {
		char c = (char) e.Char;
		std::string s(1, c);
		m_Doc.Insert(m_Caret, s);
		m_Caret += s.length();
		return;
	}

	// Control combinations
	if(e.WithCtrl) {
		switch(e.Key) {
			case 'C':
			{
				if(m_SelectionStart != SIZE_MAX && m_SelectionEnd != SIZE_MAX && m_SelectionEnd > m_SelectionStart) {
					size_t len = m_SelectionEnd - m_SelectionStart;
					m_Doc.Copy(m_SelectionStart, len);
				}
				return;
			}
			case 'X':
			{
				if(m_SelectionStart != SIZE_MAX && m_SelectionEnd != SIZE_MAX && m_SelectionEnd > m_SelectionStart) {
					size_t len = m_SelectionEnd - m_SelectionStart;
					m_Doc.Cut(m_SelectionStart, len);
					m_Caret = m_SelectionStart;
					m_SelectionStart = m_SelectionEnd = SIZE_MAX;
				}
				return;
			}
			case 'V':
			{
				m_Doc.Paste(m_Caret);
				// After paste, caret moves to end of pasted text - naive: move to doc end of paste
				// We can't easily get pasted length; for simplicity move caret to end of doc.
				m_Caret = m_Doc.GetCharCount();
				return;
			}
			case 'Z': m_Doc.Undo(); return;
			case 'Y': m_Doc.Redo(); return;
			case 'S':
			{
				// save to default file
				SaveToFile(L"saved.txt");
				return;
			}
		}
	}

	// Navigation & editing
	switch(e.Key) {
		case VK_LEFT:
			if(m_Caret > 0) --m_Caret;
			break;
		case VK_RIGHT:
			if(m_Caret < m_Doc.GetCharCount()) ++m_Caret;
			break;
		case VK_UP:
		{
			size_t line = m_Doc.GetLineNumberAtIndex(m_Caret);
			if(line > 0) {
				size_t col = m_Caret - m_Doc.GetLineStartIndex(line);
				size_t prevStart = m_Doc.GetLineStartIndex(line - 1);
				size_t prevLen = m_Doc.GetLine(line - 1).length();
				size_t newCol = min(col, prevLen);
				m_Caret = prevStart + newCol;
				if(line - 1 < m_FirstVisibleLine) m_FirstVisibleLine = line - 1;
			}
			break;
		}
		case VK_DOWN:
		{
			size_t line = m_Doc.GetLineNumberAtIndex(m_Caret);
			if(line + 1 < m_Doc.GetLineCount()) {
				size_t col = m_Caret - m_Doc.GetLineStartIndex(line);
				size_t nextStart = m_Doc.GetLineStartIndex(line + 1);
				size_t nextLen = m_Doc.GetLine(line + 1).length();
				size_t newCol = min(col, nextLen);
				m_Caret = nextStart + newCol;
				// ensure visible
				size_t linesVisible = (size_t) (m_Height / m_LineHeight);
				if(line + 1 >= m_FirstVisibleLine + linesVisible) {
					m_FirstVisibleLine = line + 1 - (linesVisible - 1);
				}
			}
			break;
		}
		case VK_BACK:
			if(m_Caret > 0) {
				m_Doc.Delete(m_Caret - 1, 1);
				--m_Caret;
			}
			break;
		case VK_DELETE:
			if(m_Caret < m_Doc.GetCharCount()) {
				m_Doc.Delete(m_Caret, 1);
			}
			break;
		case VK_RETURN:
			m_Doc.Insert(m_Caret, std::string("\n"));
			++m_Caret;
			break;
	}
}

void TextEditorWidget::OnMouseMove(const MouseMoveEvent &e) {
	// nothing for now
	(void) e;
}

void TextEditorWidget::OnMouseClick(int x, int y) {
	// Map click to line/column
	if(x < m_X || x > m_X + (int) m_Width) return;
	if(y < m_Y || y > m_Y + (int) m_Height) return;

	int localY = y - m_Y;
	int lineOffset = localY / (int) m_LineHeight;
	size_t lineIndex = m_FirstVisibleLine + lineOffset;
	if(lineIndex >= m_Doc.GetLineCount()) {
		m_Caret = m_Doc.GetCharCount();
		return;
	}

	std::string lineBytes = m_Doc.GetLine(lineIndex);
	std::wstring wline;
	for(char c : lineBytes) wline.push_back((unsigned char) c);

	float px = (float) (m_X + m_LeftPadding);
	float clickX = (float) x;

	// iterate characters to find nearest column
	size_t col = 0;
	float acc = px;
	for(size_t i = 0; i < wline.size(); ++i) {
		wchar_t ch[2] = { wline[i], 0 };
		float cw, chh;
		// temporary painter required - we cannot access painter here; approximate with font size
		// Fallback: assume monospace width by measuring single character with a temporary Painter is heavy.
		// Use simple per-character width estimate via MeasureText - need a painter, so map to center of widget on click
		// Instead approximate width as 8 pixels per character for ASCII (simple heuristic)
		float charWidth = 8.0f;
		acc += charWidth;
		if(acc >= clickX) { col = i; break; }
		col = i + 1;
	}

	m_Caret = m_Doc.GetLineStartIndex(lineIndex) + col;
}

bool TextEditorWidget::LoadFromFile(const std::wstring &path) {
	std::ifstream ifs(std::string(path.begin(), path.end()), std::ios::binary);
	if(!ifs) return false;
	std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	//m_Doc = PieceTable(content);
	m_Caret = 0;
	m_FirstVisibleLine = 0;
	return true;
}

bool TextEditorWidget::SaveToFile(const std::wstring &path) {
	std::ofstream ofs(std::string(path.begin(), path.end()), std::ios::binary);
	if(!ofs) return false;
	std::string content = m_Doc.GetText();
	ofs.write(content.c_str(), (std::streamsize) content.size());
	return true;
}