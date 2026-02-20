#include "XmlNode.h"
#include <sstream>

XmlNode::XmlNode(const std::string& name) :
    m_name(name) 
{}

XmlNode::XmlNode(std::string&& name) :
    m_name(std::move(name))
{}

XmlNode::XmlNode(const std::string& name, const std::string& value) :
    m_name(name), 
    m_value(value)
{}

XmlNode::XmlNode(std::string&& name, std::string&& value) :
    m_name(std::move(name)),
    m_value(std::move(value)) 
{}

std::string XmlNode::getAttribute(const std::string& name) const
{
    auto it = m_attributes.find(name);
    return (it != m_attributes.end()) ? it->second : "";
}

bool XmlNode::hasAttribute(const std::string& name) const 
{
    return m_attributes.find(name) != m_attributes.end();
}

void XmlNode::addChild(const XmlNode& child)
{
    m_children.push_back(std::make_shared<XmlNode>(child));
}

void XmlNode::addChild(std::shared_ptr<XmlNode> child) 
{
    m_children.push_back(child);
}

std::string XmlNode::escapeXml(const std::string& text) const 
{
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        switch (c) {
        case '&':  result += "&amp;"; break;
        case '<':  result += "&lt;"; break;
        case '>':  result += "&gt;"; break;
        case '"':  result += "&quot;"; break;
        case '\'': result += "&apos;"; break;
        default:   result += c; break;
        }
    }
    return result;
}

std::string XmlNode::getIndentation(int level) const
{
    return std::string(level * 2, ' ');
}

void XmlNode::toOSS(std::ostringstream& oss, int indent, bool singleLine) const
{
    std::string ind = singleLine ? "" : getIndentation(indent);
    std::string newline = singleLine ? "" : "\n";

    // Balise ouvrante
    oss << ind << "<" << m_name;

    // Attributs
    for (const auto& attr : m_attributes)
    {
        oss << " " << attr.first << "=\"" << escapeXml(attr.second) << "\"";
    }

    // Si le noeud a une valeur mais pas d'enfants
    if (hasValue() && !hasChildren())
    {
        oss << ">" << escapeXml(m_value) << "</" << m_name << ">";
    }
    // Si le noeud a des enfants
    else if (hasChildren())
    {
        oss << ">" << newline;
        for (const auto& child : m_children)
        {
            oss << child->toString(indent + 1, singleLine);
            if (!singleLine)
                oss << "\n";
        }
        oss << ind << "</" << m_name << ">";
    }
    // Noeud vide
    else
    {
        oss << " />";
    }
}

std::string XmlNode::toString(int indent, bool singleLine) const 
{
    std::ostringstream oss;
    toOSS(oss, indent, singleLine);
    return oss.str();
}

bool XmlNode::appStr(char*& p, char* end, const char* str) 
{
    while (*str) {
        if (p >= end) return false;
        *p++ = *str++;
    }
    return true;
}

bool XmlNode::appStr(char*& p, char* end, const std::string& str) 
{
    for (char c : str) {
        if (p >= end) return false;
        *p++ = c;
    }
    return true;
}

bool XmlNode::appChr(char*& p, char* end, char c)
{
    if (p >= end) return false;
    *p++ = c;
    return true;
}

bool XmlNode::appEscapedXml(char*& p, char* end, const std::string& text) const 
{
    for (char c : text) {
        const char* escaped = nullptr;
        switch (c) {
        case '&':  escaped = "&amp;"; break;
        case '<':  escaped = "&lt;"; break;
        case '>':  escaped = "&gt;"; break;
        case '"':  escaped = "&quot;"; break;
        case '\'': escaped = "&apos;"; break;
        default:
            if (p >= end) return false;
            *p++ = c;
            continue;
        }
        if (!appStr(p, end, escaped)) return false;
    }
    return true;
}

bool XmlNode::toBuffer(char*& p, char* end, bool singleLine) const 
{
    // Balise ouvrante
    if (!appChr(p, end, '<')) return false;
    if (!appStr(p, end, m_name)) return false;

    // Attributs
    for (const auto& attr : m_attributes) 
    {
        if (!appChr(p, end, ' ')) return false;
        if (!appStr(p, end, attr.first)) return false;
        if (!appStr(p, end, "=\"")) return false;
        if (!appEscapedXml(p, end, attr.second)) return false;
        if (!appChr(p, end, '"')) return false;
    }

    // Si le noeud a une valeur mais pas d'enfants
    if (hasValue() && !hasChildren())
    {
        if (!appChr(p, end, '>')) return false;
        if (!appEscapedXml(p, end, m_value)) return false;
        if (!appStr(p, end, "</")) return false;
        if (!appStr(p, end, m_name)) return false;
        if (!appChr(p, end, '>')) return false;
    }
    // Si le noeud a des enfants
    else if (hasChildren())
    {
        if (!appChr(p, end, '>')) return false;
        for (const auto& child : m_children) {
            if (!child->toBuffer(p, end, singleLine)) return false;
        }
        if (!appStr(p, end, "</")) return false;
        if (!appStr(p, end, m_name)) return false;
        if (!appChr(p, end, '>')) return false;
    }
    // Noeud vide
    else 
    {
        if (!appStr(p, end, " />")) return false;
    }

    return true;
}