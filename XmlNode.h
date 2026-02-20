#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

class XmlNode {
public:
    // Constructeurs
    XmlNode(const std::string& name);
    XmlNode(std::string&& name);
    XmlNode(const std::string& name, const std::string& value);
    XmlNode(std::string&& name, std::string&& value);

    // Gestion du nom
    void setName(const std::string& name) { m_name = name; }
    void setName(std::string&& name) { m_name = std::move(name); }
    std::string getName() const { return m_name; }

    // Gestion de la valeur
    void setValue(const std::string& value) { m_value = value; }
    void setValue(std::string&& value) { m_value = std::move(value); }
    std::string getValue() const { return m_value; }
    bool hasValue() const { return !m_value.empty(); }

    // Gestion des attributs
    void setAttribute(const std::string& name, const std::string& value) { m_attributes[name] = value; }
    void setAttribute(std::string&& name, std::string&& value) { m_attributes[std::move(name)] = std::move(value); }
    std::string getAttribute(const std::string& name) const;
    bool hasAttribute(const std::string& name) const;

    // Gestion des enfants
    void addChild(const XmlNode& child);
    void addChild(std::shared_ptr<XmlNode> child);
    const std::vector<std::shared_ptr<XmlNode>>& getChildren() const { return m_children; }
    bool hasChildren() const { return !m_children.empty(); }

    // Génération XML
    void toOSS(std::ostringstream& oss, int indent = 0, bool singleLine = false) const;
    std::string toString(int indent = 0, bool singleLine = false) const;

    // Génération XML directe dans un buffer (optimisé pour éviter les allocations)
    // Retourne false si le buffer est trop petit
    bool toBuffer(char*& p, char* end, bool singleLine = true) const;

private:
    std::string escapeXml(const std::string& text) const;
    std::string getIndentation(int level) const;

    // Fonctions helper pour toBuffer
    static bool appStr(char*& p, char* end, const char* str);
    static bool appStr(char*& p, char* end, const std::string& str);
    static bool appChr(char*& p, char* end, char c);
    bool appEscapedXml(char*& p, char* end, const std::string& text) const;

private:
    std::string m_name;
    std::string m_value;
    std::map<std::string, std::string> m_attributes;
    std::vector<std::shared_ptr<XmlNode>> m_children;
};
