#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

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
#include "MessageHeader.h"

class EasyTcpClient
{
public:
    EasyTcpClient() {
        _sock = INVALID_SOCKET;
    }

    virtual ~EasyTcpClient() {
        Close(); //关闭监听套接字
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // 初始化socket
    void initSocket() {
#ifdef _WIN32
        //启动Windows socket 2.x环境
        WORD ver = MAKEWORD(2, 2);
        WSADATA dat;
        WSAStartup(ver, &dat);
#endif
        if(INVALID_SOCKET != _sock)
            Close();
        // 建立一个socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            printf("Error, create socket failed... \n");
        }
        else {
            printf("create socket<%d> succeed \n", (int)_sock);
        }
    }

    // 连接服务器
    int Connect(char *ip, unsigned short port) {
        if(INVALID_SOCKET == _sock)
            initSocket();

        // connect 连接服务器
        sockaddr_in _sin = {}; //设置要连接的服务器
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        _sin.sin_addr.s_addr = inet_addr(ip);
#endif
        int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
        if (SOCKET_ERROR == ret){
            printf("ERROR, connect server<%s> failed\n", ip);
        }
        else {
            printf("connect server<%s> succeed\n", ip);
        }
        return ret;
    }

    //关闭socket
    void Close(){
        //不要重复close
        if(INVALID_SOCKET == _sock)
            return;

#ifdef _WIN32
        closesocket(_sock);
#else
        close(_sock);
#endif
        _sock = INVALID_SOCKET;
    }


    // 查询监听事件
    bool OnRun() {
        // 不在运行中，就返回
        if(!isRun()) {
            return false;
        }
        fd_set fdRead;
        FD_ZERO(&fdRead);

        FD_SET(_sock, &fdRead);
        timeval t = {1, 0};
        int ret = select(_sock+1, &fdRead, NULL, NULL, &t);
        if(ret < 0){
            printf("<Socket=%d> select work finished\n", (int)_sock);
            return false;
        }

        //如果listen fd有读事件，就调用accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            if(-1 == RecvData(_sock)){
                printf("select work finished\n");
//                break;
            }
        }
        return true;
    }

    bool isRun() {
        return _sock != INVALID_SOCKET;
    }

    // 接收消息，处理粘包，拆包
    int RecvData(SOCKET _cSock)
    {
        // 缓冲区
        char szRecv[4096] = {};
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader *header = (DataHeader*)szRecv;
        if (nLen <= 0){
            printf("Stop the connection by server\n");
            Close();
            return -1;
        }

        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);

        OnNetMsg(header);
        return 0;
    }

    // 处理消息
    void OnNetMsg(DataHeader* header) {
        switch(header->cmd)
        {
        case CMD_LOGIN_RESULT:
            {
            LoginResult *loginResult = (LoginResult*)header;
            printf("Revceive message: CMD_LOGIN_RESULT, dataLength=%d,\n",
                   loginResult->dataLength);
            }
            break;

        case CMD_LOGOUT_RESULT:
            {
            LogoutResult *loginResult = (LogoutResult*)header;
            printf("Revceive message: CMD_LOGOUT_RESULT, dataLength=%d,\n",
                   loginResult->dataLength);
            }
            break;
        case CMD_NEW_USER_JOIN:
            {
            NewUserJoin *newUser = (NewUserJoin*)header;
            printf("Revceive message: CMD_NEW_USER_JOIN, dataLength=%d,\n",
                   newUser->dataLength);
            }
            break;

        }
    }

    // 发送数据
    int SendData(DataHeader* header) {
        if(isRun() && header) {
            send(_sock, (const char*)header, header->dataLength, 0);
        }
        return SOCKET_ERROR;
    }
private:
    SOCKET _sock;
};

#endif // EASYTCPCLIENT_H
