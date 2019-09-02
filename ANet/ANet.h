#pragma once

#include "NetworkInterface.h"

#ifdef ANET_EXPORTS
#define ANET_API __declspec(dllexport)
#else
#define ANET_API __declspec(dllimport)
#endif

struct SocketData
{
	const char* ip;
	NetworkInterface::ProtocolType protocolType;
	bool isOutOfRange;
};

struct InitReturnStruct
{
	NetworkInterface* NetHandle;
	bool isSuccess;
};

extern "C"
{
	// Init the ANet API
	// returnType: Was the initialization successfull?
	ANET_API InitReturnStruct ANetInit();

	// Create a new socket with tcp or udp protocol
	// familyType: The ip family you want to use
	// protocolType: The protocol you want to use
	// isListen: Is this socket an listen socket
	// port: The port to bind the socket to
	// addr: The addr to bind the socket to
	// returnType: The created socket or an error code
	ANET_API SOCKET CreateSocket(NetworkInterface* netHandler, NetworkInterface::FamilyType familyType, NetworkInterface::ProtocolType protocolType, bool isListen, const char* port, const char* addr = NULL);

	// Connect the socket to remote location
	// socket: The socket to make an connection on
	// timeout: Seconds to wait on connection
	// returnType: if a connection was successfull
	ANET_API int Connect(NetworkInterface* netHandler, SOCKET socket, float timeout);

	// Bind the socket to localhost
	// socket: The socket to bind on
	// return: if kernel could bind the given socket
	ANET_API int Bind(NetworkInterface* netHandler, SOCKET socket);

	// Listen for incoming connections
	// socket: The socket to listen on
	// returnType: If listen was successfull
	ANET_API int Listen(NetworkInterface* netHandler, SOCKET socket);

	// Accept an incoming connection
	// socket: The socket to accept an connection on
	// returnType: Data about the accepted connection
	ANET_API NetworkInterface::SAcceptData Accept(NetworkInterface* netHandler, SOCKET socket);

	// Get the last error code
	// returnType: The error code
	ANET_API int GetLastErrorCode(NetworkInterface* netHandler);

	// Recieve a buffer on the given socket
	// socket: The socket to receive an buffer on
	// returnType: data about the received buffer
	ANET_API NetworkInterface::SRecvData RecieveBufferA(NetworkInterface* netHandler, SOCKET socket);

	// Send a buffer to an remote connection/location
	// socket: The socket to send an buffer on
	// buffer: The buffer to send
	// sizeOfBuffer: The buffer size
	// addr: What addr to send to if udp protocol
	// port: What port to send on if udp protocol
	// returnType: Have the buffer been sent away successfully?
	ANET_API int SendBufferA(NetworkInterface* netHandler, SOCKET socket, char* buffer, size_t sizeOfBuffer, char* addr = NULL, int port = NULL);
	
	// Adds an socket to the socketData vector
	// socket: What socket to add
	// protocolType: The protocolType of the socket
	ANET_API void AddSocket(NetworkInterface* netHandler, SOCKET socket, NetworkInterface::ProtocolType protocolType);

	// Package and buffer and struct into an single buffer
	// buffer: The buffer to package to an single buffer
	// bufferSize: The size of the given buffer
	// header: The header to package to an single buffer
	// headerSize: The size of the given header
	// returnType: The full buffer
	ANET_API char* PackageStructA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize);

	// Unpackage the buffer and struct from the given buffer
	// buffer: The entire buffer containing the header and the buffer
	// bufferSize: The size of the given buffer
	// header: The header dataType to save the header data into
	// headerSize: The size of the given header
	// returnType: The small buffer in the given buffer
	ANET_API char* UnpackageStructA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize);

	// Unpackage the buffer to and given dataType and header
	// buffer: The entire buffer containing the header and the smaller buffer
	// bufferSize: The size of the given buffer
	// header: The header to save the header data into
	// headerSize: The size of the given header
	// bufferTo: The dataType to save the smaller buffer into
	ANET_API void UnpackageStructToA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize, void* bufferTo);

	// Gets the addrinfo and protocol for the given socket
	// socket: The socket to get the data from
	ANET_API SocketData GetSocketData(NetworkInterface* netHandler, SOCKET socket);

	// Get the header from the package buffer
	// buffer: The buffer containing the header
	// header: The header dataType to save the data into
	// headerSize: The size of the given header
	ANET_API void GetStructA(NetworkInterface* netHandler, char* buffer, size_t startAt, void* header, size_t headerSize);

	// Clear the memory of the given buffer
	// buffer: The buffer to clear the memory from
	ANET_API void ClearBuffer(char* buffer);

	// Deinit the ANet API
	ANET_API void ANetDeinit(NetworkInterface* netHandler);
}