#include "pch.h"
#include "NetworkInterface.h"

int NetworkInterface::WebsockInit()
{
	int resultCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (resultCode != 0)
		return SOCKERROR;
	else
		return SOCKOK;
}

int NetworkInterface::GetSize()
{
	int size = 0;
	size += sizeof(wsaData);
	size += socketData.size() * sizeof(std::pair<SOCKET, SockData>);

	return size;
}

SOCKET NetworkInterface::InitNewSocket(FamilyType familyType, ProtocolType protocolType, bool isListen, const char* port, const char* addr)
{
	struct addrinfo *result = NULL, hints;
	int resultCode;
	SockData data;

	ZeroMemory(&hints, sizeof(hints));

	if (familyType == FamilyType::IPv4 && isListen)
		hints.ai_family = AF_INET;
	else if (familyType == FamilyType::IPv6 && isListen)
		hints.ai_family = AF_INET6;
	else
		hints.ai_family = AF_UNSPEC;

	if (protocolType == ProtocolType::TCP)
	{
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}
	else if (protocolType == ProtocolType::UDP)
	{
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}

	if (isListen)
		hints.ai_flags = AI_PASSIVE;

	if (isListen == false && addr != NULL)
		resultCode = getaddrinfo(addr, port, &hints, &result);
	else if (isListen == false && addr == NULL)
		return SOCKERROR;
	else if (isListen)
		resultCode = getaddrinfo(NULL, port, &hints, &result);

	if (resultCode != 0)
		return SOCKERROR;

	SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (sock == INVALID_SOCKET)
		return SOCKERROR;

	data.socketAddrInfo = result;
	data.protocolType = protocolType;

	socketData.insert(std::pair<SOCKET, SockData>(sock, data));

	return sock;
}

int NetworkInterface::Connect(SOCKET socket)
{
	if (socket != SOCKERROR)
	{
		SockData data = socketData.at(socket);

		int resultCode = connect(socket, data.socketAddrInfo->ai_addr, (int)data.socketAddrInfo->ai_addrlen);

		if (resultCode == SOCKET_ERROR)
			return SOCKERROR;
	}
	else
		return SOCKERROR;

	return SOCKOK;
}

int NetworkInterface::Bind(SOCKET socket)
{
	if (socket != SOCKERROR)
	{
		SockData data = socketData.at(socket);

		int resultCode = bind(socket, data.socketAddrInfo->ai_addr, (int)data.socketAddrInfo->ai_addrlen);

		if (resultCode == SOCKET_ERROR)
			return SOCKERROR;
	}
	else
		return SOCKERROR;

	return SOCKOK;
}

int NetworkInterface::Listen(SOCKET socket)
{
	if (socket != SOCKERROR)
	{
		int resultCode = listen(socket, SOMAXCONN);

		if (resultCode == SOCKET_ERROR)
			return SOCKERROR;
	}
	else
		return SOCKERROR;

	return SOCKOK;
}

NetworkInterface::SAcceptData NetworkInterface::Accept(SOCKET socket)
{
	SAcceptData data;

	if (socket != SOCKERROR)
	{
		sockaddr_in in;
		int sockAddrLen = sizeof(in);

		ZeroMemory(&in, sizeof(in));

		data.socket = accept(socket, (SOCKADDR*)& in, &sockAddrLen);

		if (socket == INVALID_SOCKET)
			data.socket = SOCKERROR;
		else
		{
			data.peerIP = inet_ntoa(in.sin_addr);
			data.peerPort = htons(in.sin_port);
		}
	}
	else
		data.socket = SOCKERROR;

	return data;
}

int NetworkInterface::GetLastErrorCode()
{
	return WSAGetLastError();
}

NetworkInterface::SRecvData NetworkInterface::ReciveMessage(SOCKET socket)
{
	// Select to check if any messsages are waiting to recive
	// If not, return an IsMessage false on the returning struct
	// Else, Get the message and save it to the returning struct

	// If error, set the isError flag to true in the returning struct

	SRecvData recvData = SRecvData();

	if (socket != SOCKERROR)
	{
		fd_set readSet;

		FD_ZERO(&readSet);
		FD_SET(socket, &readSet);

		timeval timeout;

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int resultCode = select(socket + 1, &readSet, nullptr, nullptr, &timeout);

		if (resultCode != SOCKET_ERROR && resultCode != 0)
		{
			// An message can be aquired

			SockData data = socketData.at(socket);

			char* recvBuffer = new char[MAXBUFFERSIZE];

			if (data.protocolType == ProtocolType::TCP)
			{
				resultCode = recv(socket, recvBuffer, MAXBUFFERSIZE, NULL);

				if (resultCode == SOCKET_ERROR)
				{
					recvData.isError = true;
					recvData.isMessage = false;
				}
				else
				{
					recvData.isError = false;
					recvData.isMessage = true;

					recvData.buffer = recvBuffer;
					recvData.bufferSize = resultCode;
				}
			}
			else if (data.protocolType == ProtocolType::UDP)
			{
				/*sockaddr_in sockAddr;
				int sockAddrSize = sizeof(sockaddr_in);

				ZeroMemory(&sockAddr, sizeof(SOCKADDR_IN));*/

				resultCode = recvfrom(socket, recvBuffer, MAXBUFFERSIZE, 0, NULL, NULL);

				if (resultCode == SOCKET_ERROR)
				{
					recvData.isError = true;
					recvData.isMessage = false;
				}
				else if (resultCode != 0)
				{
					recvData.isError = false;
					recvData.isMessage = true;

					recvData.buffer = recvBuffer;
					recvData.bufferSize = resultCode;
				}
				else
				{
					recvData.isError = true;
					recvData.isMessage = false;
				}
			}
		}
		else if (resultCode == SOCKET_ERROR)
		{
			recvData.isMessage = false;
			recvData.isError = true;
		}
		else
		{
			recvData.isMessage = false;
			recvData.isError = false;
		}
	}
	else
	{
		recvData.isMessage = false;
		recvData.isError = true;
	}

	return recvData;
}

int NetworkInterface::SendBuffer(SOCKET socket, char* buffer, size_t sizeOfBuffer, const char* addr, int port)
{
	// Send the buffer to and remote location

	if (socket != SOCKERROR)
	{
		SockData sData = socketData.at(socket);
		int resultCode = 0;

		if (sData.protocolType == ProtocolType::TCP)
		{
			// Send the message with the send function

			resultCode = send(socket, buffer, sizeOfBuffer, 0);

			if (resultCode == SOCKET_ERROR)
				return SOCKERROR;
		}
		else if (sData.protocolType == ProtocolType::UDP)
		{
			// Send the message with the sendto function

			sockaddr_in to;

			ZeroMemory(&to, sizeof(to));

			if (addr != NULL && port != NULL)
			{
				to.sin_family = AF_INET;
				inet_pton(AF_INET, addr, &to.sin_addr);
				to.sin_port = htons(port);
			}
			else
				return SOCKERROR;

			resultCode = sendto(socket, buffer, sizeOfBuffer, 0, (SOCKADDR*)&to, sizeof(to));

			if (resultCode == SOCKET_ERROR)
				return SOCKERROR;
		}
	}
	else
		return SOCKERROR;

	return SOCKOK;
}

NetworkInterface::SockData* NetworkInterface::GetSocketData(SOCKET socket)
{
	if (socket != SOCKERROR)
	{
		auto it = socketData.find(socket);

		if (it != socketData.end())
			return &(*it).second;
		else
		{
			NetworkInterface::SockData* sData = new NetworkInterface::SockData();
			sData->isOutOfRange = true;
			return sData;
		}
	}
	else
	{
		return nullptr;
	}
}

void NetworkInterface::AddSocket(SOCKET socket, ProtocolType protocolType)
{
	SockData data;

	data.protocolType = protocolType;

	socketData.insert(std::pair<SOCKET, SockData>(socket, data));
}

char* NetworkInterface::PackageStruct(char* buffer, size_t bufferSize, void* header, size_t headerSize)
{
	char* headerBuffer = new char[headerSize];

	memcpy(&headerBuffer[0], header, headerSize);

	char* pBuffer = new char[headerSize + bufferSize];

	memcpy(&pBuffer[0], &headerBuffer[0], headerSize);
	memcpy(&pBuffer[headerSize], &buffer[0], bufferSize);

	delete[] headerBuffer;

	return pBuffer;
}

char* NetworkInterface::UnpackageStruct(char* buffer, size_t bufferSize, void* header, size_t headerSize)
{
	// Get the header
	memcpy(header, &buffer[0], headerSize);

	char* message = new char[bufferSize - headerSize];

	// Get the message
	memcpy(&message[0], &buffer[headerSize], bufferSize - headerSize);

	return message;
}

void NetworkInterface::UnpackageStructTo(char* buffer, size_t bufferSize, void* header, size_t headerSize, void* messageTo)
{
	// Get the header
	memcpy(header, &buffer[0], headerSize);

	// Get the message
	memcpy(messageTo, &buffer[headerSize], bufferSize - headerSize);
}

void NetworkInterface::GetStruct(char* buffer, size_t startAt, void* s, size_t headerSize)
{
	memcpy(s, &buffer[startAt], headerSize);
}
