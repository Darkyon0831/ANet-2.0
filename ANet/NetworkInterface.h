#ifndef NETWORKINTERFACE
#define NETWORKINTERFACE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // !_WINSOCK_DEPRECATED_NO_WARNINGS

#define SOCKERROR -1
#define SOCKOK 0

#define CONNECTERROR 2
#define CONNECTTIMEOUT 1
#define CONNECTOK 0

#define MAXBUFFERSIZE 1024

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

class NetworkInterface
{
public:

	enum FamilyType
	{
		IPv4,
		IPv6
	};

	enum ProtocolType
	{
		TCP,
		UDP
	};

	struct SAcceptData
	{
		SOCKET socket;
		char* peerIP;
		int peerPort;
	};

	struct SRecvData
	{
		char* buffer;
		int bufferSize;
		bool isMessage;
		bool isError;
	};

	struct SockData
	{
		addrinfo* socketAddrInfo;
		ProtocolType protocolType;
		bool isOutOfRange;
	};

	// Init the Websocket API
	int WebsockInit();

	int GetSize();

	// Init a new socket with tcp or udp protocol
	SOCKET InitNewSocket(FamilyType familyType, ProtocolType protocolType, bool isListen, const char* port, const char* addr = NULL);

	// Connect the socket to remote location
	int Connect(SOCKET socket);

	// Bind the socket to localhost
	int Bind(SOCKET socket);

	// Listen for incoming connections
	int Listen(SOCKET socket);

	// Accept an incoming connection
	SAcceptData Accept(SOCKET socket);

	// Get the last error
	int GetLastErrorCode();

	// Recive a message on the given socket
	SRecvData ReciveMessage(SOCKET socket);

	// Send a buffer to an remote connection/location
	int SendBuffer(SOCKET socket, char* buffer, size_t sizeOfBuffer, const char* addr = NULL, int port = NULL);

	// Gets the addrinfo and protocol for the given socket
	SockData* GetSocketData(SOCKET socket);

	// Adds an socket to the socketData vector
	void AddSocket(SOCKET socket, ProtocolType protocolType);

	// Package and buffer and struct into an single buffer
	char* PackageStruct(char* buffer, size_t bufferSize, void* header, size_t headerSize);

	// Unpackage the buffer and struct from the given buffer
	char* UnpackageStruct(char* buffer, size_t bufferSize, void* header, size_t headerSize);

	// Unpackage the buffer to and given dataType and header
	void UnpackageStructTo(char* buffer, size_t bufferSize, void* header, size_t headerSize, void* messageTo);

	// Get the header from the package buffer
	void GetStruct(char* buffer, size_t startAt, void* s, size_t headerSize);

	std::map<SOCKET, SockData> GetSocketDataMap() { return socketData; }
	
private:
	WSADATA wsaData;

	std::map<SOCKET, SockData> socketData;
};

#endif // !NETWORKINTERFACE
