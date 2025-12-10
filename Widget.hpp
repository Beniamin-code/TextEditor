#pragma once

#include "Painter.hpp"

//m_Root = &Make<Column>()
//	.Add(
//		Make<SizeBox>(
//			Make<Row>()
//				.Add(
//					Make<Button>(L"Files")
//				).Add(
//					Make<Button>(L"Edit")
//				).Gap(4)
//		).Height(48)
//	).Add(
//		Make<Expand>(
//			Make<TextBox>()
//		)
//	);

//m_Root = &Make<Button>(L"Click me!");

struct Constraint {
	unsigned MinWidth = 0, MinHeight = 0;
	unsigned MaxWidth = -1, MaxHeight = -1;
	unsigned Width = 0, Height = 0;

	Constraint &Preferred(int x, int y);
	Constraint &Min(int x, int y);
	Constraint &Max(int x, int y);
};

struct Context {

};

struct Widget {
	virtual ~Widget() { }

	virtual void Resize(const Constraint &parentCon);
	virtual void Paint(const Painter &painter) = 0;

	void Position(int x, int y);

protected:
	unsigned m_X = 0, m_Y = 0;
	unsigned m_Width = 0, m_Height = 0;
	Constraint m_Constraint;
};

struct Child {
	Widget *Widget;
	Child *Next;
};

struct ChildrenIterator {
	Child *Current;

	Widget &operator*() { return *Current->Widget; }
	ChildrenIterator &operator++() { Current = Current->Next; return *this; }
	bool operator!=(const ChildrenIterator &ci) { return Current != ci.Current; }
};

struct ChildrenList {
	Child *First, *Last;
	ChildrenIterator begin() { return { First }; }
	ChildrenIterator end() { return { 0 }; }
};

struct Container : Widget {
	Container &Gap(int gap);
	~Container();
	Container &Add(Widget &w);
	ChildrenList &Children();

	void Paint(const Painter &painter) override;

protected:
	ChildrenList m_Children { 0, 0 };
	int m_Gap = 0;
};

struct SingleChildWidget : Widget {
	SingleChildWidget(Widget &child);

	void Paint(const Painter &painter) override;

protected:
	Widget *m_Child;
};

struct SizeBox : SingleChildWidget {
	using SingleChildWidget::SingleChildWidget;

	SizeBox &Width(int width);
	SizeBox &Height(int height);

protected:
	int m_SizeWidth = -1, m_SizeHeight = -1;
};

struct Expand : SingleChildWidget {
	using SingleChildWidget::SingleChildWidget;
};

struct Column : Container {
	void Resize(const Constraint &parentCon) override;
};

struct Row : Container {

};

struct Button : Widget {
	Button(const wchar_t *text);

	void Paint(const Painter &painter) override;

protected:
	const wchar_t *m_Text;
};

struct TextBox : Widget {
	TextBox();

	void Paint(const Painter &painter) override;
};