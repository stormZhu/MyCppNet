#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

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
        printf("错误，连接服务器失败...\n");
    }
    else {
        printf("连接服务器成功\n");
    }

    while (true) {
        // 3.输入请求命令
        char cmdBuf[128] = {}; //存储输入的命令
        scanf("%s", cmdBuf);
        // 4.处理请求命令
        if ( 0 == strcmp(cmdBuf, "exit")){
            printf("收到exit命令，任务结束\n");
            break;
        }
        else {
            // 5.向服务器发送请求命令
            send(_sock, cmdBuf, strlen(cmdBuf)+1, 0); //+1是为了将'\0'也发过去
        }

        //3.接收服务器信息 recv
        char recvBuf[128] = {};
        int nlen = recv(_sock, recvBuf, sizeof(recvBuf), 0);
        if (nlen > 0) {
            DataPackage* dp = (DataPackage*)recvBuf; // 不太安全
            printf("接收到数据：年龄=%d, 姓名=%s \n", dp->age, dp->name);
        }
    }

    //6. closesocket 关闭套接字
    closesocket(_sock);

    WSACleanup();
    printf("已退出\n");
    getchar();
    return 0;
}
