#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define TAM 30
#define TAM_STR 100
#define MAX_EMPRESA 30
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
	DWORD in, out;
	Empresa empresas[TAM];
	DWORD indiceUltimaTransacao, numEmpresas;
} SDATA;

typedef struct {
	BOOL continua;
	DWORD nDisplayEmp;
	HANDLE hSemL, hMutexOut, hSemO;
	SDATA* shm;
} TDATA;

DWORD WINAPI atualizaTopEmpresas(LPVOID data) {

	TDATA* td = (TDATA*)data;
	Empresa emp[MAX_EMPRESA];
	DWORD i = 0, pos = 0;

	do {
		WaitForSingleObject(td->hSemO, INFINITE);
		WaitForSingleObject(td->hMutexOut, INFINITE);
		CopyMemory(&emp, &(td->shm->empresas[td->shm->out]), sizeof(Empresa) * MAX_EMPRESA);
		pos = td->shm->out;
		td->shm->out = (td->shm->out + 1) % TAM;
		ReleaseMutex(td->hMutexOut);
		ReleaseSemaphore(td->hSemL, 1, NULL);

		system("cls");
		_tprintf(_T("Ações Mais Valiosas e Última Transação:\n"));
		_tprintf(_T("-------------------------------------------------------\n"));

		for (DWORD i = 0; i < td->nDisplayEmp; i++) {
			if (_tcslen(td->shm->empresas[i].nomeEmp) > 0) {
				_tprintf_s(_T("Empresa: %s, Ações: %d, Valor por Ação: %i\n"), td->shm->empresas[i].nomeEmp, td->shm->empresas[i].numAcoes, td->shm->empresas[i].valorAcao);
			}
		}

		_tprintf(_T("-------------------------------------------------------\n"));
		_tprintf_s(_T("Última Transação: %s, Ações: %d, Valor: %i\n"),
			td->shm->empresas[td->shm->indiceUltimaTransacao].nomeEmp,
			td->shm->empresas[td->shm->indiceUltimaTransacao].numAcoes,
			td->shm->empresas[td->shm->indiceUltimaTransacao].valorAcao);

		Sleep((rand() % (4 - 2 + 1) + 2) * 1000);

	} while (td->continua);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMap, hThread, hMutexIn;
	TDATA td;
	TCHAR str[40];
	BOOL primeiro = FALSE;
	DWORD displays = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	if (argc > 1) {
		displays = _tstoi(argv[1]);
	}
	else {
		displays = 10;
	}

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SDATA), NOME_SM);
	if (hMap != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		primeiro = TRUE;
	}
	td.shm = (SDATA*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	//INICIAR
	td.hMutexOut = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	hMutexIn = CreateMutex(NULL, FALSE, NOME_MUTEX_OUT);
	td.hSemL = CreateSemaphore(NULL, TAM, TAM, NOME_SEM_L);
	td.hSemO = CreateSemaphore(NULL, 0, TAM, NOME_SEM_O);

	if (primeiro) {
		WaitForSingleObject(hMutexIn, INFINITE);
		//td.shm->p = 0;
		td.shm->in = 0;
		ReleaseMutex(hMutexIn);
		WaitForSingleObject(td.hMutexOut, INFINITE);
		//td.shm->c = 0;
		td.shm->out = 0;
		ReleaseMutex(td.hMutexOut);

	}

	WaitForSingleObject(td.hMutexOut, INFINITE);
	ReleaseMutex(td.hMutexOut);
	td.nDisplayEmp = displays;
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, atualizaTopEmpresas, &td, 0, NULL);

	do {
		_tscanf_s(_T("%s"), str, 40 - 1);
	} while (_tcscmp(str, _T("fim")) != 0);
	td.continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(td.hSemO);
	CloseHandle(td.hSemL);
	CloseHandle(td.hMutexOut);
	CloseHandle(hMutexIn);
	UnmapViewOfFile(td.shm);
	CloseHandle(hMap);
	_tprintf(_T("Board fechando..."));
	return 0;
}