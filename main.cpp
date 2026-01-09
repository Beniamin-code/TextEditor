#include "Window.hpp"

struct TextEditorWindow : Window {
	TextEditorWindow(WindowSettings windowSettings) : Window(
		L"Text Editor",
		windowSettings
			.Background(0x181818)
			.Size(640, 480)
	) {
		m_Font = new Font(L"Segoe UI", 18, 600);

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
	}

	~TextEditorWindow() {
		delete m_Font;
	}

	void Handle(const ResizeEvent &e) override {
		m_Width = (float) e.Width;
		m_Height = (float) e.Height;
	}

	void Handle(const PaintEvent &e) override {
		e.Painter.SetColor(0x202020);
		e.Painter.FillRect(10, 34, m_Width - 20, m_Height - 44, 8);

		wchar_t text[] = L"You pressed ' ' 🐒";
		text[13] = m_Char;

		e.Painter.SetColor(0xE0E0E0);
		e.Painter.PutText(*m_Font, text, 20, 44);

		e.Painter.SetColor(0xFF9900);
		e.Painter.FillRect(m_X - 10, m_Y - 10, 10, 10);
	}

	void Handle(const KeyInputEvent &e) override {
		if(!e.IsDown) return;
		if(e.Char < 0x20 || e.Char >= 0x80)
			return;

		m_Char = e.Char;
		Redraw();
	}

	void Handle(const MouseMoveEvent &e) override {
		m_X = (float) e.X;
		m_Y = (float) e.Y;
		Redraw();
	}

private:
	float m_Width = 0, m_Height = 0;
	float m_X = 0, m_Y = 0;
	wchar_t m_Char = ' ';

	Font *m_Font = 0;
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