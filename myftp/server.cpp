#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h> //htons
#include <sys/socket.h> //inet_pton
#include <string.h> //memset string
#include <iostream>  //cout
#include <unistd.h>  //getcwd
#include <dirent.h> //DIR opendir  dirnet
#include <stdlib.h> //exit
#include <stdio.h>  //fopen

#define MAX_SIZE 4096
#define HOST_PORT 8000
#define DATA_TRAN_PORT 8001
#define BACKLOG 20         //waiting service number
#define SOCKET_ERROR -1

void sendResponse(int commandSocket, std::string response) {
	int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
	if(sendLength != strlen(response.c_str())) {
		std::cout << "Response to client failed!" << std::endl;
		exit(0);
	}
}
int main(int argc, char** argv) {

	//**********************set listening socket****************************
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
	std::cout << "Waiting for client connect!" << std::endl;

	//get file path
	char pathBuffer[MAX_SIZE];
	getcwd(pathBuffer, sizeof(pathBuffer));
	std::string path = std::string(pathBuffer);

	//*********************waiting for client connect************************
	while (true) {
		//-------------------client connect with server--------------------
		struct sockaddr_in clientAddr;
		socklen_t addrLength = sizeof(clientAddr);
		int commandSocket;
		if ((commandSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLength)) == SOCKET_ERROR) {
			std::cout << "Accept command socket failed!" << std::endl;
			exit(0);
		}
		std::cout << "Client connect successful!" << std::endl;
		while(true) {
			//-------------------receive client command------------------------
			char recvBuffer[MAX_SIZE];
			std::string recvMsg = "";
			int recvLength = 0;
			memset(recvBuffer, '\0', sizeof(recvBuffer));
			/*
			while ((recvLength = recv(commandSocket, recvBuffer, sizeof(recvBuffer), 0)) > 0) {
				recvMsg += std::string(recvBuffer);
				memset(recvBuffer, '\0', sizeof(recvBuffer));
			}
			*/
			recvLength = recv(commandSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLength < 0) {
				std::cout << "Receive client command error!" << std::endl;
				continue;
			}
			recvMsg += std::string(recvBuffer);
			std::cout << "Receive command : " << recvMsg << std::endl;

			//-------------------respone to command----------------------------
			if (strcmp(recvMsg.substr(0, 1).c_str(), "?") == 0) {
				std::string response = "?   ";
				response += "get [filename]\n";
				response += "put [filename]\n";
				response += "pwd\n";
				response += "dir\n";
				response += "cd\n";
				response += "?\n";
				response += "quit\n";

				sendResponse(commandSocket, response);
			}
			else if(strcmp(recvMsg.substr(0, 2).c_str(), "cd") == 0) {
				if(strcmp(recvMsg.substr(3, 2).c_str(), "..") == 0) {
					if(path[path.size()-1] == '/') {
						path.erase(path.size()-1);
					}
					int pos = path.rfind("/");
					if(pos > 0) {
						path.erase(pos);
					}
					std::string response = "cd  ";
					response += path;
					sendResponse(commandSocket, response);
				}
				else {
					std::string extraPath = recvMsg.substr(3);
					std::string tPath = path;
					if(tPath[tPath.size()-1] != '/') {
						tPath += "/";
					}
					tPath += extraPath;
					if(tPath[tPath.size()-1] == '\n') {
						tPath.erase(tPath.size()-1);
					}
					if(tPath[tPath.size()-1] == '/') {
						tPath.erase(tPath.size()-1);
					}

					DIR* dir = opendir(tPath.c_str());
					if(dir == NULL) {
						std::string response = "dir ";
						response += "no such dir";
						sendResponse(commandSocket, response);
					}
					else {
						path = tPath;
						std::string response = "dir ";
						response += path;
						sendResponse(commandSocket, response);
						closedir(dir);
					}
				}
			}
			else if(strcmp(recvMsg.substr(0, 3).c_str(), "dir") == 0) {
				if(path[path.size()-1] == '/') {
					path.erase(path.size()-1);
				}
				DIR* dir = opendir(path.c_str());
				std::string response = "dir ";
				if(dir == NULL) {
					std::cout << "Open dir failed!" << std::endl;
					exit(0);
				}
				else {
					struct dirent* fileName = NULL;
					while(fileName = readdir(dir)) {
						if(strcmp(fileName->d_name, ".") == 0 || strcmp(fileName->d_name, "..") == 0)
							continue;
						response += fileName->d_name;
						response += "\n";
					}
				}
				sendResponse(commandSocket, response);
				closedir(dir);
			}
			else if(strcmp(recvMsg.substr(0, 3).c_str(), "pwd") == 0) {
				std::string response = "pwd ";
				response += path;
				response += "\n";
				sendResponse(commandSocket, response);

			}
			else if(strcmp(recvMsg.substr(0, 4).c_str(), "quit") == 0) {
				std::string response = "quit";
				response += "Bye";
				sendResponse(commandSocket, response);
				break;
			}
			/*
			else if (strcmp(recvMsg.substr(0, 3).c_str(), "get") == 0) {
				std::string fileName = recvMsg.substr(4);
				std::string filePath = std::string(path) + "/" + fileName;
				std::cout << "get " << filePath << std::endl;
				FILE* fin = fopen(filePath.c_str(), "rb");
				if (fin == NULL) {
					std::string response = "get fail";
					int sendLength = send(commandSocket, response.c_str(), strlen(response.c_str()), 0);
					if (sendLength != strlen(response.c_str())) {
						std::cout << "Response to client failed!" << std::endl;
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

					int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
					if (listenSocket == SOCKET_ERROR) {
						std::cout << "Create data socket failed!" << std::endl;
						exit(0);
					}

					std::cout << "Data of file : " << data << std::endl;
					int dataLength = send(dataSocket, data.c_str(), strlen(data.c_str()), 0);
					if (dataLength != strlen(data.c_str())) {
						std::cout << "Response get to client failed!" << std::endl;
						exit(0);
					}
					std::cout << "get file successful!" << std::endl;

				}
			}
			*/
			else if(strcmp(recvMsg.substr(0, 3).c_str(), "put") == 0) {
				//-------------------------
				std::string fileName = recvMsg.substr(4);
				std::string filePath = path + "/" + fileName;
				if(filePath[filePath.size()-1] == '\n') {
					filePath.erase(filePath.size()-1);
				}
				FILE* fout = fopen(filePath.c_str(), "wb");
				std::cout << "put " << filePath << std::endl;
				std::string response = "put ";
				//-------------------------open file failed-----------------------------------
				if(fout == NULL) {
					response += "0000";
					sendResponse(commandSocket, response);
					continue;
				}

				///------------------------create server data listen socket------------------------
				int dataListenSocket = socket(AF_INET, SOCK_STREAM, 0);
				if (dataListenSocket == SOCKET_ERROR) {
					std::cout << "Create data listen socket failed!" << std::endl;
					exit(0);
				}
				//set data socket
				struct sockaddr_in dtaAddr;
				memset(&dtaAddr, 0, sizeof(dtaAddr));
				dtaAddr.sin_family = AF_INET;
				dtaAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				dtaAddr.sin_port = htons(DATA_TRAN_PORT);
				//bind socket and address
				if (bind(dataListenSocket, (struct sockaddr*)&dtaAddr, sizeof(dtaAddr)) == SOCKET_ERROR) {
					std::cout << "Data socket bind to local host failed!" << std::endl;
					exit(0);
				}

				if (listen(dataListenSocket, BACKLOG) == SOCKET_ERROR) {
					std::cout << "Error listening on data listen socket.\n" << std::endl;
					exit(0);
				}
				std::cout << "Waiting for client connect!" << std::endl;
				//send server data port to client
				response += "8001";
				sendResponse(commandSocket, response);

				int dataSocket;
				if ((dataSocket = accept(dataListenSocket, NULL, NULL)) == SOCKET_ERROR) {
					std::cout << "Accept data socket failed!" << std::endl;
					exit(0);
				}
				std::cout << "Client connect successful!" << std::endl;

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
				close(dataListenSocket);
				close(dataSocket);

			}
			else {
				std::cout << "Command illegal!" << std::endl;
				std::string response = "Command illegal!";
				sendResponse(commandSocket, response);
			}
		}
		close(commandSocket);
	}
	close(listenSocket);

}
