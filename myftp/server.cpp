#include <netinet/in.h>  //sockaddr_in
#include <arpa/inet.h> //htons
#include <sys/socket.h> //inet_pton
#include <string.h> //memset


int main(int argc, char** argv) {
	//����Ϸ��Լ��
	if (argc != 4) {
		std::cout << "Usage:client <address> <port> <filename>" << std::endl;
		return 0;
	}
	//����socket��ַ�ṹ
	struct sockaddr_in serverAddr;
	memset(serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INFT, argv[1], &serverAddr.sin_addr) == 0) {
		std::cout << "IP Address error" << std::endl;
		return 0;
	}

	//������Դ������socket
	int commandSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (commandSocket < 0) {
		cout << "File to create a command socket!" << std::endl;
		return 0;
	}

	int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (dataSocket < 0) {
		cout << "Fail to create a data socket!" << std::endl;
		return 0;
	}

	//��Զ�˷���������TCP����
	int ret; //��¼connect������
	if ((ret = connect(commandSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		std::cout << "Command socket connect to server failed!" << std::endl;
		return 0;
	}

	if ((ret = connect(dataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		std::cout << "Data socket connect to server failed!" << std::endl;
		return 0;
	}



}