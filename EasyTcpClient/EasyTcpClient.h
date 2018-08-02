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
        Close(); //�رռ����׽���
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // ��ʼ��socket
    void initSocket() {
#ifdef _WIN32
        //����Windows socket 2.x����
        WORD ver = MAKEWORD(2, 2);
        WSADATA dat;
        WSAStartup(ver, &dat);
#endif
        if(INVALID_SOCKET != _sock)
            Close();
        // ����һ��socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            printf("���󣬽���socketʧ��... \n");
        }
        else {
            printf("����Socket�ɹ� \n");
        }
    }

    // ���ӷ�����
    int Connect(char *ip, unsigned short port) {
        if(INVALID_SOCKET == _sock)
            initSocket();

        // connect ���ӷ�����
        sockaddr_in _sin = {}; //����Ҫ���ӵķ�����
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        _sin.sin_addr.s_addr = inet_addr(ip);
#endif
        int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
        if (SOCKET_ERROR == ret){
            printf("�������ӷ�����ʧ��...\n");
        }
        else {
            printf("���ӷ������ɹ�\n");
        }
        return ret;
    }

    //�ر�socket
    void Close(){
        //��Ҫ�ظ�close
        if(INVALID_SOCKET == _sock)
            return;

#ifdef _WIN32
        closesocket(_sock);
#else
        close(_sock);
#endif
        _sock = INVALID_SOCKET;
    }


    // ��ѯ�����¼�
    bool OnRun() {
        // ���������У��ͷ���
        if(!isRun()) {
            return false;
        }
        fd_set fdRead;
        FD_ZERO(&fdRead);

        FD_SET(_sock, &fdRead);
        timeval t = {1, 0};
        int ret = select(_sock+1, &fdRead, NULL, NULL, &t);
        if(ret < 0){
            printf("<Socket=%d> select�������\n", _sock);
            return false;
        }

        //���listen fd�ж��¼����͵���accept
        if(FD_ISSET(_sock, &fdRead)){
            FD_CLR(_sock, &fdRead);

            if(-1 == RecvData(_sock)){
                printf("select�������\n");
//                break;
            }
        }
        return true;
    }

    bool isRun() {
        return _sock != INVALID_SOCKET;
    }

    // ������Ϣ������ճ�������
    int RecvData(SOCKET _cSock)
    {
        // ������
        char szRecv[4096] = {};
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader *header = (DataHeader*)szRecv;
        if (nLen <= 0){
            printf("��������Ͽ����ӣ� ���������\n");
            Close();
            return -1;
        }

        recv(_cSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);

        OnNetMsg(header);
        return 0;
    }

    // ������Ϣ
    void OnNetMsg(DataHeader* header) {
        switch(header->cmd)
        {
        case CMD_LOGIN_RESULT:
            {
            LoginResult *loginResult = (LoginResult*)header;
            printf("�յ��������Ϣ�� CMD_LOGIN_RESULT, ���ݳ���=%d,\n",
                   loginResult->dataLength);
            }
            break;

        case CMD_LOGOUT_RESULT:
            {
            LogoutResult *loginResult = (LogoutResult*)header;
            printf("�յ��������Ϣ�� CMD_LOGOUT_RESULT, ���ݳ���=%d,\n",
                   loginResult->dataLength);
            }
            break;
        case CMD_NEW_USER_JOIN:
            {
            NewUserJoin *newUser = (NewUserJoin*)header;
            printf("�յ��������Ϣ�� CMD_NEW_USER_JOIN, ���ݳ���=%d,\n",
                   newUser->dataLength);
            }
            break;

        }
    }

    // ��������
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
