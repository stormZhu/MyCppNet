#ifndef EASYTCPSERVER_H
#define EASYTCPSERVER_H

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
#include <vector>
#include <algorithm>
#include "messageheader.h"

class EasyTcpServer
{
private:
    SOCKET _sock;
    std::vector<SOCKET> g_clients;
public:
    EasyTcpServer() {
        _sock = INVALID_SOCKET;
    }

    virtual ~EasyTcpServer() {
        Close();
    }

    //初始化Socket
    SOCKET InitSocket() {
#ifdef _WIN32
        //启动Windows socket 2.x环境
        WORD ver = MAKEWORD(2, 2);
        WSADATA dat;
        WSAStartup(ver, &dat);
#endif
        if(INVALID_SOCKET != _sock) {
            printf("Init: close old connection\n");
            Close();
        }
        // 建立一个socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            printf("ERROR, create socket failed... \n");
        }
        else {
            printf("create socket<%d> succeed \n", (int)_sock);
        }
        return _sock;
    }

    //绑定ip和端口号
    int Bind(const char* ip, unsigned short port) {
        //2.bind 绑定用于接受客户端连接的网络端口
        sockaddr_in _sin = {}; // {}相当于初始化为0了
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);

        if (ip)
            _sin.sin_addr.s_addr = inet_addr(ip);
        else
            _sin.sin_addr.s_addr = INADDR_ANY;

        int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
        if( SOCKET_ERROR == ret ){
            printf("ERROR,bind the port<%d> failed\n", port);
        }
        else{
            printf("bind the port<%d>  succeed\n", port);
        }

        return ret;
    }

    //监听端口号
    int Listen(int n) {
        //3.listen 监听
        int ret = listen(_sock, n);
        if ( SOCKET_ERROR == ret ){
            printf("ERROR, Socket<%d> listen port failed...\n",(int)_sock);
        }
        else{
            printf("Socket<%d> listen port succeed.\n", (int)_sock);
        }
        return ret;
    }

    //接受客户端连接
    int Accept() {
        //4.accept 等待客户端连接
        sockaddr_in clientAddr = {};
        int nAddrLen = sizeof(clientAddr);
        SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
        _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
        _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
        if (INVALID_SOCKET == _cSock) {
            printf("Socket<%d>ERROR, Receive invalid client Socket...\n", (int)_sock);
//                continue; //不要continue，因为accept失败不影响处理下面的其他客户端
        }
        else{
            //群发一个有客户登陆的消息
            NewUserJoin userJoin;
            SendDataToAll((DataHeader*)&userJoin);
            g_clients.push_back(_cSock); //将客户端的sock储存起来
            printf("Socket<%d> New Client Join, socket = %d socketIP = %s \n", (int)_sock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
        }
        return _cSock;
    }

    //关闭Socket
    void Close(){
        //不要重复close
        if(INVALID_SOCKET == _sock)
            return;

        for(size_t i=0;i<g_clients.size();i++){
            Close(g_clients[i]);
        }

        Close(_sock);

        _sock = INVALID_SOCKET;
    }
    void Close(SOCKET _sock){
    #ifdef _WIN32
        closesocket(_sock);
    #else
        close(_sock);
    #endif
    }
    //处理网络消息
    bool OnRun() {
        if (!isRun()) {
            return false;
        }
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

        SOCKET maxSock = _sock;
        //添加客户端的监控
        for(size_t n = 0; n < g_clients.size(); n++){
            FD_SET(g_clients[n], &fdRead);
            if(maxSock < g_clients[n])
                maxSock = g_clients[n];
        }
        timeval t = {1, 0};
        // 返回值有可能为0吗 ？ 返回的时候fdRead会被修改
        int ret = select(maxSock+1, &fdRead, &fdWrite, &fdExp, &t);
        if(ret < 0){
            Close();
            printf("select work finished\n");
            return false;
        }

        //如果listen fd有读事件，就调用accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            Accept();
        }


        //循环处理客户端消息 _sock已经从fdRead中删除了，所以遍历一遍fdRead即可
        auto iter = g_clients.begin();
        while(iter != g_clients.end()){
            //如果有事件
            if(FD_ISSET(*iter, &fdRead)){
                //处理事件，如果失败，移除
                if(-1 == RecvData(*iter)){
                    Close(*iter); //不先关闭，后面指向就变了，所以要先关
//                    printf("移除%d\n", *iter);
                    iter = g_clients.erase(iter); //这个if是判断处理失败的情况，失败了就将套接字移除，下次就不会加入监听
                    continue;
                }
            }
            iter++;
        }

        return true;
    }

    //是否在工作中
    //接收数据 处理粘包 拆分包
    //-1：客户端退出了
    int RecvData(SOCKET _cSock)
    {
        char szRecv[4096] = {};
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader *header = (DataHeader*)szRecv;
        if (nLen <= 0){
            printf("Client<Socket=%d> exit\n", _cSock);
            return -1;
        }
        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);

        OnNetMag(_cSock, header);
        return 0;
    }
    //响应网络消息 不同的客户端可能有不同的实现！子类可以修改这个函数
    virtual void OnNetMag(SOCKET _cSock, DataHeader* header) {
        switch(header->cmd)
        {
        case CMD_LOGIN:
            {

            Login *login = (Login*)header;
            printf("Receive Client<Socket=%d> Request: CMD_LOGIN, dataLength=%d, userName=%s, passWord=%s\n",
                   _cSock, login->dataLength, login->userName, login->passWord);
            //发送应答
            LoginResult ret;
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
            }
            break;

        case CMD_LOGOUT:
            {
            Logout *logout = (Logout*)header;
            printf("Receive Client<Socket=%d> Request: CMD_LOGOUT, dataLength=%d, userName=%s\n",
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
    }

    //发送给指定客户端数据
    int SendData(SOCKET _cSock, DataHeader* header) {
        if( isRun() && header ) {
            return send(_cSock, (const char*)header, header->dataLength, 0);
        }
        return SOCKET_ERROR;
    }
    //群发
    void SendDataToAll(DataHeader* header) {
        for(size_t n=0;n<g_clients.size();n++){
            SendData(g_clients[n], header);
        }
    }

    bool isRun() {
        return _sock != INVALID_SOCKET;
    }

};

#endif // EASYTCPSERVER_H

