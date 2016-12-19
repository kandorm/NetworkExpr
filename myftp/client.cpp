#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h>   //htons
#include <sys/socket.h>  //inet_pton
#include <string.h>      //memset  string
#include <iostream>      //cout
#include <stdlib.h>      //exit
#include <stdio.h>		 //stdin fread
#include <unistd.h>  //getcwd  close(socket)

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
		exit(0);
	}
	std::cout << "Connect to " << argv[1] << " " << argv[2] << " successfully!" << std::endl;

	//get file path
	char pathBuffer[MAX_SIZE];
	getcwd(pathBuffer, sizeof(pathBuffer));
	std::string path = std::string(pathBuffer);

	//**************************input command******************************************
	while (true) {
		//---------------------send command----------------------------------------
		std::cout << "Input Command:" << std::endl;
		char sendBuffer[MAX_SIZE];
		memset(sendBuffer, '\0', sizeof(sendBuffer));
		fgets(sendBuffer, sizeof(sendBuffer), stdin);
		sendResponse(commandSocket, std::string(sendBuffer));
		std::cout << "Send command successfully!" << std::endl;
		//---------------------receive response------------------------------------
		char recvBuffer[MAX_SIZE];
		std::string recvMsg = "";
		int recvLength = 0;
		memset(recvBuffer, '\0', sizeof(recvBuffer));
		/*
		while ((recvLength = recv(commandSocket, recvBuffer, sizeof(recvBuffer), 0)) > 0) {
			recvMsg += recvBuffer;
			memset(recvBuffer, '\0', sizeof(recvBuffer));
		}
		*/
		recvLength = recv(commandSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLength < 0) {
			std::cout << "Receive server response error!" << std::endl;
			std::cout << "Please input command again!" << std::endl;
			continue;
		}
		recvMsg += recvBuffer;
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
		else if(strcmp(recvMsg.substr(0, 3).c_str(), "put") == 0) {
			std::cout << recvMsg << std::endl;
			int servDtaPort = atoi(recvMsg.substr(4).c_str());
			//----------------open file failed--------------------------
			if(servDtaPort == 0) {
				std::cout << "Remote create file failed!" << std::endl;
				continue;
			}
			//-----------------create data socket--------------------------
			int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
			if(dataSocket == SOCKET_ERROR) {
				std::cout << "Create data socket failed!" << std::endl;
				exit(0);
			}

			struct sockaddr_in servDtaAddr;
			memset(&servDtaAddr, 0, sizeof(servDtaAddr));
			servDtaAddr.sin_family = AF_INET;
			servDtaAddr.sin_port = htons(servDtaPort);
			servDtaAddr.sin_addr = serverAddr.sin_addr;

			if ((ret = connect(dataSocket, (struct sockaddr*)&servDtaAddr, sizeof(servDtaAddr))) == -1) {
				std::cout << "Data socket connect to server failed!" << std::endl;
				exit(0);
			}
			std::cout << "Connect to " << argv[1] << " " << servDtaPort << " successfully!" << std::endl;
			//---------------------read file-------------------------------------
			if(path[path.size()-1] != '/') {
				path += '/';
			}
			std::string filePath = path + std::string(sendBuffer).substr(4);
			if(filePath[filePath.size()-1] == '\n') {
				filePath.erase(filePath.size()-1);
			}
			FILE* fin = fopen(filePath.c_str(), "rb");
			std::string dataResponse = "";
			if(fin == NULL) {
				std::cout << "File is not exit!" << std::endl;
				sendResponse(dataSocket, dataResponse);
			}
			else {
				dataResponse = "";
				char dataBuffer[MAX_SIZE];
				memset(dataBuffer, '\0', sizeof(dataBuffer));
				int length = 0;
				while( (length = fread(dataBuffer, sizeof(char), MAX_SIZE, fin)) > 0) {
					dataResponse += dataBuffer;
					memset(dataBuffer, '\0', sizeof(dataBuffer));
				}
				fclose(fin);
				std::cout << "dataResponse " << dataResponse << std::endl;
				sendResponse(dataSocket, dataResponse);
			}
			close(dataSocket);
		}
		else if(strcmp(recvMsg.substr(0, 3).c_str(), "get") == 0) {
			std::cout << recvMsg << std::endl;
			int servDtaPort = atoi(recvMsg.substr(4).c_str());
			//----------------open file failed--------------------------
			if(servDtaPort == 0) {
				std::cout << "File is not exit!" << std::endl;
				continue;
			}
			//-----------------create data socket--------------------------
			int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
			if(dataSocket == SOCKET_ERROR) {
				std::cout << "Create data socket failed!" << std::endl;
				exit(0);
			}

			struct sockaddr_in servDtaAddr;
			memset(&servDtaAddr, 0, sizeof(servDtaAddr));
			servDtaAddr.sin_family = AF_INET;
			servDtaAddr.sin_port = htons(servDtaPort);
			servDtaAddr.sin_addr = serverAddr.sin_addr;

			if ((ret = connect(dataSocket, (struct sockaddr*)&servDtaAddr, sizeof(servDtaAddr))) == -1) {
				std::cout << "Data socket connect to server failed!" << std::endl;
				exit(0);
			}
			std::cout << "Connect to " << argv[1] << " " << servDtaPort << " successfully!" << std::endl;
			//---------------------write file-------------------------------------
			if(path[path.size()-1] != '/') {
				path += '/';
			}
			std::string filePath = path + std::string(sendBuffer).substr(4);
			if(filePath[filePath.size()-1] == '\n') {
				filePath.erase(filePath.size()-1);
			}
			FILE* fout = fopen(filePath.c_str(), "wb");
			int length = 0;
			char dataBuffer[MAX_SIZE];
			memset(dataBuffer, '\0', sizeof(dataBuffer));
			std::string data = "";
			while((length = recv(dataSocket, dataBuffer, MAX_SIZE, 0)) > 0) {
				data += dataBuffer;
				fwrite(dataBuffer, sizeof(char), length, fout);
				memset(dataBuffer, '\0', sizeof(dataBuffer));
			}
			fclose(fout);
			std::cout << "Data " << data << std::endl;

			close(dataSocket);
		}
		else {
			std::cout << recvMsg << std::endl;
		}
	}
	close(commandSocket);


}
