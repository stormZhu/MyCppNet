#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPE... û����

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
int main()
{
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    // -------------

    //1.����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //2.bind �����ڽ��ܿͻ������ӵ�����˿�
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");
    if( SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) ){
        printf("ERROR, �����ڽ��ܿͻ������ӵ�����˿�ʧ��\n");
    }
    else{
        printf("������˿ڳɹ�\n");
    }
    //3.listen ����
    if ( SOCKET_ERROR == listen(_sock, 5) ){
        printf("����,��������˿�ʧ��...\n");
    }
    else{
        printf("��������˿ڳɹ�...\n");
    }

    //4.accept �ȴ��ͻ�������
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(clientAddr);
    SOCKET _cSock = INVALID_SOCKET;
    char msgBuf[] = "Hello, I'm Server.";
    while(true){
        _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
        if (INVALID_SOCKET == _cSock) {
            printf("����, ���յ���Ч�ͻ���SOCKET...\n");
        }
        printf("�¿ͻ��˼��룺IP = %s \n", inet_ntoa(clientAddr.sin_addr));
        //5.send ��ͻ��˷���һ������
        send(_cSock, msgBuf, strlen(msgBuf)+1, 0); //����'\0'
    }
    //6.closesocket �ر��׽���
    closesocket(_sock);

    WSACleanup();
    return 0;
}
