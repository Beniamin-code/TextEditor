#include "TextEditorWidget.hpp"

#include <fstream>
#include <string>

static std::wstring ToWString(const std::string &s) {
	std::wstring ws;
	ws.reserve(s.size());
	for(unsigned char c : s) ws.push_back((wchar_t) c);
	return ws;
}

static void NormalizeSelection(size_t &a, size_t &b, size_t &outStart, size_t &outEnd) {
	if(a == SIZE_MAX || b == SIZE_MAX) { outStart = outEnd = SIZE_MAX; return; }
	if(a <= b) { outStart = a; outEnd = b; } else { outStart = b; outEnd = a; }
}

TextEditorWidget::TextEditorWidget(Font *font)
	: m_Font(font) {
	m_Constraint.Min(100, 100);
	m_Constraint.Max(-1, -1);
	m_Constraint.Width = 400;
	m_Constraint.Height = 300;
}

void TextEditorWidget::Resize(const Constraint &parentCon) {
	Widget::Resize(parentCon);
}

void TextEditorWidget::Paint(const Painter &painter) {
	float w, h;
	painter.MeasureText(*m_Font, L"Mg", w, h);
	if(h > 1.0f) m_LineHeight = h;

	int linesVisible = (int) ((float) m_Height / m_LineHeight);
	size_t totalLines = m_Doc.GetLineCount();

	size_t selStart, selEnd;
	NormalizeSelection(m_SelectionStart, m_SelectionEnd, selStart, selEnd);

	for(int i = 0; i < linesVisible; ++i) {
		size_t lineIndex = m_FirstVisibleLine + i;
		if(lineIndex >= totalLines) break;

		std::string lineBytes = m_Doc.GetLine(lineIndex);
		std::wstring wline = ToWString(lineBytes);

		float tx = (float) (m_X + m_LeftPadding);
		float ty = (float) (m_Y + i * m_LineHeight + 2);

		if(selStart != SIZE_MAX && selEnd > selStart) {
			size_t lineStart = m_Doc.GetLineStartIndex(lineIndex);
			size_t lineLen = lineBytes.length();
			size_t lineEnd = lineStart + lineLen;

			size_t overlapStart = max(lineStart, selStart);
			size_t overlapEnd = min(lineEnd, selEnd);

			if(overlapStart < overlapEnd) {
				size_t beforeCount = overlapStart - lineStart;
				size_t selCount = overlapEnd - overlapStart;

				std::wstring beforeW;
				if(beforeCount > 0) beforeW.assign(wline.begin(), wline.begin() + (std::ptrdiff_t) beforeCount);

				std::wstring selW;
				if(selCount > 0) selW.assign(wline.begin() + (std::ptrdiff_t) beforeCount,
					wline.begin() + (std::ptrdiff_t) (beforeCount + selCount));

				float bx = tx;
				if(!beforeW.empty()) {
					float bw, bh;
					painter.MeasureText(*m_Font, beforeW.c_str(), bw, bh);
					bx += bw;
				}

				float sw = 0;
				if(!selW.empty()) {
					float tw, th;
					painter.MeasureText(*m_Font, selW.c_str(), tw, th);
					sw = tw;
				}

				painter.SetColor(0x1248A0);
				painter.FillRect(bx, ty, sw, m_LineHeight - 4);
			}
		}

		painter.SetColor(0xE0E0E0);
		painter.PutText(*m_Font, wline.c_str(), tx, ty);
	}

	size_t selS, selE;
	NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
	if(!(selS != SIZE_MAX && selE > selS)) {
		size_t caretLine = m_Doc.GetLineNumberAtIndex(m_Caret);
		if(caretLine >= m_FirstVisibleLine && caretLine < m_FirstVisibleLine + (size_t) linesVisible) {
			size_t lineStart = m_Doc.GetLineStartIndex(caretLine);
			size_t column = m_Caret - lineStart;

			std::string lineBytes = m_Doc.GetLine(caretLine);
			std::wstring wline = ToWString(lineBytes);

			std::wstring before;
			for(size_t i = 0; i < column && i < wline.size(); ++i) before.push_back(wline[i]);

			float cx = (float) (m_X + m_LeftPadding);
			if(!before.empty()) {
				float bw, bh;
				painter.MeasureText(*m_Font, before.c_str(), bw, bh);
				cx += bw;
			}
			float cy = (float) (m_Y + (int) (caretLine - m_FirstVisibleLine) * m_LineHeight + 2);

			painter.SetColor(0xE0E0E0);
			painter.FillRect(cx, cy, 1.5f, m_LineHeight - 4);
		}
	}
}

void TextEditorWidget::OnKey(const KeyInputEvent &e) {
	if(!e.IsDown) return;

	if(e.Char >= 0x20 && e.Char < 0x80) {
		char c = (char) e.Char;
		std::string s(1, c);

		size_t selS, selE;
		NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
		if(selS != SIZE_MAX && selE > selS) {
			m_Doc.Delete(selS, selE - selS);
			m_Caret = selS;
			m_SelectionStart = m_SelectionEnd = SIZE_MAX;
		}

		m_Doc.Insert(m_Caret, s);
		m_Caret += s.length();
		return;
	}

	if(e.WithCtrl) {
		switch(e.Key) {
			case 'C':
			{
				size_t selS, selE;
				NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
				if(selS != SIZE_MAX && selE > selS) {
					size_t len = selE - selS;
					m_Doc.Copy(selS, len);
				}
				return;
			}
			case 'X':
			{
				size_t selS, selE;
				NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
				if(selS != SIZE_MAX && selE > selS) {
					size_t len = selE - selS;
					m_Doc.Cut(selS, len);
					m_Caret = selS;
					m_SelectionStart = m_SelectionEnd = SIZE_MAX;
				}
				return;
			}
			case 'V':
			{
				size_t selS, selE;
				NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
				if(selS != SIZE_MAX && selE > selS) {
					m_Doc.Delete(selS, selE - selS);
					m_Caret = selS;
					m_SelectionStart = m_SelectionEnd = SIZE_MAX;
				}
				m_Doc.Paste(m_Caret);
				m_Caret = m_Doc.GetCharCount();
				return;
			}
			case 'Z': m_Doc.Undo(); return;
			case 'Y': m_Doc.Redo(); return;
			case 'S':
			{
				SaveToFile(L"saved.txt");
				return;
			}
		}
	}

	auto StartSelectionIfNeeded = [&](const size_t oldCaret) {
		if(e.WithShift) {
			if(m_SelectionStart == SIZE_MAX || m_SelectionEnd == SIZE_MAX) {
				m_SelectionStart = oldCaret;
			}
			m_SelectionEnd = m_Caret;
		} else {
			m_SelectionStart = m_SelectionEnd = SIZE_MAX;
		}
	};

	switch(e.Key) {
		case VK_LEFT:
		{
			size_t old = m_Caret;
			if(m_Caret > 0) --m_Caret;
			StartSelectionIfNeeded(old);
			break;
		}
		case VK_RIGHT:
		{
			size_t old = m_Caret;
			if(m_Caret < m_Doc.GetCharCount()) ++m_Caret;
			StartSelectionIfNeeded(old);
			break;
		}
		case VK_UP:
		{
			size_t old = m_Caret;
			size_t line = m_Doc.GetLineNumberAtIndex(m_Caret);
			if(line > 0) {
				size_t col = m_Caret - m_Doc.GetLineStartIndex(line);
				size_t prevStart = m_Doc.GetLineStartIndex(line - 1);
				size_t prevLen = m_Doc.GetLine(line - 1).length();
				size_t newCol = min(col, prevLen);
				m_Caret = prevStart + newCol;
				if(line - 1 < m_FirstVisibleLine) m_FirstVisibleLine = line - 1;
			}
			StartSelectionIfNeeded(old);
			break;
		}
		case VK_DOWN:
		{
			size_t old = m_Caret;
			size_t line = m_Doc.GetLineNumberAtIndex(m_Caret);
			if(line + 1 < m_Doc.GetLineCount()) {
				size_t col = m_Caret - m_Doc.GetLineStartIndex(line);
				size_t nextStart = m_Doc.GetLineStartIndex(line + 1);
				size_t nextLen = m_Doc.GetLine(line + 1).length();
				size_t newCol = min(col, nextLen);
				m_Caret = nextStart + newCol;

				size_t linesVisible = (size_t) (m_Height / m_LineHeight);
				if(line + 1 >= m_FirstVisibleLine + linesVisible) {
					m_FirstVisibleLine = line + 1 - (linesVisible - 1);
				}
			}
			StartSelectionIfNeeded(old);
			break;
		}
		case VK_BACK:
		{
			size_t selS, selE;
			NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
			if(selS != SIZE_MAX && selE > selS) {
				m_Doc.Delete(selS, selE - selS);
				m_Caret = selS;
				m_SelectionStart = m_SelectionEnd = SIZE_MAX;
			} else if(m_Caret > 0) {
				m_Doc.Delete(m_Caret - 1, 1);
				--m_Caret;
			}
			break;
		}
		case VK_DELETE:
		{
			size_t selS, selE;
			NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
			if(selS != SIZE_MAX && selE > selS) {
				m_Doc.Delete(selS, selE - selS);
				m_Caret = selS;
				m_SelectionStart = m_SelectionEnd = SIZE_MAX;
			} else if(m_Caret < m_Doc.GetCharCount()) {
				m_Doc.Delete(m_Caret, 1);
			}
			break;
		}
		case VK_RETURN:
		{
			size_t selS, selE;
			NormalizeSelection(m_SelectionStart, m_SelectionEnd, selS, selE);
			if(selS != SIZE_MAX && selE > selS) {
				m_Doc.Delete(selS, selE - selS);
				m_Caret = selS;
				m_SelectionStart = m_SelectionEnd = SIZE_MAX;
			}
			m_Doc.Insert(m_Caret, std::string("\n"));
			++m_Caret;
			break;
		}
	}
}

void TextEditorWidget::OnMouseMove(const MouseMoveEvent &e) {
	
}

void TextEditorWidget::OnMouseClick(int x, int y) {
	if(x < m_X || x > m_X + (int) m_Width) return;
	if(y < m_Y || y > m_Y + (int) m_Height) return;

	int localY = y - m_Y;
	int lineOffset = localY / (int) m_LineHeight;
	size_t lineIndex = m_FirstVisibleLine + lineOffset;
	if(lineIndex >= m_Doc.GetLineCount()) {
		m_Caret = m_Doc.GetCharCount();
		m_SelectionStart = m_SelectionEnd = SIZE_MAX;
		return;
	}

	std::string lineBytes = m_Doc.GetLine(lineIndex);
	std::wstring wline = ToWString(lineBytes);

	float px = (float) (m_X + m_LeftPadding);
	float clickX = (float) x;

	size_t col = 0;
	float acc = px;
	const float charWidth = 8.0f;
	for(size_t i = 0; i < wline.size(); ++i) {
		acc += charWidth;
		if(acc >= clickX) { col = i; break; }
		col = i + 1;
	}

	m_Caret = m_Doc.GetLineStartIndex(lineIndex) + col;

	m_SelectionStart = m_SelectionEnd = SIZE_MAX;
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