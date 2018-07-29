#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
int main()
{
    //启动Windows socket 2.x环境
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    // -------------

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
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret){
        printf("错误，连接失败...\n");
    }
    else {
        printf("连接成功\n");
    }
    //3.接收服务器信息 recv
    char recvBuf[256] = {};
    int nlen = recv(_sock, recvBuf, sizeof(recvBuf), 0);
    if (nlen > 0) {
        printf("接收到数据：%s \n", recvBuf);
    }
    //4. closesocket 关闭套接字
    closesocket(_sock);

    WSACleanup();
    getchar();
    return 0;
}
