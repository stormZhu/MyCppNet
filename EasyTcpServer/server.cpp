#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#include <vector>
#include <algorithm>
//内存对齐，字节大小，前后端必须保证一致

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

std::vector<SOCKET> g_clients;

//-1：客户端退出了
int processor(SOCKET _cSock)
{
    char szRecv[4096] = {};
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader *header = (DataHeader*)szRecv;
    if (nLen <= 0){
        printf("客户端<Socket=%d>已退出， 任务结束。\n", _cSock);
        return -1;
    }

    switch(header->cmd)
    {
    case CMD_LOGIN:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
        Login *login = (Login*)szRecv;
        printf("收到客户端<Socket=%d>请求：CMD_LOGIN, 数据长度=%d, userName=%s, passWord=%s\n",
               _cSock, login->dataLength, login->userName, login->passWord);
        //发送应答
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
        }
        break;

    case CMD_LOGOUT:
        {
        recv(_cSock, szRecv+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
        Logout *logout = (Logout*)szRecv;
        printf("收到客户端<Socket=%d>请求：CMD_LOGOUT, 数据长度=%d, userName=%s\n",
               _cSock, logout->dataLength, logout->userName);
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

    return 0;
}

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

    while(true){
        // 伯克利 socket
        // 第一个参数在windows里面没有意义  linux中 ：监控的最大文件描述符+1
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;
        //清空
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);
        //添加listen fd监控
        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExp);

        //添加客户端的监控
        for(size_t n = 0; n < g_clients.size(); n++){
            FD_SET(g_clients[n], &fdRead);
        }
        timeval t = {1, 0};
        // 返回值有可能为0吗 ？ 返回的时候fdRead会被修改
        int ret = select(_sock+1, &fdRead, &fdWrite, &fdExp, &t);
        if(ret < 0){
            printf("select任务结束\n");
            break;
        }

        //如果listen fd有读事件，就调用accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            //4.accept 等待客户端连接
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(clientAddr);
            SOCKET _cSock = INVALID_SOCKET;

            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
            if (INVALID_SOCKET == _cSock) {
                printf("错误, 接收到无效客户端SOCKET...\n");
//                continue; //不要continue，因为accept失败不影响处理下面的其他客户端
            }
            else{
                //群发一个有客户登陆的消息
                for(size_t n=0;n<g_clients.size();n++){
                    NewUserJoin userJoin;
                    send(g_clients[n], (const char*)&userJoin, sizeof(userJoin), 0);
                }
                g_clients.push_back(_cSock); //将客户端的sock储存起来
                printf("新客户端加入： socket = %d socketIP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
            }
        }

        //循环处理客户端消息 _sock已经从fdRead中删除了，所以遍历一遍fdRead即可
        for(size_t n=0;n<fdRead.fd_count;n++){
            if(-1 == processor(fdRead.fd_array[n])){
                auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
                if(iter != g_clients.end()) {
                    g_clients.erase(iter); //这个if是判断处理失败的情况，失败了就将套接字移除，下次就不会加入监听
                }
            }
        }

//        printf("空闲时间处理其他事件\n");
    }
    //6.closesocket 关闭套接字
    for(size_t i=0;i<g_clients.size();i++){
        closesocket(g_clients[i]);
    }
    closesocket(_sock);
    WSACleanup();
    printf("已退出，任务结束.\n");\
    getchar();
    return 0;
}
