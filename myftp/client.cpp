#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h>   //htons
#include <sys/socket.h>  //inet_pton
#include <string.h> //memset
#include <iostream>      //cout
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define MAX_SIZE 4096

int main(int argc, char** argv) {
	//input check
	if (argc != 4) {
		std::cout << "Usage:client <address> <port> <filename>" << std::endl;
		exit(0);
	}
	//set socket address
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &serverAddr.sin_addr) == 0) {
		std::cout << "IP Address error" << std::endl;
		return 0;
	}

	//create socket
	int commandSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (commandSocket < 0) {
		std::cout << "File to create a command socket!" << std::endl;
		return 0;
	}

	int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (dataSocket < 0) {
		std::cout << "Fail to create a data socket!" << std::endl;
		return 0;
	}

	//connect with remote server
	int ret; //connect error number
	if ((ret = connect(commandSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		std::cout << "Command socket connect to server failed!" << std::endl;
		return 0;
	}

	if ((ret = connect(dataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		std::cout << "Data socket connect to server failed!" << std::endl;
		return 0;
	}

	while (true) {
		//input command
		std::cout << "Input Command:" << std::endl;
		char sendBuffer[MAX_SIZE];
		memset(sendBuffer, '\0', sizeof(sendBuffer));
		fgets(sendBuffer, sizeof(sendBuffer), stdin);
		int sendLength = send(commandSocket, sendBuffer, strlen(sendBuffer), 0);
		if (sendLength != strlen(sendBuffer)) {
			std::cout << "Send command to server failed!" << std::endl;
			std::cout << "Please input command again!" << std::endl;
			continue;
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
			std::cout << "Receive server response error!" << std::endl;
			std::cout << "Please input command again!" << std::endl;
			continue;
		}

	}


}
