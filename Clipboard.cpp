#include "Clipboard.hpp"
#define NOMINMAX
#include <windows.h>
#include <cstring>

namespace Platform {
	void SetClipboardText(const std::string &Text) {
		if(!OpenClipboard(nullptr)) return;

		EmptyClipboard();

		HGLOBAL Hg = GlobalAlloc(GMEM_MOVEABLE, Text.size() + 1);
		if(!Hg) {
			CloseClipboard();
			return;
		}

		void *RawMemory = GlobalLock(Hg);

		memcpy(RawMemory, Text.c_str(), Text.size() + 1);

		GlobalUnlock(Hg);

		SetClipboardData(CF_TEXT, Hg);

		CloseClipboard();
	}

	std::string GetClipboardText() {
		if(!OpenClipboard(nullptr)) return "";

		HANDLE HData = GetClipboardData(CF_TEXT);

		if(HData == nullptr) {
			CloseClipboard();
			return "";
		}

		char *PTextFromClipboard = static_cast<char *>(GlobalLock(HData));

		if(PTextFromClipboard == nullptr) {
			CloseClipboard();
			return "";
		}

		std::string TextFromClipboard(PTextFromClipboard);

		GlobalUnlock(HData);
		CloseClipboard();
		
		return TextFromClipboard;
	}
}