#pragma once
#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint16_t

class TrameHelper
{
public:
	// ----------------- Helpers parsing: in-place, zero allocation -----------------

	static bool matchAt(const char* p, const char* end, const char* lit);
	static bool findLastTag(const char* data, size_t len, const char* open, const char* close, const char*& valBegin, std::size_t& valLen);
	static bool findOpenTagWindow(const char* data, std::size_t len, const char* tagOpen, const char*& winBegin, const char*& winEnd);
	static bool parseU16FromSpan(const char* b, const char* e, std::uint16_t& out);
	static bool parseDoubleFromSpan(const char* b, const char* e, double& out);
	static bool extractAttrDoubles(const char* begin, const char* end, const char* const* keys, double* out, const int size=6);
};
