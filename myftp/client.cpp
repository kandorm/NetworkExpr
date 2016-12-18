#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h>   //htons
#include <sys/socket.h>  //inet_pton
#include <string.h>      //memset  string
#include <iostream>      //cout
#include <errno.h>

#define MAX_SIZE 4096
#define SOCKET_ERROR -1

void sendResponse(int commandSocket, std::string response) {
	int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
	if(sendLength != strlen(response.c_str())) {
		std::cout << "Response to client failed!" << std::endl;
		exit(0);
	}
}

int main(int argc, char** argv) {
	//********************************parameter check***********************************
	if (argc != 3) {
		std::cout << "Usage:client <address> <port>" << std::endl;
		exit(0);
	}
	//********************************set server address********************************
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &serverAddr.sin_addr) == 0) {
		std::cout << "IP Address error" << std::endl;
		exit(0);
	}
	//*****************************set command socket***********************************
	int commandSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (commandSocket < 0) {
		std::cout << "Create a command socket failed!" << std::endl;
		exit(0);
	}
	//**************************connect with remote server*****************************
	int ret; //connect error number
	if ((ret = connect(commandSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		std::cout << "Command socket connect to server failed!" << std::endl;
		return 0;
	}
	std::cout << "Connect to" << argv[1] << " " << argv[2] << "successfully!" << std::endl;
	//**************************input command******************************************
	while (true) {
		//---------------------send command----------------------------------------
		std::cout << "Input Command:" << std::endl;
		char sendBuffer[MAX_SIZE];
		memset(sendBuffer, '\0', sizeof(sendBuffer));
		fgets(sendBuffer, sizeof(sendBuffer), stdin);
		sendResponse(commandSocket, std::string(sendBuffer));
		//---------------------receive response------------------------------------
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
		//---------------------action to server's message-------------------------
		if(strcmp(recvMsg.substr(0, 1).c_str(), "?") == 0) {
			std::cout << recvMsg.substr(4) << std::endl;
		}
		else if(strcmp(recvMsg.substr(0, 2).c_str(), "cd") == 0) {
			std::cout << recvMsg.substr(4) << std::endl;
		}
		else if(strcmp(recvMsg.substr(0, 3).c_str(), "dir") == 0) {
			std::cout << recvMsg.substr(4) << std::endl;
		}
		else if(strcmp(recvMsg.substr(0, 3).c_str(), "pwd") == 0) {
			std::cout << recvMsg.substr(4) << std::endl;
		}
		else if(strcmp(recvMsg.substr(0, 4).c_str(), "quit") == 0) {
			std::cout << recvMsg.substr(4) << std::endl;
			break;
		}
		else {
			std::cout << recvMsg << std::endl;
		}
	}
	close(commandSocket);


}
