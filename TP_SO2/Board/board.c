#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define TAM 30
#define TAM_STR 100
#define MAX_EMPRESAS 30
#define NEMPRESAS_DISPLAY 10
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
	Empresa empresas[MAX_EMPRESAS];
}ArrayEmpresa;

//typedef struct {
//	DWORD in, out;
//	Empresa empresas[TAM];
//	DWORD indiceUltimaTransacao, numEmpresas;
//} SDATA;

typedef struct {
	BOOL continua;
	DWORD nDisplayEmp;
	HANDLE hMutex, hEv;
	Empresa empresas[MAX_EMPRESAS];
	//SDATA* shm;
} TDATA;

DWORD WINAPI atualizaTopEmpresas(LPVOID data) {

	TDATA* td = (TDATA*)data;
	//Empresa emp[MAX_EMPRESA];
	//ArrayEmpresa emp;
	DWORD i = 0, pos = 0;
	HANDLE hEv = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T("Event"));
	HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, _T("Mutex"));
	Empresa* pEmpresas;
	do {

		WaitForSingleObject(hEv, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);

		HANDLE hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, _T("Empresa"));
		pEmpresas = (Empresa*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Empresa) * NEMPRESAS_DISPLAY);

		system("cls");
		_tprintf(_T("Ações Mais Valiosas e Última Transação:\n"));
		_tprintf(_T("-------------------------------------------------------\n"));

		for (DWORD i = 0; i < td->nDisplayEmp; i++) {
			if (_tcslen(pEmpresas[i].nomeEmp) > 0) {
				_tprintf_s(_T("Empresa: %s, Ações: %u, Valor por Ação: %u\n"), pEmpresas[i].nomeEmp, pEmpresas[i].numAcoes, pEmpresas[i].valorAcao);
			}
		}

		//_tprintf(_T("-------------------------------------------------------\n"));
		//_tprintf_s(_T("Última Transação: %s, Ações: %d, Valor: %i\n"),
		//	pEmpresas[td->shm->indiceUltimaTransacao].nomeEmp,
		//	pEmpresas[td->shm->indiceUltimaTransacao].numAcoes,
		//	pEmpresas[td->shm->indiceUltimaTransacao].valorAcao);
		Sleep(2000);
		UnmapViewOfFile(pEmpresas);
		ReleaseMutex(hMutex);
		CloseHandle(hMap);

	} while (td->continua);

	CloseHandle(hEv);
	CloseHandle(hMutex);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMap, hThread, hSem;
	TDATA td;
	TDATA* pData;
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

	hSem = CreateSemaphore(NULL, MAX_EMPRESAS, MAX_EMPRESAS, _T("Sem"));
	//Esperar No semaforo (-1)
	WaitForSingleObject(hSem, INFINITE);

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TDATA), NOME_SM);
	if (hMap != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		primeiro = TRUE;
	}
	pData = (TDATA*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	//INICIAR
	td.hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	td.hEv = CreateEvent(NULL, TRUE, FALSE, _T("Event"));
	td.nDisplayEmp = displays;
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, atualizaTopEmpresas, &td, 0, NULL);
	_tprintf(_T("Criei a Thread"));
	do {
		_tscanf_s(_T("%s"), str, 40 - 1);
		_tprintf(_T("Estou no loop"));
	} while (_tcscmp(str, _T("fim")) != 0);
	td.continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(td.hMutex);
	CloseHandle(td.hEv);
	UnmapViewOfFile(pData);
	CloseHandle(hMap);
	ReleaseSemaphore(hSem, 1, NULL);
	_tprintf(_T("Board fechando..."));
	return 0;
}