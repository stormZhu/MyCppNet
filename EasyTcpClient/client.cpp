#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

struct DataPackage {
    int age;
    char name[32];
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
        else {
            // 5.�������������������
            send(_sock, cmdBuf, strlen(cmdBuf)+1, 0); //+1��Ϊ�˽�'\0'Ҳ����ȥ
        }

        //3.���շ�������Ϣ recv
        char recvBuf[128] = {};
        int nlen = recv(_sock, recvBuf, sizeof(recvBuf), 0);
        if (nlen > 0) {
            DataPackage* dp = (DataPackage*)recvBuf; // ��̫��ȫ
            printf("���յ����ݣ�����=%d, ����=%s \n", dp->age, dp->name);
        }
    }

    //6. closesocket �ر��׽���
    closesocket(_sock);

    WSACleanup();
    printf("���˳�\n");
    getchar();
    return 0;
}
