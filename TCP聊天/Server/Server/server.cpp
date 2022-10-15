#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
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
				WSAEnumNetworkEvents(cliSock[i], cliEvent[i], &networkEvents);//查看是什么事件
				//事件选择
				if (networkEvents.lNetworkEvents & FD_ACCEPT)//若产生accept事件（此处与位掩码相与）
				{
					if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						SYSTEMTIME st = { 0 };
						GetLocalTime(&st);
						cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：连接时产生错误" << endl;
						cout << "-------------------------------------------------------------" << endl;
						continue;
					}
					//接受链接
					if (total + 1 < MAX_LINK_NUM)//若增加一个客户端仍然小于最大连接数，则接受该链接
					{
						//total为已连接客户端数量
						int nextIndex = total + 1;//分配给新客户端的下标
						int addrLen = sizeof(SOCKADDR);
						SOCKET newSock = accept(servSock, (SOCKADDR*)&cliAddr[nextIndex], &addrLen);
						if (newSock != INVALID_SOCKET)
						{
							cliSock[nextIndex] = newSock;
							WSAEVENT newEvent = WSACreateEvent();
							WSAEventSelect(cliSock[nextIndex], newEvent, FD_CLOSE | FD_READ | FD_WRITE);
							cliEvent[nextIndex] = newEvent;
							total++;//客户端连接数增加
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << endl;
							cout << "#" << nextIndex << "用户（IP：" << inet_ntoa(cliAddr[nextIndex].sin_addr) << ")进入了聊天室，当前连接数：" << total << endl;
							cout << "-------------------------------------------------------------" << endl;
							//给所有客户端发送欢迎消息
							char buf[BUF_SIZE] = "欢迎用户（IP：";
							strcat(buf, inet_ntoa(cliAddr[nextIndex].sin_addr));
							strcat(buf, ")进入聊天室");
							for (int j = i; j <= total; j++)
							{
								send(cliSock[j], buf, sizeof(buf), 0);
							}
						}
					}
				}
				else if (networkEvents.lNetworkEvents & FD_CLOSE)//客户端被关闭，即断开连接
				{
					//i表示已关闭的客户端下标
					total--;
					SYSTEMTIME st = { 0 };
					GetLocalTime(&st);
					cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << endl;
					cout << "#" << i << "用户（IP：" << inet_ntoa(cliAddr[i].sin_addr) << ")退出了聊天室,当前连接数：" << total << endl;
					cout << "-------------------------------------------------------------" << endl;
					//释放这个客户端的资源
					closesocket(cliSock[i]);
					WSACloseEvent(cliEvent[i]);
					//数组调整,用顺序表删除元素
					for (int j = i; j < total; j++)
					{
						cliSock[j] = cliSock[j + 1];
						cliEvent[j] = cliEvent[j + 1];
						cliAddr[j] = cliAddr[j + 1];
					}
					//给所有客户端发送退出聊天室的消息
					char buf[BUF_SIZE] = "用户（IP：";
					strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
					strcat(buf, ")退出聊天室");
					SYSTEMTIME st = { 0 };
					GetLocalTime(&st);
					cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << endl;
					cout << "#" << i << "用户（IP：" << inet_ntoa(cliAddr[i].sin_addr) << ")退出了聊天室,当前连接数：" << total << endl;
					cout << "-------------------------------------------------------------" << endl;
					for (int j = 1; j <= total; j++)
					{
						send(cliSock[j], buf, sizeof(buf), 0);
					}
				}
				else if (networkEvents.lNetworkEvents & FD_READ)//接收到消息
				{
					char buffer[BUF_SIZE] = { 0 };//字符缓冲区，用于接收和发送消息
					char buffer2[BUF_SIZE] = { 0 };
					for (int j = 1; j <= total; j++)
					{
						int nrecv = recv(cliSock[j], buffer, sizeof(buffer), 0);//nrecv是接收到的字节数
						if (strcmp(buffer, "quit") == 0)
						{
							total--;
							//释放这个客户端的资源
							closesocket(cliSock[j]);
							WSACloseEvent(cliEvent[j]);
							//数组调整,用顺序表删除元素
							for (int k = j; k < total; k++)
							{
								cliSock[k] = cliSock[k + 1];
								cliEvent[k] = cliEvent[k + 1];
								cliAddr[k] = cliAddr[k + 1];
							}
							//给所有客户端发送退出聊天室的消息
							char buf[BUF_SIZE] = "用户（IP：";
							strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
							strcat(buf, ")退出聊天室");
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << endl;
							cout << "#" << i << "用户（IP：" << inet_ntoa(cliAddr[i].sin_addr) << ")退出了聊天室,当前连接数：" << total << endl;
							cout << "-------------------------------------------------------------" << endl;
							for (int k = 1; k <= total; k++)
							{
								send(cliSock[k], buf, sizeof(buf), 0);
							}
						}
						else if (nrecv > 0)//如果接收到的字符数大于0
						{
							sprintf(buffer2, "[#%d]%s", j, buffer);
							//在服务端显示
							SYSTEMTIME st = { 0 };
							GetLocalTime(&st);
							cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：接收到客户端信息" << endl;
							cout << buffer2 << endl;
							cout << "-------------------------------------------------------------" << endl;
							//在其他客户端显示（广播给其他客户端）
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
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：服务器关闭" << endl;
			cout << "-------------------------------------------------------------" << endl;
			return 0;
		}
		if (t > 0) {
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：消息成功发送" << endl;
			cout << "-------------------------------------------------------------" << endl;
		}
		memset(bufSend, 0, BUF_SIZE);
	}
}

int main()
{
	//初始化DLL
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	//创建套接字
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;  //使用IPv4地址
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
	sockAddr.sin_port = htons(12345);  //端口
	bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	WSAEVENT servEvent = WSACreateEvent();//创建一个人工重设为传信的事件对象
	WSAEventSelect(servSock, servEvent, FD_ALL_EVENTS);//绑定事件对象，并且监听所有事件
	cliSock[0] = servSock;
	cliEvent[0] = servEvent;
	//进入监听状态
	listen(servSock, 10);
	cout << "聊天室服务器已开启" << endl;
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, servSend, (LPVOID)&servSock, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	//终止 DLL 的使用
	WSACleanup();
	return 0;
}