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
    // 构造的时候初始化消息头的信息
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

//-1：退出了
int processor(SOCKET _cSock)
{
    char szRecv[4096] = {};
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader *header = (DataHeader*)szRecv;
    if (nLen <= 0){
        printf("与服务器断开连接， 任务结束。\n");
        return -1;
    }

    switch(header->cmd)
    {
    case CMD_LOGIN_RESULT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        LoginResult *loginResult = (LoginResult*)szRecv;
        printf("收到服务端消息： CMD_LOGIN_RESULT, 数据长度=%d,\n",
               loginResult->dataLength);
        }
        break;

    case CMD_LOGOUT_RESULT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        LogoutResult *loginResult = (LogoutResult*)szRecv;
        printf("收到服务端消息： CMD_LOGOUT_RESULT, 数据长度=%d,\n",
               loginResult->dataLength);
        }
        break;
    case CMD_NEW_USER_JOIN:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        NewUserJoin *newUser = (NewUserJoin*)szRecv;
        printf("收到服务端消息： CMD_NEW_USER_JOIN, 数据长度=%d,\n",
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
        // 输入请求命令
        char cmdBuf[128] = {}; //存储输入的命令
        scanf("%s", cmdBuf);
        // 处理请求命令
        if ( 0 == strcmp(cmdBuf, "exit")){
            printf("收到exit命令，退出cmdThread\n");
            g_bRun = false;
            break;
        }
        else if ( 0 == strcmp(cmdBuf, "login")){
            // 向服务器发送请求命令
            Login login;
            strcpy(login.userName, "zyq");
            strcpy(login.passWord, "zyqmm");
            send(_sock, (const char*)&login, sizeof(Login), 0);//直接发送内容
        }
        else if ( 0 == strcmp(cmdBuf, "logout")){
            // 向服务器发送请求命令
            Logout logout;
            strcpy(logout.userName, "zyq");
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
        }
        else {
            // 向服务器发送请求命令
            printf("不支持的命令，请重新输入. \n");
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    //启动Windows socket 2.x环境
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
#endif
    // -------------

    std::string target_ip = SERVER_IP;
    if (argc > 1){
        target_ip = argv[1];
    }

    //1.建立一个socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _sock){
        printf("错误，建立socket失败... \n");
    }
    else {
        printf("建立Socket成功 \n");
    }

    //2.connect 连接服务器
    sockaddr_in _sin = {}; //设置要连接的服务器
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr(target_ip.c_str());
#else
    _sin.sin_addr.s_addr = inet_addr(target_ip.c_str());
#endif
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret){
        printf("错误，连接服务器失败...\n");
    }
    else {
        printf("连接服务器成功\n");
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
            printf("select任务结束\n");
            break;
        }

        //如果listen fd有读事件，就调用accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            if(-1 == processor(_sock)){
                printf("select任务结束\n");
                break;
            }
        }
    }

    //6. closesocket 关闭套接字
#ifdef _WIN32
    closesocket(_sock);
#else
    close(_sock);
#endif

#ifdef _WIN32
    WSACleanup();
#endif
    printf("已退出\n");
    getchar();
    return 0;
}
