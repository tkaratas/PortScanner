// PortScanner - A simple port scanner

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool isOpen(const string& ip, int port) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		cerr << "Error creating socket: " << WSAGetLastError() << endl;
		return false;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	int result = connect(sock, (sockaddr*)&addr, sizeof(addr));
	closesocket(sock);
	return result == 0;
}

int main() {
	WSADATA wsaData;
	int wsInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsInit != 0) {
		cerr << "WSAStartup failed: " << wsInit << endl;
		return 1;
	}

	string targetIP;
	int startPort;
	int endPort;

	cout << "Enter target IP address: ";
	cin >> targetIP;
	cout << "Enter start port: ";
	cin >> startPort;
	cout << "Enter end port: ";
	cin >> endPort;

	for (int port = startPort; port <= endPort; ++port) {
		if (isOpen(targetIP, port)) {
			cout << "Port " << port << " is open." << endl;
		}
		else {
			cout << "Port " << port << " is closed." << endl;
		}
	}

	WSACleanup();
	return 0;
}