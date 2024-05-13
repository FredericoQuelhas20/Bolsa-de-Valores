#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define MAX_PIPE_BUFFER_SIZE 1024
#define MAX_EMPRESAS 30
#define NEMPRESAS_DISPLAY 10
#define TAM_STR 100
#define NOME_SM		    _T("memória")
#define NOME_MUTEX_IN   _T("mutex_in")
#define NOME_MUTEX_OUT  _T("mutex_out")
#define NOME_SEM_L	    _T("sem_livres")
#define NOME_SEM_O      _T("sem_ocupados")

typedef struct {
	TCHAR nomeEmp[TAM_STR];
	DWORD valorAcao;
	DWORD numAcoes;
} Empresa;

typedef struct {
	Empresa emp[MAX_EMPRESAS];
} CarteiraAcoes;

typedef struct {
	TCHAR username[MAX_USERNAME_LENGTH];
	TCHAR password[MAX_PASSWORD_LENGTH];
	float saldo;
	BOOL estado;
	CarteiraAcoes carteira;
} Cliente;

//typedef struct {
//	TCHAR userName[TAM_STR];
//	DWORD saldo;
//	TCHAR password[TAM_STR];
//	BOOL estado; //flag para ver se está ativo ou não
//	CarteiraAcoes carteira;
//} Cliente;

typedef struct {
	DWORD in, out;
	Empresa empresas[MAX_EMPRESAS];
	DWORD indiceUltimaTransacao, numEmpresas;
} SDATA;

typedef struct {
	BOOL continua;
	HANDLE hMutex, hEv;
	SDATA* shm;
} TDATA;

void geraEmpresa(Empresa* emp) {
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		_tcscpy_s(emp[i].nomeEmp, sizeof(emp[i].nomeEmp), _T("EmpresaX"));
		emp[i].valorAcao = rand() % (99 - 10 + 1) + 10;
		emp[i].numAcoes = rand() % (99 - 10 + 1) + 10;
	}

}

void addc(Empresa* emp, DWORD numEmpresas, TCHAR* nomeEmpresa, DWORD precoAcao, DWORD numAcoes) {
	_tcscpy_s(emp[numEmpresas++].nomeEmp, sizeof(emp[numEmpresas++].nomeEmp), nomeEmpresa);
	emp[numEmpresas++].numAcoes = numAcoes;
	emp[numEmpresas++].valorAcao = precoAcao;
}

void listc(Empresa* emp) {
	_tprintf(_T("\nLista de Empresas Registadas:\n"));
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
			_tprintf_s(_T("Empresa: %s, Ações: %d, Valor por Ação: %i\n"), emp[i].nomeEmp, emp[i].numAcoes, emp[i].valorAcao);
	}
}

//void stock(Empresa* emp, TCHAR* nomeEmpresa, DWORD precoAcao) {
//	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
//		if (_tcscmp_s(emp[i].nomeEmp, nomeEmpresa) == 0) {
//			emp[i].valorAcao = precoAcao;
//			_tprintf(_T("Valor alterado com sucesso! Empresa: %s Valor (autualizado): %i"), emp[i].nomeEmp, emp[i].valorAcao);
//		}
//	}
//}

//void users(Cliente* cli, DWORD numCli) {
//	//_tprintf(_T("Função ainda não implementada!"));
//	_tprintf(_T("Lista de Utilizadores: \n"));
//	for (DWORD i = 0; i < numCli; i++)
//	{
//		if (cli[i].estado == TRUE) {
//			_tprintf(_T("User: %s, Saldo: %i, Online?: Ativo"), cli[i].userName, cli[i].saldo);
//		}
//		_tprintf(_T("User: %s, Saldo: %i, Online?: Inativo"), cli[i].userName, cli[i].saldo);
//		
//	}
//}

void readUserListAndSend(const TCHAR* filename, HANDLE pipeHandle) {
	HANDLE fileHandle = CreateFile(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Erro ao abrir o arquivo %s. Código de erro: %d\n"), filename, GetLastError());
		return;
	}

	Cliente cliente;
	TCHAR buffer[MAX_PIPE_BUFFER_SIZE];
	DWORD bytesRead;
	OVERLAPPED overlapped = { 0 };

	while (ReadFile(fileHandle, &cliente, sizeof(Cliente), &bytesRead, &overlapped) || GetLastError() == ERROR_IO_PENDING) {
		DWORD bytesWritten;

		// Aguardar a conclusão da operação de leitura
		WaitForSingleObject(fileHandle, INFINITE);

		if (GetOverlappedResult(fileHandle, &overlapped, &bytesRead, FALSE)) {
			// Construir a mensagem a ser enviada pelo named pipe
			_stprintf(buffer, _T("%s %s %.2f %d"), cliente.username, cliente.password, cliente.saldo, cliente.estado);

			// Enviar mensagem pelo named pipe
			if (!WriteFile(pipeHandle, buffer, _tcslen(buffer) * sizeof(TCHAR), &bytesWritten, NULL)) {
				_tprintf(_T("Erro ao escrever no named pipe. Código de erro: %d\n"), GetLastError());
				CloseHandle(fileHandle);
				return;
			}
		}
		else {
			_tprintf(_T("Erro ao realizar a operação de leitura. Código de erro: %d\n"), GetLastError());
			CloseHandle(fileHandle);
			return;
		}

		// Limpar a estrutura OVERLAPPED para a próxima operação
		ZeroMemory(&overlapped, sizeof(OVERLAPPED));
	}

	// Fechar o arquivo
	CloseHandle(fileHandle);
}

void pause() {
	_tprintf(_T("Função ainda não implementada!"));
}

//COMANDO CLOSE

DWORD WINAPI comunicacaoBoard(LPVOID data) {

	TDATA* td = (TDATA*)data;
	Empresa emp[MAX_EMPRESAS];
	DWORD i = 0, pos = 0, contador = 0;
	HANDLE hMap;
	Empresa* pEmpresas;

	do {
		geraEmpresa(emp);
		WaitForSingleObject(td->hEv, INFINITE);
		hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, _T("Empresa"));
		if (hMap == NULL) {
			_tprintf(_T("Erro ao abrir FileMapping.\n"));
			return 1;
		}
		pEmpresas = (Empresa*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Empresa) * NEMPRESAS_DISPLAY);
		if (pEmpresas == NULL) {
			_tprintf(_T("Erro ao mapear a visão do arquivo.\n"));
			CloseHandle(hMap);
			return 1;
		}
		WaitForSingleObject(td->hMutex, INFINITE);
		CopyMemory(pEmpresas, &emp, sizeof(Empresa) * NEMPRESAS_DISPLAY);
		ReleaseMutex(td->hMutex);
		UnmapViewOfFile(pEmpresas);
		Sleep(2000);
	} while (td->continua);

	//_tprintf(_T("\nA Thread atualizou %d\n"), contador);

	return 0;
}
int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMap, hThread, pipeHandle;
	TDATA td;
	TCHAR str[40];
	const TCHAR* pipeName = _T("\\\\.\\pipe\\UserListPipe");	

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	// Criar named pipe
	pipeHandle = CreateNamedPipe(
		pipeName,
		PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		MAX_PIPE_BUFFER_SIZE,
		MAX_PIPE_BUFFER_SIZE,
		0,
		NULL);

	if (pipeHandle == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Erro ao criar named pipe. Código de erro: %d\n"), GetLastError());
		return 1;
	}

	// Conectar ao cliente
	if (!ConnectNamedPipe(pipeHandle, NULL)) {
		_tprintf(_T("Erro ao conectar ao named pipe. Código de erro: %d\n"), GetLastError());
		CloseHandle(pipeHandle);
		return 1;
	}

	// Caminho completo do arquivo de usuários
	const TCHAR* userlistPath = _T("C:\\Users\\drll_\\source\\repos\\SO2_TP\\TP_SO2\\x64\\Debug\\userlist.txt");

	// Ler o arquivo de usuários e enviar os dados pelo named pipe
	readUserListAndSend(userlistPath, pipeHandle);	

	td.hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	td.hEv = CreateEvent(NULL, TRUE, TRUE, _T("Event"));
	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Empresa) * NEMPRESAS_DISPLAY, _T("Empresa"));
	if (hMap == NULL) {
		_tprintf(_T("Erro ao criar o mapeamento do arquivo.\n"));
		return 1;
	}

	_tprintf(_T("\nBOLSA A COMEÇAR... Escreva 'close' para terminar...\n"));
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, comunicacaoBoard, &td, 0, NULL);

	do {
		_tscanf_s(_T("%s"), str, 40 - 1);
	} while (_tcscmp(str, _T("close")) != 0);
	td.continua = FALSE;
	_tprintf(_T("\nBolsa fechando...\n"));
	WaitForSingleObject(hThread, INFINITE);	
	CloseHandle(pipeHandle);
	CloseHandle(td.hEv);
	CloseHandle(td.hMutex);
	CloseHandle(hMap);
	return 0;
}