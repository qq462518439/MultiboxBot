#include <iostream>
#include <string>

#include "Client.h"
#include "Game.h"
#include "Functions.h"
#include "MemoryManager.h"

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

void Client::sendMessage(std::string msg) {
	//Send message
	send(sock, msg.c_str(), msg.length(), 0);
}

void Client::recvMessage() {
	char buffer[128];
	recv(sock, buffer, sizeof(buffer), 0);
	if (buffer[0] == 'B') {
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
	else if (buffer[0] == 'S') {
		char* buffTmp = subchar(buffer, 6, 7);
		playerSpec = ((int)buffTmp[0] - '0');
	}
	else if (buffer[0] == 'P') {
		char* buffTmp = subchar(buffer, 5, 6);
		positionCircle = ((int)buffTmp[0] - '0');
	}
	else if (buffer[0] == 'T') {
		char* buffTmp = subchar(buffer, 6, 25);
		int y = 0;
		for (unsigned int i = 0; i < strlen(buffTmp); i++) {
			if ((buffTmp[i] >= 'a' && buffTmp[i] <= 'z') || (buffTmp[i] >= 'A' && buffTmp[i] <= 'Z')) y++;
		}
		tankName = subchar(buffTmp, 0, y);
	}
	else if (buffer[0] == 'M') {
		char* buffTmp = subchar(buffer, 7, 25);
		int y = 0;
		for (unsigned int i = 0; i < strlen(buffTmp); i++) {
			if ((buffTmp[i] >= 'a' && buffTmp[i] <= 'z') || (buffTmp[i] >= 'A' && buffTmp[i] <= 'Z')) y++;
		}
		meleeName = subchar(buffTmp, 0, y);
	}
	else if (buffer[0] == 'K' && buffer[1] == '1') {
		keyHearthstone = true;
	}
	else if (buffer[0] == 'K' && buffer[1] == '2') {
		keyMount = true;
	}
	else if (buffer[0] == 'K' && buffer[1] == '3') {
		keyTarget = true;
	}
	else if (buffer[0] == 'C' && buffer[1] == '1') {
		//Tank auto focus
		tankAutoFocus = ((int)buffer[2] - '0');
	}
	else if (buffer[0] == 'C' && buffer[1] == '2') {
		//Tank auto move
		tankAutoMove = ((int)buffer[2] - '0');
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