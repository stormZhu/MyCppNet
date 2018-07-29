#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... 没用上

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

enum CMD {
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

struct DataHeader {
    short dataLength;
    short cmd;
};

struct Login {
    char userName[32];
    char passWord[32];
};

struct LoginResult {
    int result;
};

struct Logout {
    char userName[32];
};

struct LogoutResult {
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
        else if ( 0 == strcmp(cmdBuf, "login")){
            // 5.向服务器发送请求命令
            DataHeader dh = {sizeof(Login), CMD_LOGIN};
            Login login = {"zyq", "zyqlogin"};
            send(_sock, (const char*)&dh, sizeof(DataHeader), 0);  //先发包头
            send(_sock, (const char*)&login, sizeof(Login), 0); //再发包体
            //接收服务器返回的数据
            DataHeader dh2 = {};
            LoginResult loginRet = {};
            recv(_sock, (char *)&dh2, sizeof(DataHeader), 0); //接收头
            recv(_sock, (char *)&loginRet, sizeof(LoginResult), 0); //接收内容
            printf("LoginResult: %d \n", loginRet.result);

        }
        else if ( 0 == strcmp(cmdBuf, "logout")){
            // 5.向服务器发送请求命令
            DataHeader dh = {sizeof(Logout), CMD_LOGOUT};
            Logout logout = {"zyq"};
            send(_sock, (const char*)&dh, sizeof(DataHeader), 0);  //先发包头
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
            //接收服务器返回的数据
            DataHeader dh2 = {};
            LogoutResult logoutRet = {};
            recv(_sock, (char *)&dh2, sizeof(DataHeader), 0); //接收头
            recv(_sock, (char *)&logoutRet, sizeof(LogoutResult), 0); //接收内容
            printf("LogoutResult: %d \n", logoutRet.result);
        }
        else {
            // 5.向服务器发送请求命令
            printf("不支持的命令，请重新输入. \n");
        }
    }

    //6. closesocket 关闭套接字
    closesocket(_sock);

    WSACleanup();
    printf("已退出\n");
    getchar();
    return 0;
}
