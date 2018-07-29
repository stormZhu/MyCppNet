#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

//�ڴ���룬�ֽڴ�С��ǰ��˱��뱣֤һ��

enum CMD {
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

struct DataHeader {
    short dataLength;
    short cmd;
};

struct Login {
    char userName[32];
    char passWord[32];
};

struct LoginResult {
    int result;
};

struct Logout {
    char userName[32];
};

struct LogoutResult {
    int result;
};
int main()
{
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    // -------------

    //1.����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //2.bind �����ڽ��ܿͻ������ӵ�����˿�
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");
    if( SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) ){
        printf("ERROR, �����ڽ��ܿͻ������ӵ�����˿�ʧ��\n");
    }
    else{
        printf("������˿ڳɹ�\n");
    }
    //3.listen ����
    if ( SOCKET_ERROR == listen(_sock, 5) ){
        printf("����,��������˿�ʧ��...\n");
    }
    else{
        printf("��������˿ڳɹ�...\n");
    }

    //4.accept �ȴ��ͻ�������
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(clientAddr);
    SOCKET _cSock = INVALID_SOCKET;

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("����, ���յ���Ч�ͻ���SOCKET...\n");
    }
    printf("�¿ͻ��˼��룺 socket = %d socketIP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

    while(true){
        //5. �Ƚ���
        DataHeader header = {};
        int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
        if (nLen <= 0){
            printf("�ͻ������˳��� ���������\n");
            break;
        }
        printf("�յ���� %d, ���ݳ��ȣ� %d\n", header.cmd, header.dataLength);

        switch(header.cmd)
        {
        case CMD_LOGIN:
            {
            Login login = {};
            recv(_cSock, (char*)&login, sizeof(Login), 0);
            //���������ж�
            //����Ӧ��
            LoginResult ret = {1};
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
            }
            break;

        case CMD_LOGOUT:
            {
            Logout logout = {0};
            recv(_cSock, (char*)&logout, sizeof(Logout), 0);
            //���������ж�...
            //����Ӧ��
            LogoutResult ret = {};
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
            }
            break;
        default:
            {
            header.cmd = CMD_ERROR;
            header.dataLength = 0;
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            }
            break;
        }
    }
    //6.closesocket �ر��׽���
    closesocket(_sock);
    WSACleanup();
    printf("���˳����������.\n");\
    getchar();
    return 0;
}
