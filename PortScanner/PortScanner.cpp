// PortScanner - A simple port scanner

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

mutex cout_mutex;
mutex file_mutex;
ofstream logFile;

bool loggingEnabled = false;

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

void logResult(const string& message) {
	lock_guard<mutex> lock(file_mutex);
	if (loggingEnabled && logFile.is_open()) {
		logFile << message << endl;
	}
}

string getServiceName(int port) {
	switch (port) {
	case 21: return "FTP";
	case 22: return "SSH";
	case 23: return "Telnet";
	case 25: return "SMTP";
	case 53: return "DNS";
	case 80: return "HTTP";
	case 110: return "POP3";
	case 143: return "IMAP";
	case 443: return "HTTPS";
	default: return "Unknown";
	}
}

void scanPort(const string& ip, int port) {
	if (isOpen(ip, port)) {
		lock_guard<mutex> lock(cout_mutex);
		string message = to_string(port) + "\t" + getServiceName(port) + "\t\tOpen";
		cout << port << "\t" << getServiceName(port) << "\t\t" << "Open" << endl;
		logResult(message);
	}
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
	string logPath;

	cout << "Enter target IP address: ";
	cin >> targetIP;
	cout << "Enter start port: ";
	cin >> startPort;
	cout << "Enter end port: ";
	cin >> endPort;

	cout << "Enter path to log file (or press Enter to skip logging): ";
	cin.ignore(); // flush newline left by previous input
	getline(cin, logPath);

	if (!logPath.empty()) {
		logFile.open(logPath);
		if (!logFile) {
			cerr << "Could not open log file! Logging disabled." << endl;
		}
		else {
			loggingEnabled = true;
		}
	}

	cout << "Port \t Service Name" << endl;

	vector<thread> threads;

	for (int port = startPort; port <= endPort; ++port) {
		threads.emplace_back(scanPort, targetIP, port);

		// Limit threads to avoid overload
		if (threads.size() >= 100) {
			for (auto& t : threads) {
				t.join();
			}
			threads.clear();
		}
	}

	for (auto& t : threads) {
		t.join();
	}

	if (logFile.is_open()) {
		logFile.close();
	}

	WSACleanup();
	return 0;
}