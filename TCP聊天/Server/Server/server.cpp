#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#pragma warning(disable:4996)
using namespace std;

const int BUF_SIZE = 1024;
const int MAX_LINK_NUM = 10;
SOCKET cliSock[MAX_LINK_NUM];
SOCKADDR_IN cliAddr[MAX_LINK_NUM];
WSAEVENT cliEvent[MAX_LINK_NUM];
int total = 0;

DWORD WINAPI servEventThread(LPVOID IpPara)
{
	SOCKET servSock = *(SOCKET*)IpPara;
	while (1)
	{
		for (int i = 0; i < total + 1; i++)
		{
			int index = WSAWaitForMultipleEvents(1, &cliEvent[i], false, 10, 0);
			index -= WSA_WAIT_EVENT_0;
			if (index == WSA_WAIT_TIMEOUT || index == WSA_WAIT_FAILED)
			{
				continue;
			}
			else if (index == 0)
			{
				WSANETWORKEVENTS networkEvents;
				WSAEnumNetworkEvents(cliSock[i], cliEvent[i], &networkEvents);//�鿴��ʲô�¼�
				//�¼�ѡ��
				if (networkEvents.lNetworkEvents & FD_ACCEPT)//������accept�¼����˴���λ�������룩
				{
					if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						SYSTEMTIME st = { 0 };
						GetLocalTime(&st);
						cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << "������ʱ��������" << endl;
						cout << "-------------------------------------------------------------" << endl;
						continue;
					}
					//��������
					if (total + 1 < MAX_LINK_NUM)//������һ���ͻ�����ȻС�����������������ܸ�����
					{
						//totalΪ�����ӿͻ�������
						int nextIndex = total + 1;//������¿ͻ��˵��±�
						int addrLen = sizeof(SOCKADDR);
						SOCKET newSock = accept(servSock, (SOCKADDR*)&cliAddr[nextIndex], &addrLen);
						if (newSock != INVALID_SOCKET)
						{
							cliSock[nextIndex] = newSock;
							WSAEVENT newEvent = WSACreateEvent();
							WSAEventSelect(cliSock[nextIndex], newEvent, FD_CLOSE | FD_READ | FD_WRITE);
							cliEvent[nextIndex] = newEvent;
							total++;//�ͻ�������������
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << endl;
							cout << "#" << nextIndex << "�û���IP��" << inet_ntoa(cliAddr[nextIndex].sin_addr) << ")�����������ң���ǰ��������" << total << endl;
							cout << "-------------------------------------------------------------" << endl;
							//�����пͻ��˷��ͻ�ӭ��Ϣ
							char buf[BUF_SIZE] = "��ӭ�û���IP��";
							strcat(buf, inet_ntoa(cliAddr[nextIndex].sin_addr));
							strcat(buf, ")����������");
							for (int j = i; j <= total; j++)
							{
								send(cliSock[j], buf, sizeof(buf), 0);
							}
						}
					}
				}
				else if (networkEvents.lNetworkEvents & FD_CLOSE)//�ͻ��˱��رգ����Ͽ�����
				{
					//i��ʾ�ѹرյĿͻ����±�
					total--;
					SYSTEMTIME st = { 0 };
					GetLocalTime(&st);
					cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << endl;
					cout << "#" << i << "�û���IP��" << inet_ntoa(cliAddr[i].sin_addr) << ")�˳���������,��ǰ��������" << total << endl;
					cout << "-------------------------------------------------------------" << endl;
					//�ͷ�����ͻ��˵���Դ
					closesocket(cliSock[i]);
					WSACloseEvent(cliEvent[i]);
					//�������,��˳���ɾ��Ԫ��
					for (int j = i; j < total; j++)
					{
						cliSock[j] = cliSock[j + 1];
						cliEvent[j] = cliEvent[j + 1];
						cliAddr[j] = cliAddr[j + 1];
					}
					//�����пͻ��˷����˳������ҵ���Ϣ
					char buf[BUF_SIZE] = "�û���IP��";
					strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
					strcat(buf, ")�˳�������");
					SYSTEMTIME st = { 0 };
					GetLocalTime(&st);
					cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << endl;
					cout << "#" << i << "�û���IP��" << inet_ntoa(cliAddr[i].sin_addr) << ")�˳���������,��ǰ��������" << total << endl;
					cout << "-------------------------------------------------------------" << endl;
					for (int j = 1; j <= total; j++)
					{
						send(cliSock[j], buf, sizeof(buf), 0);
					}
				}
				else if (networkEvents.lNetworkEvents & FD_READ)//���յ���Ϣ
				{
					char buffer[BUF_SIZE] = { 0 };//�ַ������������ڽ��պͷ�����Ϣ
					char buffer2[BUF_SIZE] = { 0 };
					for (int j = 1; j <= total; j++)
					{
						int nrecv = recv(cliSock[j], buffer, sizeof(buffer), 0);//nrecv�ǽ��յ����ֽ���
						if (strcmp(buffer, "quit") == 0)
						{
							total--;
							//�ͷ�����ͻ��˵���Դ
							closesocket(cliSock[j]);
							WSACloseEvent(cliEvent[j]);
							//�������,��˳���ɾ��Ԫ��
							for (int k = j; k < total; k++)
							{
								cliSock[k] = cliSock[k + 1];
								cliEvent[k] = cliEvent[k + 1];
								cliAddr[k] = cliAddr[k + 1];
							}
							//�����пͻ��˷����˳������ҵ���Ϣ
							char buf[BUF_SIZE] = "�û���IP��";
							strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
							strcat(buf, ")�˳�������");
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << endl;
							cout << "#" << i << "�û���IP��" << inet_ntoa(cliAddr[i].sin_addr) << ")�˳���������,��ǰ��������" << total << endl;
							cout << "-------------------------------------------------------------" << endl;
							for (int k = 1; k <= total; k++)
							{
								send(cliSock[k], buf, sizeof(buf), 0);
							}
						}
						else if (nrecv > 0)//������յ����ַ�������0
						{
							sprintf(buffer2, "[#%d]%s", j, buffer);
							//�ڷ������ʾ
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << "�����յ��ͻ�����Ϣ" << endl;
							cout << buffer2 << endl;
							cout << "-------------------------------------------------------------" << endl;
							//�������ͻ�����ʾ���㲥�������ͻ��ˣ�
							for (int k = 1; k < j; k++)
							{
								send(cliSock[k], buffer2, sizeof(buffer), 0);
							}
							for (int k = j + 1; k <= total; k++)
							{
								send(cliSock[k], buffer2, sizeof(buffer), 0);
							}
						}
					}
				}
			}
		}
	}
}

DWORD WINAPI servSend(LPVOID IpPara)
{
	SOCKET servSock = *(SOCKET*)IpPara;
	char bufSend[BUF_SIZE] = { 0 };
	int t;
	while (1)
	{
		cin >> bufSend;
		for (int i = 1; i <= total; i++)
		{
			t = send(cliSock[i], bufSend, sizeof(bufSend), 0);
		}
		if (strcmp(bufSend, "quit") == 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			closesocket(servSock);
			cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << "���������ر�" << endl;
			cout << "-------------------------------------------------------------" << endl;
			return 0;
		}
		if (t > 0) {
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			cout << st.wYear << "��" << st.wMonth << "��" << st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "�� " << st.wMilliseconds << "����" << "����Ϣ�ɹ�����" << endl;
			cout << "-------------------------------------------------------------" << endl;
		}
		memset(bufSend, 0, BUF_SIZE);
	}
}

int main()
{
	//��ʼ��DLL
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	//�����׽���
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
	//���׽���
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;  //ʹ��IPv4��ַ
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //�����IP��ַ
	sockAddr.sin_port = htons(12345);  //�˿�
	bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	WSAEVENT servEvent = WSACreateEvent();//����һ���˹�����Ϊ���ŵ��¼�����
	WSAEventSelect(servSock, servEvent, FD_ALL_EVENTS);//���¼����󣬲��Ҽ��������¼�
	cliSock[0] = servSock;
	cliEvent[0] = servEvent;
	//�������״̬
	listen(servSock, 10);
	cout << "�����ҷ������ѿ���" << endl;
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, servSend, (LPVOID)&servSock, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	//��ֹ DLL ��ʹ��
	WSACleanup();
	return 0;
}