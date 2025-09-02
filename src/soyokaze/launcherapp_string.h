#pragma once

#include <string>
#include <cctype>
#include <algorithm>

class String : public std::string
{
public:
	using std::string::string;

	String(const std::string& s) : std::string(s)
	{
	}

	String(const wchar_t* p) : std::string()
	{
		int requiredLen = WideCharToMultiByte(CP_UTF8, 0, p, -1, nullptr, 0, 0, 0);
		if (requiredLen <= 1) {
			return;
		}

		resize(requiredLen-1);
		WideCharToMultiByte(CP_UTF8, 0, p, -1, &(*begin()), requiredLen-1, 0, 0);
	}

	size_t GetLength() const {
		return size();
	}
	
	void MakeLower() {
		std::transform(begin(), end(), begin(), [](unsigned char c) { return std::tolower(c); });
	}

	void TrimLeft() {
		auto it = begin();
    while (it != end() && std::isspace(static_cast<unsigned char>(*it))) {
        ++it;
    }
		erase(begin(), it);
	}

	int CompareNoCase(const std::string& b) const {
		size_t len = (std::min)(size(), b.size());
		for (size_t i = 0; i < len; ++i) {
			auto ca = std::tolower(static_cast<unsigned char>(at(i)));
			auto cb = std::tolower(static_cast<unsigned char>(b.at(i)));
			if (ca != cb) {
				return ca - cb;
			}
		}
		return static_cast<int>(size()) - static_cast<int>(b.size());
	}

	std::string Left(int n) const
	{
		return substr(0, n);
	}

	std::string Mid(int s, int n) const
	{
		if (size() <= s) {
			return "";
		}
		return substr(s, n);
	}

	std::string Mid(int s) const
	{
		if (size() <= s) {
			return "";
		}
		return substr(s);
	}


	int Find(const char* p, size_t n = 0) const
	{
    auto pos = find(p, n);
    return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
	}

	int Find(const std::string& str, size_t n = 0) const
	{
    auto pos = find(str, n);
    return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
	}

	int Find(char c, size_t n = 0) const
	{
		auto pos = find(c, n);
		return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
	}

	bool IsEmpty() const
	{
		return empty();
	}

	void Replace(char from, char to) {
		for (auto& c : *this) {
			if (c == from) {
				c = to;
			}
		}
	}

};

