#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h> //htons
#include <sys/socket.h> //inet_pton
#include <string.h> //memset
#include <errno.h>
#include <iostream> //cout
#include <stdlib.h>

#define MAX_SIZE 4096
#define HOST_PORT 21
#define DATA_SEND_PORT 20
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

	if (listen(listenSocket, DATA_SEND_PORT) == SOCKET_ERROR) {
		std::cout << "Error listening on socket.\n" << std::endl;
		exit(0);
	}

	while (true) {
		int commandSocket;
		if ((commandSocket = accept(listenSocket, NULL, NULL)) == SOCKET_ERROR) {
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

		if (recvMsg.substr(0, 3).equals("get")) {
			std::string fileName = recvMsg.substr(4);
			char filePath[MAX_SIZE];
			std::string filePath = std::string(getcwd(filePath, sizeof(filePath))) + "/" + fileName;
			std::cout << "get " << filePath << std::endl;
			FILE fin* = fopen(filePath.c_str(), "rb");
			if (fin == NULL) {
				string response = "get fail";
				int sendLength = send(commnadSocket, response, strlen(response), 0);
				if (sendLength != strlen(response)) {
					std::cout << "Response get to client failed!" << std::endl;
					exit(0);
				}
			}
			else {
				string response
			}
		}
	}




}
