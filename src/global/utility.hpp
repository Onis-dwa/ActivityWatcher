/**/
#pragma once

constexpr size_t nameFromFullName(std::string& orig, std::string& outName) {
	size_t pos = orig.rfind(std::filesystem::path::preferred_separator);
	outName = move(orig.substr(pos + 1));
	return pos;
}

constexpr size_t getIndxByValue(const auto& arr, const auto& T) {
	auto it = std::find(arr.cbegin(), arr.cend(), T);
	return it - arr.cbegin();
}
