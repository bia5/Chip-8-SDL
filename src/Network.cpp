#include "Network.h"

Network::Network(){}

//If this is being used by itself you need to do SDLNet_INIT!
void Network::init(bool __isServer) {
	if(!forceServer)
		isServer = __isServer; //Dont forget to change for server if needed!
	start = time(0);
	if (isServer) { //Server INIT
		std::cout << "Network: INIT Server" << std::endl;
		std::cout << "Network: Opening port: " << PORT << std::endl;
		UDPSocket = SDLNet_UDP_Open(PORT);
		if (UDPSocket == nullptr)
			std::cout << "Error: Network::init: SDLNet_UDP_Open: " << SDLNet_GetError() << std::endl;
		std::cout << "Network: INIT Done!" << std::endl;
		std::cout << "Network: Waiting For Clients" << std::endl;

	} else { //Client INIT
		std::cout << "Network: INIT Client" << std::endl;
		std::cout << "Network: Opening port: " << PORT << std::endl;
		short port = PORT;
		UDPSocket = SDLNet_UDP_Open(PORT);
		while (UDPSocket == nullptr) {
			std::cout << "Error: Network::init: SDLNet_UDP_Open: " << SDLNet_GetError() << std::endl;
			PORT--;
			refreshIP();
			UDPSocket = SDLNet_UDP_Open(PORT);
		}
		std::cout << "Network: Port Opened at: " << PORT << std::endl;
		PORT = port;
		std::cout << "Network: Set port back to: " << PORT << std::endl;
		refreshIP();
	}
	hasInit = true;
}

std::vector<ClientMessage> Network::update() {
	if (hasInit) {
		std::vector<ClientMessage> msgs;
		if (difftime(time(0), start) >= 1) {
			lastPacketsSent = timedPacketsSent;
			lastPacketsRecv = timedPacketsRecv;
			lastBytesSent = timedBytesSent;
			lastBytesRecv = timedBytesRecv;

			timedPacketsSent = 0;
			timedPacketsRecv = 0;
			timedBytesSent = 0;
			timedBytesRecv = 0;
		}

		UDPpacket** packets = SDLNet_AllocPacketV(PACKET_AMT, PACKET_SIZE);
		int amt = SDLNet_UDP_RecvV(UDPSocket, packets);
		if (amt == -1)
			std::cout << "Error: Network::update: " << SDLNet_GetError() << std::endl;
		int length = (sizeof(packets) / sizeof(*packets));
		if (amt > 0) {
			for (int i = 0; i < amt; i++) {
				//Handle Messages
				std::string dataa = (char*)packets[i]->data;
				std::string data = dataa.substr(0, packets[i]->len);
				totalBytesRecv += data.size();
				totalPacketsRecv += 1;
				timedBytesRecv += data.size();
				timedPacketsRecv += 1;
				ClientMessage cm;
				cm.ip = packets[i]->address;
				cm.message = data;
				msgs.push_back(cm);
				if (data == MESSAGE_PING)
					sendMessage(MESSAGE_PEW, packets[i]->address);
				else if (data == MESSAGE_PEW) {
					std::chrono::time_point<std::chrono::system_clock> end = std::chrono::high_resolution_clock::now();
					std::chrono::nanoseconds dur = end - pingTimer;
					ping = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
					if (DEBUG)
						std::cout << "PING: " << ping << std::endl;
				}
				if (isServer) {
					if (data == MESSAGE_JOIN) {
						if (!isIPRegistered(packets[i]->address)) {
							if (clientCount < MAX_CLIENTS) {
								Client* client = new Client;
								client->ip = packets[i]->address;
								clients.push_back(client);
								clientCount++;
								std::cout << "Player connected! " << clientCount << " players connected!" << std::endl;
							}
							else {
								sendMessage(MESSAGE_FULL, packets[i]->address);
							}
						}
						else {
							sendMessage(MESSAGE_ALREADYJOINED, packets[i]->address);
						}
					}
					else if (data == MESSAGE_DISCONNECTED) {
						kickIP(packets[i]->address);
					}
				}
				if (DEBUG)
					std::cout << "Incoming Packet with data: " << data << std::endl << "Total Packets: " << totalPacketsRecv << ". Total Bytes: " << totalBytesRecv << std::endl;
			}
		}
		
		SDLNet_FreePacketV(packets);
		packets = NULL;
		return msgs;
	}
}

void Network::sendMessage(std::string message, IPaddress _ip) {
	if (hasInit) {
		totalBytesSent += message.size();
		totalBytesSent += 1;
		timedBytesSent += message.size();
		timedBytesSent += 1;
		
		if(packet == NULL)
			packet = SDLNet_AllocPacket(PACKET_SIZE);

		packet->address.host = _ip.host;
		packet->address.port = _ip.port;

		packet->data = (Uint8*)message.c_str();
		packet->len = message.length();

		if (!SDLNet_UDP_Send(UDPSocket, -1, packet))
			std::cout << "Error: Network::sendMessage: " << SDLNet_GetError() << std::endl;
	}
}

UDPpacket* Network::allocPacket(int32_t size) {
	UDPpacket* pack = SDLNet_AllocPacket(size);
	if (pack == nullptr)
		std::cout << "Error: Network::allocPacket: " << SDLNet_GetError() << std::endl;
	return pack;
}

UDPpacket** Network::allocPackets(int32_t amt, int32_t size) {
	UDPpacket** pack = SDLNet_AllocPacketV(amt, size);
	if(pack == nullptr)
		std::cout << "Error: Network::allocPackets: " << SDLNet_GetError() << std::endl;
	return pack;
}

void Network::refreshIP() {
	if (SDLNet_ResolveHost(&ip, serverName.c_str(), PORT) == -1) {
		std::cout << "Error: Network::refreshIP: " << SDLNet_GetError() << std::endl;
	}
}

void Network::refreshPing(IPaddress ip) {
	if (hasInit) {
		sendMessage(MESSAGE_PING, ip);
		pingTimer = std::chrono::high_resolution_clock::now();
	}
}

bool Network::isIPRegistered(IPaddress ip) {
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i]->ip.host == ip.host && clients[i]->ip.port == ip.port) {
			return true;
		}
	}
	return false;
}

int Network::getIPLoc(IPaddress ip) {
	for (int i = 0; i < clients.size(); i++)
		if (clients[i]->ip.host == ip.host && clients[i]->ip.port == ip.port)
			return i;
	return -1;
}

void Network::kickIP(IPaddress ip) {
	if (isIPRegistered(ip)) {
		int i = getIPLoc(ip);
		clients.erase(clients.begin() + i);
		clientCount--;
		std::cout << "Player disconnected! " << clientCount << " players connected!" << std::endl;
	}
}

IPaddress Network::getIP()
{
	return ip;
}

long long Network::getPing()
{
	return ping;
}

void Network::setServerName(std::string name)
{
	serverName = name;
}

std::string Network::getServerName()
{
	return serverName;
}

void Network::setServerPort(short port)
{
	PORT = port;
}

short Network::getServerPort()
{
	return PORT;
}

void Network::setIsServer(bool isIt)
{
	forceServer = true;
	isServer = isIt;
}

bool Network::getIsServer()
{
	return isServer;
}

int Network::getPacketSize()
{
	return PACKET_SIZE;
}

void Network::setPacketAmt(int amt)
{
	PACKET_AMT = amt;
}

int Network::getPacketAmt()
{
	return PACKET_AMT;
}

void Network::setDebugMode(bool isIt)
{
	DEBUG = isIt;
}

bool Network::getDebugMode()
{
	return DEBUG;
}

void Network::setMaxClients(int amt)
{
	if(amt > clientCount)
		MAX_CLIENTS = amt;
}

int Network::getMaxClients()
{
	return MAX_CLIENTS;
}

int Network::getClientCount()
{
	return clientCount;
}

int Network::getTotalPacketsSent()
{
	return totalPacketsSent;
}

int Network::getTotalPacketsRecv()
{
	return totalPacketsRecv;
}

int Network::getTotalBitsSent()
{
	return totalBytesSent;
}

int Network::getTotalBitsRecv()
{
	return totalBytesRecv;
}

int Network::getTimedPacketsSent()
{
	return lastPacketsSent;
}

int Network::getTimedPacketsRecv()
{
	return lastPacketsRecv;
}

int Network::getTimedBitsSent()
{
	return lastBytesSent;
}

int Network::getTimedBitsRecv()
{
	return lastBytesRecv;
}

std::string Network::getDataFromClientMessage(ClientMessage cm)
{
	return cm.message;
}

IPaddress Network::getIPFromClientMessage(ClientMessage cm)
{
	return cm.ip;
}
