#include "NativeUdpSocket.h"

std::atomic<uint32_t> NativeUdpSocket::s_instanceCount = 0;

NativeUdpSocket::NativeUdpSocket(const std::string& localIp, u_short localPort, int recvBufferSize) :
	m_socket(INVALID_SOCKET),
	m_localIp(localIp),
	m_localPort(localPort),
	m_recvBufferSize(recvBufferSize)
{
	if (!ensure_wsa_started())
		return;
}

NativeUdpSocket::NativeUdpSocket(const char* localIp, u_short localPort, int recvBufferSize) :
	m_socket(INVALID_SOCKET),
	m_localIp(localIp),
	m_localPort(localPort),
	m_recvBufferSize(recvBufferSize)
{
	if (!ensure_wsa_started()) 
		return;
}

NativeUdpSocket::~NativeUdpSocket()
{
	close();
	maybe_wsa_cleanup();
}

bool NativeUdpSocket::ensure_wsa_started() 
{
	uint32_t prev = s_instanceCount.fetch_add(1, std::memory_order_acq_rel);
	if (prev == 0) 
	{
		WSADATA wsaData;
		const int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (rc != 0) {
			s_instanceCount.fetch_sub(1, std::memory_order_acq_rel);
			return false;
		}
	}
	return true;
}

void NativeUdpSocket::maybe_wsa_cleanup() 
{
	uint32_t prev = s_instanceCount.fetch_sub(1, std::memory_order_acq_rel);
	if (prev == 1) 
	{
		WSACleanup();
	}
}

bool NativeUdpSocket::open()
{
	if (m_socket != INVALID_SOCKET) 
		return true; // déjà ouvert

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET)
		return false;

	if (m_recvBufferSize > 0)
	{
		setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF,
			reinterpret_cast<const char*>(&m_recvBufferSize),
			sizeof(m_recvBufferSize));
	}
	m_opened = true;
	return true;
}

bool NativeUdpSocket::bind()
{
	if (m_socket == INVALID_SOCKET)
		return false;

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_localPort);
	if (inet_pton(AF_INET, m_localIp.c_str(), &addr.sin_addr) != 1)
	{
		return false;
	}
	return ::bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != SOCKET_ERROR;
}

void NativeUdpSocket::close()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET; 
	}
	m_connected = false;
	m_opened = false;
}

int NativeUdpSocket::sendTo(const char* data, int len, const std::string& destIp, u_short destPort)
{
	if (m_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(destPort);
	if (inet_pton(AF_INET, destIp.c_str(), &addr.sin_addr) != 1)
	{
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}
	return sendto(m_socket, data, len, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

bool NativeUdpSocket::connectTo(const std::string& destIp, u_short destPort)
{
	return connectTo(destIp.c_str(), destPort);
}

bool NativeUdpSocket::connectTo(const char* destIp, u_short destPort)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(destPort);
	if (inet_pton(AF_INET, destIp, &addr.sin_addr) != 1)
	{
		return false;
	}
	int rc = ::connect(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	m_connected = (rc != SOCKET_ERROR);
	return m_connected;
}

int NativeUdpSocket::send(const char* data, int len)
{
	if (m_socket == INVALID_SOCKET || !m_connected)
	{
		WSASetLastError(WSAENOTCONN);
		return SOCKET_ERROR;
	}
	return ::send(m_socket, data, len, 0);
}

bool NativeUdpSocket::waitForData(int timeoutMs)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_socket, &readfds);

	timeval tv;
	tv.tv_sec = timeoutMs / 1000;
	tv.tv_usec = (timeoutMs % 1000) * 1000;

	// Sous Winsock, le premier argument de select est ignoré
	int result = select(0, &readfds, nullptr, nullptr, &tv);
	return (result > 0) && FD_ISSET(m_socket, &readfds);
}

int NativeUdpSocket::recvFrom(char* buffer, int buflen, char* senderIp, int senderIpCap, u_short& senderPort)
{
	if (m_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	sockaddr_in from = {};
	int fromlen = sizeof(from);

	int ret = recvfrom(m_socket, buffer, buflen, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
	if (ret > 0)
	{
		// Write sender IP to senderIp buffer
		if (inet_ntop(AF_INET, &from.sin_addr, senderIp, senderIpCap) == nullptr)
		{
			// If conversion fails, set first char to null
			if (senderIp) 
				senderIp[0] = '\0';
		}
		senderPort = ntohs(from.sin_port);
	}
	return ret;
}

int NativeUdpSocket::recvFrom(char* buffer, int buflen, std::string& senderIp, u_short& senderPort)
{
	if (m_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	sockaddr_in from = {};
	int fromlen = sizeof(from);

	int ret = recvfrom(m_socket, buffer, buflen, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
	if (ret > 0)
	{
		char ipbuf[INET_ADDRSTRLEN] = {};
		if (inet_ntop(AF_INET, &from.sin_addr, ipbuf, sizeof(ipbuf)) != nullptr)
		{
			senderIp = ipbuf;
		}
		else
		{
			senderIp.clear();
		}
		senderPort = ntohs(from.sin_port);
	}
	return ret;
}

int NativeUdpSocket::recv(char* buffer, int buflen)
{
	if (m_socket == INVALID_SOCKET || !m_connected)
	{
		WSASetLastError(WSAENOTCONN);
		return SOCKET_ERROR;
	}
	return ::recv(m_socket, buffer, buflen, 0);
}
