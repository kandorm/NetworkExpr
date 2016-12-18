#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h> //htons
#include <sys/socket.h> //inet_pton
#include <string.h> //memset string
#include <iostream>  //cout
#include <unistd.h>  //getcwd
#include <dirent.h> //DIR opendir
#define MAX_SIZE 4096
#define HOST_PORT 8000
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

		while(true) {
			//-------------------receive client command------------------------
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

			//-------------------respone to command----------------------------
			if (strcmp(recvMsg.substr(0, 1).c_str(), "?") == 0) {
				std::string response = "?   ";
				response += "get [port] [filename]\n";
				response += "put [port] [filename]\n";
				response += "pwd\n";
				response += "dir\n";
				response += "cd\n";
				response += "?\n";
				response += "quit\n";

				sendResponse(commandSocket, response);
			}
			else if(strcmp(recvMsg.substr(0, 2).c_str(), "cd") == 0) {
				if(strcmp(recvMsg.substr(3).c_str(), "..") == 0) {
					int pos = path.rfind("/");
					if(pos > 0) {
						path.erase(pos);
					}
					std::string response = "cd  ";
					response += path;
					sendResponse(commandSocket, response);
				}
				else {
					std::string extraPath = recvMsg(4);
					if(path[path.size()-1] != "/") {
						path += "/";
					}
					std::string tPath = path;
					tPath += extraPath;

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

			}
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
			else if(strcmp(recvMsg.substr(0, 3).c_str(), "put") == 0) {
				std::string fileName = recvMsg.substr(4);
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
		close(commandSocket);
	}
	close(listenSocket);

}
