#pragma once
#include <string>

namespace Platform {
	void SetClipboardText(const std::string &Text);
	std::string GetClipboardText();
}