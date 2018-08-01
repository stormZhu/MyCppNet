#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

//�ڴ���룬�ֽڴ�С��ǰ��˱��뱣֤һ��

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

struct DataHeader {
    short dataLength;
    short cmd;
};

struct Login : public DataHeader
{
    // �����ʱ���ʼ����Ϣͷ����Ϣ
    Login(){
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char passWord[32];
};

struct LoginResult : public DataHeader
{
    LoginResult() {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 1;
    }
    int result;
};

struct Logout : public DataHeader
{
    Logout(){
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct LogoutResult : public DataHeader
{
    LogoutResult(){
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 2;
    }
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
        char szRecv[4096] = {};
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader *header = (DataHeader*)szRecv;
        if (nLen <= 0){
            printf("�ͻ������˳��� ���������\n");
            break;
        }
//        if (header->dataLength >= sizeof(szRecv)){
//            //���ܻ���������
//        }
        switch(header->cmd)
        {
        case CMD_LOGIN:
            {
            recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
            Login *login = (Login*)szRecv;
            printf("�յ����CMD_LOGIN, ���ݳ���=%d, userName=%s, passWord=%s\n",
                   login->dataLength, login->userName, login->passWord);
            //����Ӧ��
            LoginResult ret;
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
            }
            break;

        case CMD_LOGOUT:
            {
            recv(_cSock, szRecv+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
            Logout *logout = (Logout*)szRecv;
            printf("�յ����CMD_LOGOUT, ���ݳ���=%d, userName=%s\n",
                   logout->dataLength, logout->userName);
            //����Ӧ��
            LogoutResult ret;
            send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
            }
            break;
        default:
            {
            DataHeader header= {0, CMD_ERROR};
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
