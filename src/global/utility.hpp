/**/
#pragma once

constexpr size_t getIndxByValue(const auto& arr, const auto& T) {
	auto it = std::find(arr.cbegin(), arr.cend(), T);
	return it - arr.cbegin();
}
