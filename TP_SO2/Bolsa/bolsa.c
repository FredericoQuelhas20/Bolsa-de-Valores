#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_EMPRESAS 30
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
	TCHAR userName[TAM_STR];
	DWORD saldo;
	TCHAR password[TAM_STR];
	BOOL estado; //flag para ver se está ativo ou não
	CarteiraAcoes carteira;
} Cliente;

typedef struct {
	DWORD in, out;
	Empresa empresas[MAX_EMPRESAS];
	DWORD indiceUltimaTransacao, numEmpresas;
} SDATA;

typedef struct {
	BOOL continua;
	HANDLE hSemL, hMutexIn, hSemO;
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

void users(Cliente* cli, DWORD numCli) {
	//_tprintf(_T("Função ainda não implementada!"));
	_tprintf(_T("Lista de Utilizadores: \n"));
	for (DWORD i = 0; i < numCli; i++)
	{
		if (cli[i].estado == TRUE) {
			_tprintf(_T("User: %s, Saldo: %i, Online?: Ativo"), cli[i].userName, cli[i].saldo);
		}
		_tprintf(_T("User: %s, Saldo: %i, Online?: Inativo"), cli[i].userName, cli[i].saldo);
		
	}
}

void pause() {
	_tprintf(_T("Função ainda não implementada!"));
}

//COMANDO CLOSE

DWORD WINAPI comunicacaoBoard(LPVOID data) {

	TDATA* td = (TDATA*)data;
	Empresa emp[MAX_EMPRESAS];
	DWORD i = 0, pos = 0, contador = 0;

	do {
		geraEmpresa(emp);
		WaitForSingleObject(td->hSemL, INFINITE);
		WaitForSingleObject(td->hMutexIn, INFINITE);
		CopyMemory(&(td->shm->empresas[td->shm->in]), &emp, sizeof(Empresa) * MAX_EMPRESAS);
		pos = td->shm->in;
		td->shm->in = (td->shm->in + 1) % MAX_EMPRESAS;
		td->shm->indiceUltimaTransacao = (pos + 1) % MAX_EMPRESAS;
		ReleaseMutex(td->hMutexIn);
		ReleaseSemaphore(td->hSemO, 1, NULL);
		contador++;
		Sleep((rand() % (4 - 2 + 1) + 2) * 1000);

	} while (td->continua);

	_tprintf(_T("\nA Thread atualizou %d\n"), contador);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMap, hThread, hMutexOut;
	TDATA td;
	TCHAR str[40];
	BOOL primeiro = FALSE;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SDATA), NOME_SM);
	if (hMap != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		primeiro = TRUE;
	}
	td.shm = (SDATA*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	//INICIAR
	td.hMutexIn = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	hMutexOut = CreateMutex(NULL, FALSE, NOME_MUTEX_OUT);
	td.hSemL = CreateSemaphore(NULL, MAX_EMPRESAS, MAX_EMPRESAS, NOME_SEM_L);
	td.hSemO = CreateSemaphore(NULL, 0, MAX_EMPRESAS, NOME_SEM_O);

	if (primeiro) {
		WaitForSingleObject(td.hMutexIn, INFINITE);
		td.shm->in = 0;
		ReleaseMutex(td.hMutexIn);
		WaitForSingleObject(hMutexOut, INFINITE);
		td.shm->out = 0;
		ReleaseMutex(hMutexOut);

	}

	WaitForSingleObject(td.hMutexIn, INFINITE);
	ReleaseMutex(td.hMutexIn);
	_tprintf(_T("\nBOLSA A COMEÇAR... Escreva 'close' para terminar...\n"));
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, comunicacaoBoard, &td, 0, NULL);

	do {
		_tscanf_s(_T("%s"), str, 40 - 1);
	} while (_tcscmp(str, _T("close")) != 0);
	td.continua = FALSE;
	_tprintf(_T("\nBolsa fechando...\n"));
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(td.hSemO);
	CloseHandle(td.hSemL);
	CloseHandle(td.hMutexIn);
	CloseHandle(hMutexOut);
	UnmapViewOfFile(td.shm);
	CloseHandle(hMap);
	return 0;
}