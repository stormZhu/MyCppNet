#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <WinSock2.h>
    #include <Windows.h>
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <string.h>
    #define SOCKET int
    #define INVALID_SOCKET	(SOCKET)(~0)
    #define SOCKET_ERROR	(-1)
#endif
#include <stdio.h>
#include <thread>

#define SERVER_IP ("127.0.0.1")
//#define SERVER_IP ("192.168.31.154")
//#define SERVER_IP ("192.168.31.31")
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

//-1���˳���
int processor(SOCKET _cSock)
{
    char szRecv[4096] = {};
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader *header = (DataHeader*)szRecv;
    if (nLen <= 0){
        printf("��������Ͽ����ӣ� ���������\n");
        return -1;
    }

    switch(header->cmd)
    {
    case CMD_LOGIN_RESULT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        LoginResult *loginResult = (LoginResult*)szRecv;
        printf("�յ��������Ϣ�� CMD_LOGIN_RESULT, ���ݳ���=%d,\n",
               loginResult->dataLength);
        }
        break;

    case CMD_LOGOUT_RESULT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        LogoutResult *loginResult = (LogoutResult*)szRecv;
        printf("�յ��������Ϣ�� CMD_LOGOUT_RESULT, ���ݳ���=%d,\n",
               loginResult->dataLength);
        }
        break;
    case CMD_NEW_USER_JOIN:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        NewUserJoin *newUser = (NewUserJoin*)szRecv;
        printf("�յ��������Ϣ�� CMD_NEW_USER_JOIN, ���ݳ���=%d,\n",
               newUser->dataLength);
        }
        break;

    }

    return 0;
}

bool g_bRun = true;

void cmdThread(SOCKET _sock)
{
    while(true){
        // ������������
        char cmdBuf[128] = {}; //�洢���������
        scanf("%s", cmdBuf);
        // ������������
        if ( 0 == strcmp(cmdBuf, "exit")){
            printf("�յ�exit����˳�cmdThread\n");
            g_bRun = false;
            break;
        }
        else if ( 0 == strcmp(cmdBuf, "login")){
            // �������������������
            Login login;
            strcpy(login.userName, "zyq");
            strcpy(login.passWord, "zyqmm");
            send(_sock, (const char*)&login, sizeof(Login), 0);//ֱ�ӷ�������
        }
        else if ( 0 == strcmp(cmdBuf, "logout")){
            // �������������������
            Logout logout;
            strcpy(logout.userName, "zyq");
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
        }
        else {
            // �������������������
            printf("��֧�ֵ��������������. \n");
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
#endif
    // -------------

    std::string target_ip = SERVER_IP;
    if (argc > 1){
        target_ip = argv[1];
    }

    //1.����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _sock){
        printf("���󣬽���socketʧ��... \n");
    }
    else {
        printf("����Socket�ɹ� \n");
    }

    //2.connect ���ӷ�����
    sockaddr_in _sin = {}; //����Ҫ���ӵķ�����
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr(target_ip.c_str());
#else
    _sin.sin_addr.s_addr = inet_addr(target_ip.c_str());
#endif
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret){
        printf("�������ӷ�����ʧ��...\n");
    }
    else {
        printf("���ӷ������ɹ�\n");
    }

    std::thread t1(cmdThread, _sock);
    t1.detach();
    while (g_bRun) {
        fd_set fdRead;
        FD_ZERO(&fdRead);

        FD_SET(_sock, &fdRead);
        timeval t = {1, 0};
        int ret = select(_sock+1, &fdRead, NULL, NULL, &t);
        if(ret < 0){
            printf("select�������\n");
            break;
        }

        //���listen fd�ж��¼����͵���accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            if(-1 == processor(_sock)){
                printf("select�������\n");
                break;
            }
        }
    }

    //6. closesocket �ر��׽���
#ifdef _WIN32
    closesocket(_sock);
#else
    close(_sock);
#endif

#ifdef _WIN32
    WSACleanup();
#endif
    printf("���˳�\n");
    getchar();
    return 0;
}
