#pragma once
#include "Trame.h"
#include "XmlNode.h"
#include <vector>
#include <string>
#include <string_view>
#include <cstddef>

#include <charconv>     // std::to_chars, std::chars_format
#include <system_error> // std::errc

class RsiTrame : public Trame
{
public:
	RsiTrame();

	inline void reset();

	void setPose(bool isCartesian, float pos[6], bool isInRobotBase);
	void setIPOC(std::string_view ipoc) { m_ipoc.assign(ipoc.data(), ipoc.size()); }

	inline std::uint16_t getOutputs() { return m_digout; }
	void setOutputs(std::uint16_t outputs) { m_digout = outputs; }
	void setOutputs(bool outputs[16]) { encodeBoolArrayToUShort(outputs, m_digout); }

	void setStopFlag(bool stopFlag) { m_flagExitCorr = stopFlag; }
	void setBlocContinue(bool blocContinue) { m_blocContinue = blocContinue; }
	void setBlocCancel(bool blocCancel) { m_blocCancel = blocCancel; }

	void setKrl(std::uint8_t krl) { m_krl = krl; }
	void setMode(std::uint8_t mode) { m_mode = mode; }

	// Inherited via Trame
	inline bool isValid() const override;
	inline std::string build() const override;
	inline bool build(char* dst, std::size_t cap, std::size_t& outLen) const override;
	inline Type type() const override
	{
		return Trame::Type::RSI;
	}

	void transmitAdditionnalXmlNodes(std::vector<XmlNode>& xmlNodes);

private:
	inline void encodeBoolArrayToUShort(bool io[16], std::uint16_t& nIO);

	static inline bool appChr(char*& p, const char* end, char c);
	static inline bool appStr(char*& p, const char* end, const char* s);

	template <class UInt>
	static inline bool appUInt(char*& p, char* end, UInt v)
	{
		auto res = std::to_chars(p, end, v);
		if (res.ec != std::errc{})
			return false;
		p = res.ptr; return true;
	}

	// to_chars pour float/double (fixed avec précision)
	static inline bool appFloatFixed(char*& p, char* end, double v, int precision);

private:
	bool m_flagExitCorr;
	bool m_isCartesian;
	bool m_isInRobotBase;
	bool m_blocContinue;
	bool m_blocCancel;
	std::uint16_t m_digout;
	std::uint8_t m_krl;
	std::uint8_t m_mode;
	std::string m_ipoc;
	float m_delta[6];

	mutable std::vector<XmlNode> m_additionnalXmlNodes;
};
