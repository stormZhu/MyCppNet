#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

//内存对齐，字节大小，前后端必须保证一致

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

int main()
{
    //启动Windows socket 2.x环境
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    // -------------

    //1.建立一个socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //2.bind 绑定用于接受客户端连接的网络端口
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");
    if( SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) ){
        printf("ERROR, 绑定用于接受客户端连接的网络端口失败\n");
    }
    else{
        printf("绑定网络端口成功\n");
    }
    //3.listen 监听
    if ( SOCKET_ERROR == listen(_sock, 5) ){
        printf("错误,监听网络端口失败...\n");
    }
    else{
        printf("监听网络端口成功...\n");
    }

    //4.accept 等待客户端连接
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(clientAddr);
    SOCKET _cSock = INVALID_SOCKET;

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("错误, 接收到无效客户端SOCKET...\n");
    }
    printf("新客户端加入： socket = %d socketIP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

    while(true){
        //5. 先接收
        char szRecv[4096] = {};
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader *header = (DataHeader*)szRecv;
        if (nLen <= 0){
            printf("客户端已退出， 任务结束。\n");
            break;
        }
//        if (header->dataLength >= sizeof(szRecv)){
//            //可能缓冲区不够
//        }
        switch(header->cmd)
        {
        case CMD_LOGIN:
            {
            recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
            Login *login = (Login*)szRecv;
            printf("收到命令：CMD_LOGIN, 数据长度=%d, userName=%s, passWord=%s\n",
                   login->dataLength, login->userName, login->passWord);
            //发送应答
            LoginResult ret;
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
            }
            break;

        case CMD_LOGOUT:
            {
            recv(_cSock, szRecv+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
            Logout *logout = (Logout*)szRecv;
            printf("收到命令：CMD_LOGOUT, 数据长度=%d, userName=%s\n",
                   logout->dataLength, logout->userName);
            //发送应答
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
    //6.closesocket 关闭套接字
    closesocket(_sock);
    WSACleanup();
    printf("已退出，任务结束.\n");\
    getchar();
    return 0;
}
