#pragma once

#include <utility>

template <typename T, typename... Args>
T &Make(Args&&... args) {
	return *new T(std::forward<Args>(args)...);
}
