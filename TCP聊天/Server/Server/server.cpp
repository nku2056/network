#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#pragma warning(disable:4996)
#define BUF_SIZE 100
using namespace std;
DWORD WINAPI Send(LPVOID sockpara) {
	SOCKET* sock = (SOCKET*)sockpara;
	char bufSend[BUF_SIZE] = { 0 };
	while (1)
	{
		cin >> bufSend;
		int t = send(*sock, bufSend, strlen(bufSend), 0);
		if (strcmp(bufSend, "quit") == 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			closesocket(*sock);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：您已退出聊天室" << endl;
			return 0L;
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

DWORD WINAPI Recv(LPVOID sock_) {
	char bufRecv[BUF_SIZE] = { 0 };
	SOCKET* sock = (SOCKET*)sock_;
	while (1) {
		int t = recv(*sock, bufRecv, BUF_SIZE, 0);
		if (strcmp(bufRecv, "quit") == 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			closesocket(*sock);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：对方已退出聊天室" << endl;
			return 0L;
		}
		if (t > 0) {
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：收到信息" << endl;
			cout << bufRecv << endl;
			cout << "-------------------------------------------------------------" << endl;
		}
		memset(bufRecv, 0, BUF_SIZE);
	}
}

int main()
{
	//初始化DLL
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	//创建套接字
	SOCKET servSock = socket(PF_INET, SOCK_STREAM, 0);
	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = PF_INET;  //使用IPv4地址
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
	sockAddr.sin_port = htons(1234);  //端口
	bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	//进入监听状态
	if (listen(servSock, 20) == 0)
	{
		cout << "listening" << endl;
	}
	//接收客户端请求
	SOCKADDR clntAddr;
	int nSize = sizeof(SOCKADDR);
	SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);
	if (clntSock > 0)
	{
		cout << "client online" << endl;
	}
	//开启多线程
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, Recv, (LPVOID)&clntSock, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, Send, (LPVOID)&clntSock, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	closesocket(clntSock);  //关闭套接字
	//关闭套接字
	closesocket(servSock);
	//终止 DLL 的使用
	WSACleanup();
	return 0;
}