/* Server 3.0.cpp 

Kommentar:  Oppgaven består av et prosjekt for serveren (dvs dette prosjektet), og et prosjekt
			for klienten. Serveren bruker winsocket, leter etter klienter, deretter oppretter en tråd.
			Tråden kjører server-chat`en, som er "serverProcess" funksjonen nedenfor, som gir serveren
			mulighet for kommunikasjon. Deretter åpner serveren for å akseptere diverse klienter fram
			til max antall klienter. Hver klient får en tråd som kjører funksjonen "clientProcess".
			Denne funksjonen sender først en melding til alle klienter oppkoblet (inkludert denne 
			klienten). Meldingen forteller om at en ny klient har koblet til og hvor mange som er 
			pålogget. Deretter venter den på nye meldinger via Recv(), og sender dem til alle klientene
			når meldinger kommer inn (evt feilmelding).
*/

#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "127.0.0.1"
#define DEFAULT_PORT "25565"
#define DEFAULT_BUFFER 512

struct client_type { //alle brukere skal ha en id og en socket
	int id;
	SOCKET socket;
};

//Settings
const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 10;
int NUMBER_OF_CLIENTS = 0;

//Deklarerer funksjonene
int clientProcess(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread);
int serverProcess(std::vector<client_type> &client_array);
int main();

int clientProcess(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread) //Funksjon for brukeren
{
	NUMBER_OF_CLIENTS++;
	std::string msg = "";
	char msgRecv[DEFAULT_BUFFER] = "";

	//Informer brukerne om at en ny klient har logget på.
	std::string msg2 = "A new client just connected. Total clients in the system: " + std::to_string(NUMBER_OF_CLIENTS);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_array[i].socket != INVALID_SOCKET) {
			int result = send(client_array[i].socket, msg2.c_str(), strlen(msg2.c_str()), 0);
		}
	}

	//Session 
	while (true)
	{
		memset(msgRecv, 0, DEFAULT_BUFFER); //funksjon som returnerer adressen til en string

		if (new_client.socket != 0) //Hvis clientens socket ikke er lik 0
		{
			int result = recv(new_client.socket, msgRecv, DEFAULT_BUFFER, 0); // result = antall bit mottatt

			if (result != SOCKET_ERROR) //hvis recv() ikke gir feil 
			{
				if (strcmp("", msgRecv))  //Sammenligner "" og  meldingen som er motatt.
				{
					msg = "User #" + std::to_string(new_client.id) + ": " + msgRecv; // printer ut brukerens id og meldingen som er motatt.
				}
				std::cout << msg.c_str() << std::endl; //serveren skriver ut meldingen i sin utprint


													   //Send meldingen til de andre brukerene
				for (int i = 0; i < MAX_CLIENTS; i++) //send til alle
				{
					if (client_array[i].socket != INVALID_SOCKET) {
						if (new_client.id != i) { //uten om den som sendte meldingen
							result = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
						}
					}
				}
			}
			else // SOCKET_ERROR
			{
				//Informer at en bruker har avsluttet. 
				NUMBER_OF_CLIENTS--;
				msg = "User #" + std::to_string(new_client.id) + " Disconnected. Total clients in the system: " + std::to_string(NUMBER_OF_CLIENTS);

				std::cout << msg << std::endl;

				closesocket(new_client.socket); //close socket
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;

				//Send meldingen til de andre brukerne
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET) {
						result = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
					}
				}
				break;
			}

		}

	} //end while

	thread.detach();

	return 0;
}

//Server kan nå sende meldinger til brukerne!
int serverProcess(std::vector<client_type> &client_array) {
	std::string msg = "";
	std::string msgIn = "";

	while (true) {
		getline(std::cin, msgIn);
		if (msgIn != "") {

			//broadcast til brukerne
			msg = "Server: " + msgIn;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_array[i].socket != INVALID_SOCKET) {
					int result = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);

					if (result == SOCKET_ERROR) {
						std::cout << "Message didn't get sent" << std::endl;
						std::cout << WSAGetLastError << std::endl;
					}
				}
			}
		}
	}
	return 0;
}


int main()
{
	WSADATA wsaData;
	struct addrinfo adressinfo;
	struct addrinfo *server = NULL;
	SOCKET server_socket = INVALID_SOCKET;

	std::string msg = "";
	std::vector<client_type> client(MAX_CLIENTS); //bruker vektor til å lagre id og socket for hver tråd (bruker)
	int num_clients = 0;
	int temp_id = -1;
	std::thread my_thread[MAX_CLIENTS]; //legg til en tråd per bruker
	std::thread w; //og en tråd for å skrive ut meldinger fra serveren

								//Start Winsock
	std::cout << "Intializing Winsock..." << std::endl;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Definer ipv4, protocoler etc
	ZeroMemory(&adressinfo, sizeof(adressinfo));
	adressinfo.ai_family = AF_INET; //bruk ipv4
	adressinfo.ai_socktype = SOCK_STREAM;
	adressinfo.ai_protocol = IPPROTO_TCP; //TCP protocol
	adressinfo.ai_flags = AI_PASSIVE;

	//Start Server
	std::cout << "Setting up server..." << std::endl;
	getaddrinfo(static_cast<LPCSTR>(IP_ADDRESS), DEFAULT_PORT, &adressinfo, &server);

	//Listening Socket
	std::cout << "Creating server socket..." << std::endl;
	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//socket muligheter:
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //re-bind til en port som har blitt brukt i løpet av de 2 siste minuttene
	setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //For "konstant" kommunikasjon mellom server og client

																					 //Gi server socket`en en adresse
	std::cout << "Binding socket..." << std::endl;
	bind(server_socket, server->ai_addr, (int)server->ai_addrlen);

	//"listen" for innkommende brukere
	std::cout << "Listening..." << std::endl;
	listen(server_socket, SOMAXCONN);

	//Lag tråd for server:
	w = std::thread(serverProcess, std::ref(client));

	//client_list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client[i] = { -1, INVALID_SOCKET }; //id to all clients are set to -1, and socket is invalid
	}

	while (true)
	{
		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(server_socket, NULL, NULL);

		if (incoming == INVALID_SOCKET) continue; // continue = resten av operasjonene vil ikke bli kjørt

		num_clients = -1;
		//Bruker får en midlertidig id 
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (client[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				client[i].socket = incoming;
				client[i].id = i;
				temp_id = i;
			}

			if (client[i].socket != INVALID_SOCKET) //legg til brukeren så lenge socket ikke gir noen feil
				num_clients++;
		}
		if (temp_id != -1)
		{
			//Send id`en til brukeren
			std::cout << "User #" << client[temp_id].id << " Accepted" << std::endl;
			msg = std::to_string(client[temp_id].id);
			send(client[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			//Opprett en tråd for brukeren
			my_thread[temp_id] = std::thread(clientProcess, std::ref(client[temp_id]), std::ref(client), std::ref(my_thread[temp_id]));

		}
		else  //siden alle id`ene er satt til -1, så vil serveren være full om det ikke er flere id`er som er -1.
		{
			msg = "Server is full";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	} //end while


	  //Avslutt listening socket
	closesocket(server_socket);

	//Avslutt client socket
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(client[i].socket);
	}

	WSACleanup();
	std::cout << "Program has ended successfully" << std::endl;

	system("pause");

	return 0;
}

