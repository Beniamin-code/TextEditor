#include "Widget.hpp"

Constraint &Constraint::Preferred(int x, int y) {
	Width = x;
	Height = y;
	return *this;
}

Constraint &Constraint::Min(int x, int y) {
	MinWidth = x;
	MinHeight = y;
	return *this;
}

Constraint &Constraint::Max(int x, int y) {
	MaxWidth = x;
	MaxHeight = y;
	return *this;
}

void Widget::Resize(const Constraint &parentCon) {
	m_Width = m_Constraint.Width;
	m_Height = m_Constraint.Height;

	if(m_Width < parentCon.MinWidth) m_Width = parentCon.MinWidth;
	else if(m_Width > parentCon.MaxWidth) m_Width = parentCon.MaxWidth;

	if(m_Height < parentCon.MinHeight) m_Height = parentCon.MinHeight;
	else if(m_Height > parentCon.MaxHeight) m_Height = parentCon.MaxHeight;
	
	if(m_Width == -1) throw L"Infinite Width";
	if(m_Height == -1) throw L"Infinite Height";
}

void Widget::Position(int x, int y) {
	m_X = x;
	m_Y = y;
}

Container::~Container() {
	// Nodes should be deleted
}

Container &Container::Gap(int gap) {
	m_Gap = gap;
	return *this;
}

Container &Container::Add(Widget &w) {
	Child *child = new Child { &w, 0 };

	if(m_Children.First)
		m_Children.Last->Next = child;
	else
		m_Children.First = child;

	m_Children.Last = child;
	return *this;
};

void Container::Paint(const Painter &painter) {
	for(auto &w : m_Children)
		w.Paint(painter);
}

ChildrenList &Container::Children() {
	return m_Children;
}

Button::Button(const wchar_t *text) : m_Text(text) {
	m_Constraint.Preferred(120, 60);
}

void Button::Paint(const Painter &painter) {
	painter.SetColor(0xFF9900);
	painter.FillRect(m_X, m_Y, m_Width, m_Height);
}

SingleChildWidget::SingleChildWidget(Widget &child) {
	m_Child = &child;
}

void SingleChildWidget::Paint(const Painter &painter) {
	m_Child->Paint(painter);
}

SizeBox &SizeBox::Width(int width) {
	m_SizeWidth = width;
	return *this;
}

SizeBox &SizeBox::Height(int height) {
	m_SizeHeight = height;
	return *this;
}

TextBox::TextBox() {

}

void TextBox::Paint(const Painter &painter) {
	painter.SetColor(0x0099FF);
	painter.FillRect(m_X, m_Y, m_Width, m_Height);
}

void Column::Resize(const Constraint &parentCon) {
	Widget::Resize(parentCon);
	
	int y = m_Y;
	
	Constraint cons = parentCon;
	cons.MinWidth = cons.MaxWidth = m_Width;
	
	for(Widget &w : m_Children) {
		w.Position(m_X, y);

		int consumedHeight = m_Constraint.Height - (y - m_Y);

		y += m_Gap;
	}
}