#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

//内存对齐，字节大小，前后端必须保证一致

struct DataPackage {
    int age;
    char name[32];
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
    char _recvBuf[128] = {};
    while(true){
        //5. 先接收
        int nLen = recv(_cSock, _recvBuf, sizeof(_recvBuf), 0);
        if (nLen <= 0){
            printf("客户端已退出， 任务结束。\n");
            break;
        }

        //6.处理请求
        printf("收到命令： %s\n", _recvBuf);
        if (0 == strcmp(_recvBuf, "getInfo")) {
            // 7.返回数据
            DataPackage dp = {18, "zyq"}; //构造时初始化
            send(_cSock, (const char*)&dp, sizeof(DataPackage), 0);
        }
        else {
            // 7.返回数据
            char msgBuf[] = "???.";
            send(_cSock, msgBuf, strlen(msgBuf)+1, 0); //发送'\0'
        }
    }
    //6.closesocket 关闭套接字
    closesocket(_sock);
    WSACleanup();
    printf("已退出，任务结束.\n");\
    getchar();
    return 0;
}
