#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h> //htons
#include <sys/socket.h> //inet_pton
#include <string.h> //memset string
#include <iostream>  //cout
#include <unistd.h>  //getcwd
#include <stdlib.h>

#define MAX_SIZE 4096
#define HOST_PORT 21
#define DATA_SEND_PORT 20
#define BACKLOG 10
#define SOCKET_ERROR -1

int main(int argc, char** argv) {

	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == SOCKET_ERROR) {
		std::cout << "Create server socket failed!" << std::endl;
		exit(0);
	}

	//set host address
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(HOST_PORT);

	//bind socket and address
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Bind to local host failed!" << std::endl;
		exit(0);
	}

	if (listen(listenSocket, BACKLOG) == SOCKET_ERROR) {
		std::cout << "Error listening on socket.\n" << std::endl;
		exit(0);
	}

	while (true) {
		struct sockaddr_in clientAddr;
		socklen_t addrLength = sizeof(clientAddr);
		int commandSocket;
		if ((commandSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLength)) == SOCKET_ERROR) {
			std::cout << "Accept command socket failed!" << std::endl;
			exit(0);
		}



		char recvBuffer[MAX_SIZE];
		std::string recvMsg = "";
		int recvLength = 0;
		memset(recvBuffer, '\0', sizeof(recvBuffer));
		while ((recvLength = recv(commandSocket, recvBuffer, sizeof(recvBuffer), 0)) > 0) {
			recvMsg += std::string(recvBuffer);
			memset(recvBuffer, '\0', sizeof(recvBuffer));
		}
		if (recvLength < 0) {
			std::cout << "Receive client command error!" << std::endl;
			continue;
		}
		std::cout << "Receive command : " << recvBuffer << std::endl;

		if (strcmp(recvMsg.substr(0, 3).c_str(), "get") == 0) {
			std::string fileName = recvMsg.substr(4);
			char path[MAX_SIZE];
			getcwd(path, sizeof(path));
			std::string filePath = std::string(path) + "/" + fileName;
			std::cout << "get " << filePath << std::endl;
			FILE* fin = fopen(filePath.c_str(), "rb");
			if (fin == NULL) {
				std::string response = "get fail";
				int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
				if (sendLength != strlen(response.c_str())) {
					std::cout << "Response get to client failed!" << std::endl;
					exit(0);
				}
			}
			else {
				std::string response = "get ";
				response += fileName;
				int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
				if (sendLength != strlen(response.c_str())) {
					std::cout << "Response get to client failed!" << std::endl;
					exit(0);
				}

				std::string data = "";
				char fbuffer[MAX_SIZE];
				while(fread(fbuffer, sizeof(char), MAX_SIZE, fin) != 0) {
					data += std::string(fbuffer);
				}
				fclose(fin);

				std::cout << "Data of file : " << data << std::endl;
				int dataLength = send(dataSocket, data.c_str(), strlen(data.c_str()), 0);
				if (dataLength != strlen(data.c_str())) {
					std::cout << "Response get to client failed!" << std::endl;
					exit(0);
				}
				std::cout << "get file successful!" << std::endl;

			}
		}
		else if(strcmp(recvMsg.substr(0, 3).c_str(), "put") == 0) {
			std::string fileName = recvMsg.substr(4);
			char path[MAX_SIZE];
			getcwd(path, sizeof(path));
			std::string filePath = std::string(path) + "/" + fileName;
			std::cout << "get " << filePath << std::endl;
			FILE* fin = fopen(filePath.c_str(), "rb");

			std::string response = recvMsg;;
			int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
			if (sendLength != strlen(response.c_str())) {
				std::cout << "Response get to client failed!" << std::endl;
				exit(0);
			}

			std::string data = "";
			char fbuffer[MAX_SIZE];
			while(fread(fbuffer, sizeof(char), MAX_SIZE, fin) != 0) {
				data += std::string(fbuffer);
			}
			fclose(fin);

			std::cout << "Data of file : " << data << std::endl;
			int dataLength = send(commandSocket, data.c_str(), strlen(data.c_str()), 0);
			if (dataLength != strlen(data.c_str())) {
				std::cout << "Response get to client failed!" << std::endl;
				exit(0);
			}
			std::cout << "get file successful!" << std::endl;

		}
	}




}
