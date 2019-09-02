#include "pch.h"
#include "ANet.h"

InitReturnStruct ANetInit()
{
	InitReturnStruct initRetStruct = InitReturnStruct();

	NetworkInterface* netHandler = new NetworkInterface();
	initRetStruct.isSuccess = (bool)netHandler->WebsockInit();
	initRetStruct.NetHandle = netHandler;

	return initRetStruct;
}

SOCKET CreateSocket(NetworkInterface* netHandler, NetworkInterface::FamilyType familyType, NetworkInterface::ProtocolType protocolType, bool isListen, const char* port, const char* addr)
{
	return netHandler->InitNewSocket(familyType, protocolType, isListen, port, addr);
}

int Connect(NetworkInterface* netHandler, SOCKET socket, float timeout)
{
	fd_set wSet;
	timeval ts;

	ts.tv_sec = (long)timeout;
	ts.tv_usec = 0;

	std::map<SOCKET, NetworkInterface::SockData>::iterator sockData = netHandler->GetSocketDataMap().find(socket);
	
	if (sockData != netHandler->GetSocketDataMap().end() && (*sockData).second.protocolType == NetworkInterface::ProtocolType::TCP)
	{
		FD_ZERO(&wSet);
		FD_SET(socket, &wSet);

		u_long blockingType = 1;

		ioctlsocket(socket, FIONBIO, &blockingType);

		int resultCode = netHandler->Connect(socket);

		if (resultCode == SOCKOK || netHandler->GetLastErrorCode() == WSAEWOULDBLOCK)
		{
			resultCode = select((int)socket + 1, nullptr, &wSet, nullptr, &ts);

			blockingType = 0;

			ioctlsocket(socket, FIONBIO, &blockingType);

			if (resultCode > 0)
			{
				return CONNECTOK;
			}
			else
			{
				return CONNECTTIMEOUT;
			}
		}
		else
		{
			return CONNECTERROR;
		}
	}
	else
	{
		return CONNECTERROR;
	}
}

int Bind(NetworkInterface* netHandler, SOCKET socket)
{
	return netHandler->Bind(socket);
}

int Listen(NetworkInterface* netHandler, SOCKET socket)
{
	return netHandler->Listen(socket);
}

NetworkInterface::SAcceptData Accept(NetworkInterface* netHandler, SOCKET socket)
{
	return netHandler->Accept(socket);
}

int GetLastErrorCode(NetworkInterface* netHandler)
{
	return netHandler->GetLastErrorCode();
}

NetworkInterface::SRecvData RecieveBufferA(NetworkInterface* netHandler, SOCKET socket)
{
	NetworkInterface::SRecvData recvData = NetworkInterface::SRecvData();

	recvData = netHandler->ReciveMessage(socket);

	return recvData;
}

int SendBufferA(NetworkInterface* netHandler, SOCKET socket, char* buffer, size_t sizeOfBuffer, char* addr, int port)
{
	return netHandler->SendBuffer(socket, buffer, sizeOfBuffer, addr, port);
}

void AddSocket(NetworkInterface* netHandler, SOCKET socket, NetworkInterface::ProtocolType protocolType)
{
	netHandler->AddSocket(socket, protocolType);
}

char* PackageStructA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize)
{
	return netHandler->PackageStruct(buffer, bufferSize, header, headerSize);
}

char* UnpackageStructA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize)
{
	return netHandler->UnpackageStruct(buffer, bufferSize, header, headerSize);
}

void UnpackageStructToA(NetworkInterface* netHandler, char* buffer, size_t bufferSize, void* header, size_t headerSize, void* bufferTo)
{
	netHandler->UnpackageStructTo(buffer, bufferSize, header, headerSize, bufferTo);
}

SocketData GetSocketData(NetworkInterface* netHandler, SOCKET socket)
{
	NetworkInterface::SockData* sockData = netHandler->GetSocketData(socket);

	SocketData sData = SocketData();
	
	if (sockData->isOutOfRange == false)
	{
		sData.ip = inet_ntoa(((SOCKADDR_IN*)sockData->socketAddrInfo->ai_addr)->sin_addr);
		sData.protocolType = sockData->protocolType;
	}
	else
	{
		sData.isOutOfRange = true;
		delete sockData;
	}

	return sData;
}

void GetStructA(NetworkInterface* netHandler, char* buffer, size_t startAt, void* header, size_t headerSize)
{
	netHandler->GetStruct(buffer, startAt, header, headerSize);
}

void ClearBuffer(char* buffer)
{
	delete[] buffer;
	buffer = nullptr;
}

void ANetDeinit(NetworkInterface* netHandler)
{
	delete netHandler;
}

void ClearData(void* data)
{
	delete data;
}
