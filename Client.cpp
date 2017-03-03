#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "84.202.185.154"
#define DEFAULT_PORT "25565"
#define DEFAULT_BUFFER 512

struct client_type //bruker struktur: socket, id, buffer 
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFFER];
};

//Deklarerer funksjoner
int clientProcess(client_type &new_client);
int main();

int clientProcess(client_type &new_client)
{
	while (true)
	{
		memset(new_client.received_message, 0, DEFAULT_BUFFER); //funksjon som returnerer adressen til en string

		if (new_client.socket != 0) 
		{
			int result = recv(new_client.socket, new_client.received_message, DEFAULT_BUFFER, 0); // result = antall bit mottatt
			
			if (result != SOCKET_ERROR)
				cout << new_client.received_message << endl; //skriv ut motatte meldinger
			else
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET) 
		cout << "The server has disconnected." << endl;

	return 0;
}

int main()
{
	WSAData wsaData;
	struct addrinfo *value = NULL, *ptr = NULL, adressinfo;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "" };
	int result = 0;
	string message;

	cout << "Starting Client...\n";

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		cout << "WSAStartup() failed with error: " << result << endl;
		return 1;
	}

	ZeroMemory(&adressinfo, sizeof(adressinfo));
	adressinfo.ai_family = AF_UNSPEC;
	adressinfo.ai_socktype = SOCK_STREAM;
	adressinfo.ai_protocol = IPPROTO_TCP;

	cout << "Connecting...\n";

	// Server addresse og port
	result = getaddrinfo(static_cast<LPCSTR>(IP_ADDRESS), DEFAULT_PORT, &adressinfo, &value);
	if (result != 0) {
		cout << "getaddrinfo() failed with error: " << result << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	for (ptr = value; ptr != NULL; ptr = ptr->ai_next) {

		// Opprett en SOCKET for oppkobling mot server
		client.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) {
			cout << "socket() failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// bruker connect()
		result = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (result == SOCKET_ERROR) {
			closesocket(client.socket); 
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(value);

	if (client.socket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Successfully Connected" << endl;

	//Skaff id til server for denne klienten;
	recv(client.socket, client.received_message, DEFAULT_BUFFER, 0);
	message = client.received_message;

	if (message != "Server is full")
	{
		client.id = atoi(client.received_message);

		thread my_thread(clientProcess, client);

		//send melding til server
		while (true)
		{
			getline(cin, sent_message);
			result = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (result <= 0)
			{
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		} //end while

		//Avslutt tråd
		my_thread.detach();
	}
	else
		cout << client.received_message << endl;

	cout << "Shutting down socket..." << endl;
	result = shutdown(client.socket, SD_SEND);
	if (result == SOCKET_ERROR) {
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}