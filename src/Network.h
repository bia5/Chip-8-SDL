#pragma once

#include "SDL2/SDL_net.h"
#include <time.h>
#include <chrono>
#include <vector>
#include <iostream>

struct Client {
	IPaddress	ip;
};

struct ClientMessage {
	std::string	message;
	IPaddress	ip;
};

//TODO make update return a vector of strings

class Network {
public:
	std::string			serverName = "localhost";
	short				PORT = 9999;
	bool				isServer = false;
	const int32_t		PACKET_SIZE = 512;
	int					PACKET_AMT = 120;
	bool				DEBUG = false;
	bool				hasInit = false;
	bool				forceServer = false;

	int					MAX_CLIENTS = 100;
	std::vector<Client*>clients;
	int					clientCount = 0;

	const std::string	MESSAGE_JOIN = "JOIN";
	const std::string	MESSAGE_PING = "PING";
	const std::string	MESSAGE_PEW = "PEW";
	const std::string	MESSAGE_FULL = "FULL";
	const std::string	MESSAGE_ACCEPTED = "ACCEPTED";
	const std::string	MESSAGE_ALREADYJOINED = "ALREADYJOINED";
	const std::string	MESSAGE_DISCONNECTED = "DISCONNECTED";

	UDPsocket			UDPSocket;	//Local Socket
	IPaddress			ip;			//Server IP
	UDPpacket*			packet;

	//Data (not needed but is cool to display)
	int	totalPacketsSent = 0, totalPacketsRecv = 0;
	int totalBytesSent = 0, totalBytesRecv = 0;
	int timedPacketsSent = 0, timedPacketsRecv = 0;
	int timedBytesSent = 0, timedBytesRecv = 0;
	int lastPacketsSent = 0, lastPacketsRecv = 0;
	int lastBytesSent = 0, lastBytesRecv = 0;

	time_t start;
	long long ping;
	std::chrono::time_point<std::chrono::system_clock> pingTimer;

	Network();

	void init(bool __isServer);
	std::vector<ClientMessage> update();

	void sendMessage(std::string message, IPaddress _ip);
	UDPpacket* allocPacket(int32_t size);
	UDPpacket** allocPackets(int32_t amt, int32_t size);
	void refreshIP();
	void refreshPing(IPaddress ip);
	bool isIPRegistered(IPaddress ip);
	int getIPLoc(IPaddress ip);
	void kickIP(IPaddress ip);

	IPaddress getIP();
	long long getPing();
	void setServerName(std::string name);
	std::string getServerName();
	void setServerPort(short port);
	short getServerPort();
	void setIsServer(bool isIt);
	bool getIsServer();
	int getPacketSize();
	void setPacketAmt(int amt);
	int getPacketAmt();
	void setDebugMode(bool isIt);
	bool getDebugMode();
	void setMaxClients(int amt);
	int getMaxClients();
	int getClientCount();

	int getTotalPacketsSent();
	int getTotalPacketsRecv();
	int getTotalBitsSent();
	int getTotalBitsRecv();
	int getTimedPacketsSent();
	int getTimedPacketsRecv();
	int getTimedBitsSent();
	int getTimedBitsRecv();

	std::string getDataFromClientMessage(ClientMessage cm);
	IPaddress getIPFromClientMessage(ClientMessage cm);
};