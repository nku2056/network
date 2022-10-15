#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
using namespace std;

const int BUF_SIZE = 1024;

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

DWORD WINAPI Recv(LPVOID sockpara) {
	char bufRecv[BUF_SIZE] = { 0 };
	SOCKET* sock = (SOCKET*)sockpara;
	while (1) {
		int t = recv(*sock, bufRecv, BUF_SIZE, 0);
		if (strcmp(bufRecv, "quit") == 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			closesocket(*sock);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：服务器已关闭" << endl;
			cout << "-------------------------------------------------------------" << endl;
			return 0;
		}
		if (t > 0) {
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：收到信息" << endl;
			cout << bufRecv << endl;
			cout << "-------------------------------------------------------------" << endl;
		}
		if (t < 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			closesocket(*sock);
			cout << st.wYear << "年" << st.wMonth << "月" << st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒 " << st.wMilliseconds << "毫秒" << "：与服务器断开连接" << endl;
			cout << "-------------------------------------------------------------" << endl;
			return 0;
		}
		memset(bufRecv, 0, BUF_SIZE);
	}
}

int main() {
	//初始化DLL
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockAddr.sin_port = htons(12345);
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, Recv, (LPVOID)&sock, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, Send, (LPVOID)&sock, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	closesocket(sock);
	WSACleanup();
	return 0;
}