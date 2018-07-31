#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

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

int main()
{
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    // -------------

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
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret){
        printf("�������ӷ�����ʧ��...\n");
    }
    else {
        printf("���ӷ������ɹ�\n");
    }

    while (true) {
        // 3.������������
        char cmdBuf[128] = {}; //�洢���������
        scanf("%s", cmdBuf);
        // 4.������������
        if ( 0 == strcmp(cmdBuf, "exit")){
            printf("�յ�exit����������\n");
            break;
        }
        else if ( 0 == strcmp(cmdBuf, "login")){
            // 5.�������������������

            Login login;
            strcpy(login.userName, "zyq");
            strcpy(login.passWord, "zyqmm");
            send(_sock, (const char*)&login, sizeof(Login), 0);//ֱ�ӷ�������
            //���շ��������ص�����
            LoginResult loginRet;
            recv(_sock, (char *)&loginRet, sizeof(LoginResult), 0); //��������
            printf("LoginResult: %d \n", loginRet.result);

        }
        else if ( 0 == strcmp(cmdBuf, "logout")){
            // 5.�������������������
            Logout logout;
            strcpy(logout.userName, "zyq");
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
            //���շ��������ص�����
            LogoutResult logoutRet = {};
            recv(_sock, (char *)&logoutRet, sizeof(LogoutResult), 0); //��������
            printf("LogoutResult: %d \n", logoutRet.result);
        }
        else {
            // 5.�������������������
            printf("��֧�ֵ��������������. \n");
        }
    }

    //6. closesocket �ر��׽���
    closesocket(_sock);

    WSACleanup();
    printf("���˳�\n");
    getchar();
    return 0;
}
