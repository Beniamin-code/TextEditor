#include "Window.hpp"
#include "TextEditorWidget.hpp"

struct TextEditorWindow : Window {
	TextEditorWindow(WindowSettings windowSettings) : Window(
		L"Text Editor",
		windowSettings
		.Background(0x181818)
		.Size(800, 600)
	) {
		m_Font = new Font(L"Segoe UI", 18, 400);

		m_Editor = new TextEditorWidget(m_Font);
		// simple layout: editor covers whole client area minus menubar (drawn in paint)
		m_Editor->Position(0, 24);
		m_Editor->m_Constraint.Min(100, 100);
		m_Editor->m_Constraint.Width = 800;
		m_Editor->m_Constraint.Height = 576;

		// try load sample file if exists
		m_Editor->LoadFromFile(L"sample.txt");
	}

	~TextEditorWindow() {
		delete m_Editor;
		delete m_Font;
	}

	void Handle(const ResizeEvent &e) override {
		m_Width = (float) e.Width;
		m_Height = (float) e.Height;

		// resize editor to fit below menubar
		int menubarH = 24;
		m_Editor->Position(0, menubarH);
		m_Editor->m_Constraint.Width = e.Width;
		m_Editor->m_Constraint.Height = e.Height - menubarH;
		m_Editor->Resize(m_Editor->m_Constraint);
	}

	void Handle(const PaintEvent &e) override {
		// menubar
		e.Painter.SetColor(0x2B2B2B);
		e.Painter.FillRect(0, 0, m_Width, 24);

		e.Painter.SetColor(0xE0E0E0);
		e.Painter.PutText(*m_Font, L"File", 8, 4);
		e.Painter.PutText(*m_Font, L"Edit", 64, 4);

		// editor paint
		m_Editor->Paint(e.Painter);
	}

	void Handle(const KeyInputEvent &e) override {
		m_Editor->OnKey(e);
		Redraw();
	}

	void Handle(const MouseMoveEvent &e) override {
		m_Editor->OnMouseMove(e);
	}

	void Handle(const MouseClickEvent &e) override {
		m_Editor->OnMouseClick(e.X, e.Y);
		Redraw();
	}

private:
	float m_Width = 0, m_Height = 0;

	Font *m_Font = 0;
	TextEditorWidget *m_Editor = 0;
};

int main() {
	try {
		WindowSettings windowSettings;
		TextEditorWindow window(windowSettings);

		while(window.IsOpen())
			Window::HandleEvents();

	} catch(const wchar_t *message) {
		Window::OpenDialog(L"Error", message, DialogType::Error);
	}
}