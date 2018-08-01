#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#include <vector>
#include <algorithm>
//�ڴ���룬�ֽڴ�С��ǰ��˱��뱣֤һ��

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
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

struct NewUserJoin : public DataHeader
{
    NewUserJoin(){
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;
};

std::vector<SOCKET> g_clients;

//-1���ͻ����˳���
int processor(SOCKET _cSock)
{
    char szRecv[4096] = {};
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader *header = (DataHeader*)szRecv;
    if (nLen <= 0){
        printf("�ͻ���<Socket=%d>���˳��� ���������\n", _cSock);
        return -1;
    }

    switch(header->cmd)
    {
    case CMD_LOGIN:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        Login *login = (Login*)szRecv;
        printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN, ���ݳ���=%d, userName=%s, passWord=%s\n",
               _cSock, login->dataLength, login->userName, login->passWord);
        //����Ӧ��
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
        }
        break;

    case CMD_LOGOUT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
        Logout *logout = (Logout*)szRecv;
        printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT, ���ݳ���=%d, userName=%s\n",
               _cSock, logout->dataLength, logout->userName);
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

    return 0;
}

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

    while(true){
        // ������ socket
        // ��һ��������windows����û������  linux�� ����ص�����ļ�������+1
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;
        //���
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);
        //���listen fd���
        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExp);

        //��ӿͻ��˵ļ��
        for(size_t n = 0; n < g_clients.size(); n++){
            FD_SET(g_clients[n], &fdRead);
        }
        timeval t = {1, 0};
        // ����ֵ�п���Ϊ0�� �� ���ص�ʱ��fdRead�ᱻ�޸�
        int ret = select(_sock+1, &fdRead, &fdWrite, &fdExp, &t);
        if(ret < 0){
            printf("select�������\n");
            break;
        }

        //���listen fd�ж��¼����͵���accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            //4.accept �ȴ��ͻ�������
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(clientAddr);
            SOCKET _cSock = INVALID_SOCKET;

            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
            if (INVALID_SOCKET == _cSock) {
                printf("����, ���յ���Ч�ͻ���SOCKET...\n");
//                continue; //��Ҫcontinue����Ϊacceptʧ�ܲ�Ӱ�촦������������ͻ���
            }
            else{
                //Ⱥ��һ���пͻ���½����Ϣ
                for(size_t n=0;n<g_clients.size();n++){
                    NewUserJoin userJoin;
                    send(g_clients[n], (const char*)&userJoin, sizeof(userJoin), 0);
                }
                g_clients.push_back(_cSock); //���ͻ��˵�sock��������
                printf("�¿ͻ��˼��룺 socket = %d socketIP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
            }
        }

        //ѭ������ͻ�����Ϣ _sock�Ѿ���fdRead��ɾ���ˣ����Ա���һ��fdRead����
        for(size_t n=0;n<fdRead.fd_count;n++){
            if(-1 == processor(fdRead.fd_array[n])){
                auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
                if(iter != g_clients.end()) {
                    g_clients.erase(iter); //���if���жϴ���ʧ�ܵ������ʧ���˾ͽ��׽����Ƴ����´ξͲ���������
                }
            }
        }

//        printf("����ʱ�䴦�������¼�\n");
    }
    //6.closesocket �ر��׽���
    for(size_t i=0;i<g_clients.size();i++){
        closesocket(g_clients[i]);
    }
    closesocket(_sock);
    WSACleanup();
    printf("���˳����������.\n");\
    getchar();
    return 0;
}
