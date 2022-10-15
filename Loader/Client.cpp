#include <iostream>
#include <string>

#include "Client.h"
#include "Game.h"

#pragma comment(lib, "ws2_32.lib")

bool Client::bot_running;
bool Client::client_running;
SOCKET Client::sock;
WSADATA Client::WSAData;

char* subchar(char* txt, int begin, int end) {
	char* res = new char[end-begin];
	for (int i = 0; i < end-begin; i++) {
		res[i] = *(txt + begin + i);
	}
	res[end-begin] = '\0';
	return res;
}

void Client::sendMessage(const char* msg) {
	//Send datasize
	send(sock, std::to_string(strlen(msg)).c_str(), sizeof(strlen(msg)), 0);
	//Send message
	send(sock, msg, strlen(msg), 0);
}

void Client::recvMessage() {
	char buffer[1024];
	recv(sock, buffer, sizeof(buffer), 0);
	if (strstr(buffer, "Bot:")) {
		char* buffTmp = subchar(buffer, 5, 7);
		if (strcmp(buffTmp, "ON") == 0) {
			std::cout << "ON activated" << "\n";
			bot_running = true;
		} else if(strcmp(buffTmp, "OFF")) {
			std::cout << "OFF activated" << "\n";
			bot_running = false;
		}
		std::cout << "Bot running: " << bot_running << "\n";
	}
}

void Client::ConnectToServer(const char* addr, int port) {
	if (WSAStartup(MAKEWORD(2, 0), &WSAData) == 0) {
		SOCKADDR_IN sin;
		sin.sin_addr.s_addr = inet_addr(addr);
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		
		std::cout << "Connecting...\n";
		connect(sock, (SOCKADDR*)&sin, sizeof(sin));
		std::cout << "Connected to " << "127.0.0.1" << ":" << port << "\n";
	}
}

void Client::DisconnectClient() {
	closesocket(sock);
	WSACleanup();
	std::cout << "Client disconnected !\n";
}

DWORD WINAPI Client::MakeLoop(void* data) {
	//Read messages from server on a separate thread then do something
	ConnectToServer("127.0.0.1", 50001);
	while (client_running == true) {
		recvMessage();
	}
	DisconnectClient();
	return 0;
}