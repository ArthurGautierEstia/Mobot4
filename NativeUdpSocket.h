#pragma once
#include "platform_windows.h"

#include <string>
#include <atomic>

class NativeUdpSocket {
public:
    // Constructeur: uniquement l'endpoint local
    NativeUdpSocket(const std::string& localIp, u_short localPort, int recvBufSize = 0);
    NativeUdpSocket(const char* localIp, u_short localPort, int recvBufSize = 0);
    ~NativeUdpSocket();

    NativeUdpSocket(const NativeUdpSocket&) = delete;
    NativeUdpSocket& operator=(const NativeUdpSocket&) = delete;
    NativeUdpSocket(NativeUdpSocket&&) = default;
    NativeUdpSocket& operator=(NativeUdpSocket&&) = default;

    bool open();
    bool bind();
    void close();

	bool isOpened() const { return m_opened; }
	bool isConnected() const { return m_connected; }

    // Envoi vers une destination fournie à l'appel
    int sendTo(const char* data, int len, const std::string& destIp, u_short destPort);

    // Mode UDP "connecté": fixe un pair par défaut (facilite send/recv)
    bool connectTo(const char* destIp, u_short destPort);
    bool connectTo(const std::string& destIp, u_short destPort);
    int send(const char* data, int len);   // nécessite connectTo() préalable

    bool waitForData(int timeoutMs);

    // Réception: version qui retourne l'expéditeur
    int recvFrom(char* buffer, int buflen, char* senderIp, int senderIpCap, u_short& senderPort);
    int recvFrom(char* buffer, int buflen, std::string& senderIp, u_short& senderPort);

    // Réception simplifiée en mode "connecté"
    int recv(char* buffer, int buflen);

    // Getters
    std::string localIp() const { return m_localIp; }
    u_short localPort() const { return m_localPort; }

private:
    static bool ensure_wsa_started();
    static void maybe_wsa_cleanup();

private:
	static std::atomic<uint32_t> s_instanceCount;
    int m_recvBufferSize{ 0 };
    bool m_opened{ false };
    bool m_connected{ false };
    u_short m_localPort;
    SOCKET m_socket;
    std::string m_localIp;
};
