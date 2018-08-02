#include <stdio.h>
#include <thread>
#include "EasyTcpClient.h"


#define SERVER_IP ("127.0.0.1")
//#define SERVER_IP ("192.168.31.154")
//#define SERVER_IP ("192.168.31.31")

bool g_bRun = true;

void cmdThread(EasyTcpClient *client)
{
    while(true){
        // ������������
        char cmdBuf[128] = {}; //�洢���������
        scanf("%s", cmdBuf);
        // ������������
        if ( 0 == strcmp(cmdBuf, "exit")){
            client->Close();
            printf("�յ�exit����˳�cmdThread\n");
            g_bRun = false;
            break;
        }
        else if ( 0 == strcmp(cmdBuf, "login")){
            // �������������������
            Login login;
            strcpy(login.userName, "zyq");
            strcpy(login.passWord, "zyqmm");
            client->SendData(&login);
        }
        else if ( 0 == strcmp(cmdBuf, "logout")){
            // �������������������
            Logout logout;
            strcpy(logout.userName, "zyq");
            client->SendData(&logout);
        }
        else {
            // �������������������
            printf("��֧�ֵ��������������. \n");
        }
    }
}

int main(int argc, char *argv[])
{

    std::string target_ip = SERVER_IP;
    if (argc > 1){
        target_ip = argv[1];
    }

    EasyTcpClient client;
    client.Connect((char*)target_ip.c_str(), 4567);

    EasyTcpClient client2;
    client2.Connect((char*)target_ip.c_str(), 4567);


    std::thread t1(cmdThread, &client);
    t1.detach();

    while(client.isRun() || client2.isRun()) {
        client.OnRun();
        client2.OnRun();
    }
    printf("���˳�\n");
    getchar();
    return 0;
}
