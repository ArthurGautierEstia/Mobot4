#include "RsiTrame.h"
#include "global_macros.h"
#include <cstring>	  // std::memcpy
#include <sstream>
#include <iomanip>
#include <cstdio>     // snprintf (fallback)

RsiTrame::RsiTrame() :
	m_flagExitCorr(false),
	m_isCartesian(true),
	m_isInRobotBase(true),
	m_blocContinue(false),
	m_blocCancel(false),
	m_digout(0),
	m_krl(0),
	m_mode(0),
	m_ipoc(),
	m_delta()
{
	std::memset(m_delta, 0, ARRAY_6_FLOAT_SIZE);
	m_ipoc.reserve(32);
	reset();
}

inline void RsiTrame::reset()
{
	m_flagExitCorr = false;
	m_isCartesian = true;
	m_blocContinue = false;
	m_blocCancel = false;
	m_digout = 0;
	m_krl = 0;
	m_mode = 0; // None
	m_ipoc.clear();
	std::memset(m_delta, 0, ARRAY_6_FLOAT_SIZE);
	m_additionnalXmlNodes.clear();
}

void RsiTrame::setPose(bool isCartesian, float pos[6], bool isInRobotBase)
{
	m_isCartesian = isCartesian;
	m_isInRobotBase = isInRobotBase;
	std::memcpy(m_delta, pos, ARRAY_6_FLOAT_SIZE);
}

void RsiTrame::transmitAdditionnalXmlNodes(std::vector<XmlNode>& xmlNodes)
{
	// swap O(1) : on prend possession des données pour traitement
	m_additionnalXmlNodes.swap(xmlNodes);
}

inline void RsiTrame::encodeBoolArrayToUShort(bool io[16], std::uint16_t& nIO)
{
	nIO = 0;
	for (int i = 0; i < 16; ++i) 
	{
		if (io[i]) 
			nIO |= (1u << i);
	}
}

inline bool RsiTrame::isValid() const
{
	return !m_ipoc.empty();
}

inline std::string RsiTrame::build() const
{
	std::ostringstream oss;
	oss << std::setprecision(6) << std::noshowpoint;

	oss << "<Sen Type=\"ImFree\">";
	oss << "<EStr></EStr>";
	oss << "<MoveType>" << (m_isCartesian ? "1" : "0") << "</MoveType>";      // 1 : CARTESIAN | 0 : ARTICULAR
	oss << "<RobotBase>" << (m_isInRobotBase ? "1" : "0") << "</RobotBase>";  // 1 : BASE | 0 : TOOL

	oss << "<DeltaPos"
		<< " X=\"" << std::setprecision(6) << m_delta[0] << "\""
		<< " Y=\"" << std::setprecision(6) << m_delta[1] << "\""
		<< " Z=\"" << std::setprecision(6) << m_delta[2] << "\""
		<< " A=\"" << std::setprecision(6) << m_delta[3] << "\""
		<< " B=\"" << std::setprecision(6) << m_delta[4] << "\""
		<< " C=\"" << std::setprecision(6) << m_delta[5] << "\""
		<< "/>";

	oss << "<Digout>" << m_digout << "</Digout>";
	oss << "<Stop>" << (m_flagExitCorr ? "1" : "0") << "</Stop>";
	oss << "<Krl>" << m_krl << "</Krl>";
	oss << "<Mode>" << m_mode << "</Mode>";
	oss << "<Bloc_Continue>" << (m_blocContinue ? "1" : "0") << "</Bloc_Continue>";
	oss << "<Bloc_Cancel>" << (m_blocCancel ? "1" : "0") << "</Bloc_Cancel>";

	oss << "<IPOC>" << m_ipoc << "</IPOC>";

	// Additionnal XML nodes
	for (const auto& xmlNode : m_additionnalXmlNodes)
	{
		xmlNode.toOSS(oss, 0, true);
	}
	m_additionnalXmlNodes.clear();

	oss << "</Sen>";

	return oss.str();
}

inline bool RsiTrame::build(char* dst, std::size_t cap, std::size_t& outLen) const
{
	char* p = dst;
	char* end = dst + cap;
	outLen = 0;

	// Début
	if (!appStr(p, end, "<Sen Type=\"ImFree\">")) return false;
	if (!appStr(p, end, "<EStr></EStr>")) return false;

	// MoveType 1/0
	if (!appStr(p, end, "<MoveType>")) return false;
	if (!appChr(p, end, m_isCartesian ? '1' : '0')) return false;
	if (!appStr(p, end, "</MoveType>")) return false;

	// RobotBase 1/0
	if (!appStr(p, end, "<RobotBase>")) return false;
	if (!appChr(p, end, m_isInRobotBase ? '1' : '0')) return false;
	if (!appStr(p, end, "</RobotBase>")) return false;

	// DeltaPos
	if (!appStr(p, end, "<DeltaPos")) return false;

	static constexpr const char* XYZABC[6] = { " X=\""," Y=\""," Z=\""," A=\""," B=\""," C=\"" };
	for (int i = 0; i < 6; ++i) {
		if (!appStr(p, end, XYZABC[i])) return false;
		if (!appFloatFixed(p, end, m_delta[i], 6)) return false; // précision 6
		if (!appChr(p, end, '"')) return false;
	}
	if (!appStr(p, end, "/>")) return false;

	// Digout
	if (!appStr(p, end, "<Digout>")) return false;
	if (!appUInt(p, end, static_cast<unsigned int>(m_digout))) return false;
	if (!appStr(p, end, "</Digout>")) return false;

	// Stop
	if (!appStr(p, end, "<Stop>")) return false;
	if (!appChr(p, end, m_flagExitCorr ? '1' : '0')) return false;
	if (!appStr(p, end, "</Stop>")) return false;

	// Krl
	if (!appStr(p, end, "<Krl>")) return false;
	if (!appUInt(p, end, static_cast<unsigned int>(m_krl))) return false;
	if (!appStr(p, end, "</Krl>")) return false;

	// Mode
	if (!appStr(p, end, "<Mode>")) return false;
	if (!appUInt(p, end, static_cast<unsigned int>(m_mode))) return false;
	if (!appStr(p, end, "</Mode>")) return false;

	// Bloc_Continue
	if (!appStr(p, end, "<Bloc_Continue>")) return false;
	if (!appChr(p, end, m_blocContinue ? '1' : '0')) return false;
	if (!appStr(p, end, "</Bloc_Continue>")) return false;

	// Bloc_Cancel
	if (!appStr(p, end, "<Bloc_Cancel>")) return false;
	if (!appChr(p, end, m_blocCancel ? '1' : '0')) return false;
	if (!appStr(p, end, "</Bloc_Cancel>")) return false;

	// IPOC
	if (!appStr(p, end, "<IPOC>")) return false;
	// copie directe du contenu IPOC
	for (char c : m_ipoc) { if (p >= end) return false; *p++ = c; }
	if (!appStr(p, end, "</IPOC>")) return false;

	// Additionnal XML nodes
	for (const auto& xmlNode : m_additionnalXmlNodes)
	{
		if (!xmlNode.toBuffer(p, end, true))
		{
			m_additionnalXmlNodes.clear();
			return false;
		}
	}
	m_additionnalXmlNodes.clear();

	// Fin
	if (!appStr(p, end, "</Sen>")) return false;

	outLen = static_cast<std::size_t>(p - dst);
	return true;
}

inline bool RsiTrame::appChr(char*& p, const char* end, char c) 
{
	if (p >= end) 
		return false; 
	*p++ = c; 
	return true;
}

inline bool RsiTrame::appStr(char*& p, const char* end, const char* s)
{
	while (*s) 
	{ 
		if (p >= end) 
			return false; 
		*p++ = *s++;
	}
	return true;
}

inline bool RsiTrame::appFloatFixed(char*& p, char* end, double v, int precision) 
{
#if __cpp_lib_to_chars >= 201611L
	auto res = std::to_chars(p, end, v, std::chars_format::fixed, precision);
	if (res.ec != std::errc{}) return false;
	p = res.ptr; return true;
#else
	int avail = static_cast<int>(end - p);
	if (avail <= 0) return false;
	int n = std::snprintf(p, avail, "%.*f", precision, v);
	if (n <= 0 || n >= avail) return false;
	p += n; return true;
#endif
}