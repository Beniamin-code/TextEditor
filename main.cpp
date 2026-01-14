#include "Window.hpp"
#include "TextEditorWidget.hpp"

struct TextEditorWindow : Window {
	TextEditorWindow(WindowSettings windowSettings) : Window(
		L"Text Editor",
		windowSettings
		.Background(0x202020)
		.Size(800, 600)
	) {
		m_Font = new Font(L"Segoe UI", 18, 400);

		m_Editor = new TextEditorWidget(m_Font);
		m_Editor->Position(0, 0);
		m_Editor->m_Constraint.Min(100, 100);
		m_Editor->m_Constraint.Width = 800;
		m_Editor->m_Constraint.Height = 576;
	}

	~TextEditorWindow() {
		delete m_Editor;
		delete m_Font;
	}

	void Handle(const ResizeEvent &e) override {
		m_Width = (float) e.Width;
		m_Height = (float) e.Height;

		m_Editor->Position(0, 0);
		m_Editor->m_Constraint.Width = e.Width;
		m_Editor->m_Constraint.Height = e.Height;
		m_Editor->Resize(m_Editor->m_Constraint);
	}

	void Handle(const PaintEvent &e) override {
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