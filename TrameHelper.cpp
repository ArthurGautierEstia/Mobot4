#include "TrameHelper.h"
#include <charconv>  // std::from_chars

bool TrameHelper::matchAt(const char* p, const char* end, const char* lit)
{
	while (*lit)
	{
		if (p >= end || *p != *lit)
			return false;
		++p;
		++lit;
	}
	return true;
}

bool TrameHelper::findLastTag(const char* data, std::size_t len, const char* open, const char* close, const char*& valBegin, std::size_t& valLen)
{
	const char* begin = data;
	const char* end = data + len;

	const std::size_t closeLen = std::strlen(close);
	for (const char* p = end - 1; p >= begin + static_cast<ptrdiff_t>(closeLen - 1); --p)
	{
		if (*(p - (closeLen - 1)) == '<' && matchAt(p - (closeLen - 1), end, close))
		{
			const char* closePos = p - (closeLen - 1);
			const std::size_t openLen = std::strlen(open);
			for (const char* q = closePos; q >= begin + static_cast<ptrdiff_t>(openLen); --q)
			{
				if (*(q - openLen) == '<' && matchAt(q - openLen, end, open))
				{
					const char* openPos = q - openLen;
					valBegin = openPos + openLen;
					valLen = static_cast<std::size_t>(closePos - (openPos + openLen));
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

bool TrameHelper::findOpenTagWindow(const char* data, std::size_t len, const char* tagOpen, const char*& winBegin, const char*& winEnd)
{
	const char* begin = data;
	const char* end = data + len;
	const std::size_t tagLen = std::strlen(tagOpen);

	for (const char* p = begin; p + tagLen < end; ++p)
	{
		if (*p == '<' && matchAt(p, end, tagOpen))
		{
			winBegin = p;
			for (const char* q = p; q + 1 < end; ++q)
			{
				if (*q == '/' && *(q + 1) == '>')
				{
					winEnd = q + 2;
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

bool TrameHelper::parseU16FromSpan(const char* b, const char* e, std::uint16_t& out)
{
	if (b >= e)
		return false;
	unsigned int acc = 0;
	const char* p = b;
	while (p < e && *p >= '0' && *p <= '9')
	{
		acc = acc * 10u + static_cast<unsigned int>(*p - '0');
		++p;
		if (acc > 65535u)
			return false;
	}
	if (p == b)
		return false; // aucun digit
	out = static_cast<std::uint16_t>(acc);
	return true;
}

bool TrameHelper::parseDoubleFromSpan(const char* b, const char* e, double& out)
{
	std::from_chars_result res = std::from_chars(b, e, out, std::chars_format::general);
	return res.ec == std::errc{}; // conversion réussie
}

bool TrameHelper::extractAttrDoubles(const char* begin, const char* end, const char* const* keys, double* out, const int size)
{
	for (int i = 0; i < size; ++i)
		out[i] = 0.0;

	for (int i = 0; i < size; ++i)
	{
		const char* k = keys[i];
		const std::size_t klen = std::strlen(k);
		for (const char* p = begin; p + klen + 2 < end; ++p) {
			if (matchAt(p, end, k) &&
				(p + klen) < end &&
				*(p + klen) == '=' &&
				(p + klen + 1) < end &&
				*(p + klen + 1) == '"')
			{
				const char* vbeg = p + klen + 2;
				const char* v = vbeg;
				while (v < end && *v != '"')
					++v;
				if (v <= end)
				{
					double dv;
					if (parseDoubleFromSpan(vbeg, v, dv))
						out[i] = dv;
				}
				break; // clé suivante
			}
		}
	}
	return true;
}
