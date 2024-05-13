#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

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
	Empresa empresas[MAX_EMPRESAS];
}ArrayEmpresa;

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

//typedef struct {
//	DWORD in, out;
//	Empresa empresas[MAX_EMPRESAS];
//	DWORD indiceUltimaTransacao, numEmpresas;
//} SDATA;

typedef struct {
	BOOL continua;
	HANDLE hMutex, hEv;
	Empresa empresas[MAX_EMPRESAS];
	//SDATA* shm;
} TDATA;

void geraEmpresa(Empresa* emp) {
	ArrayEmpresa* empresas;
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		_tcscpy_s(emp[i].nomeEmp, sizeof(emp[i].nomeEmp)/sizeof(emp[i].nomeEmp[0]), _T("EmpresaX"));
		emp[i].valorAcao = rand() % (99 - 10 + 1) + 10;
		emp[i].numAcoes = rand() % (99 - 10 + 1) + 10;
	}

}

void addc(Empresa* emp, DWORD* numEmpresas, TCHAR* nomeEmpresa, DWORD precoAcao, DWORD numAcoes) {

	_tcscpy_s(emp[*numEmpresas].nomeEmp, sizeof(emp[*numEmpresas].nomeEmp)/ sizeof(emp[*numEmpresas].nomeEmp[0]), nomeEmpresa);
	
	emp[*numEmpresas].numAcoes = numAcoes;
	emp[*numEmpresas].valorAcao = precoAcao;

	(*numEmpresas)++;

	_tprintf(_T("Empresa adicionada: %s, Ações: %lu, Preço por Ação: %lu\n"), nomeEmpresa, numAcoes, precoAcao);
}

void listc(Empresa* emp, DWORD* numEmpresas) {
	_tprintf(_T("\nLista de Empresas Registadas:\n"));
	for (DWORD i = 0; i < *numEmpresas; i++) {
			_tprintf_s(_T("Empresa: %s, Ações: %u, Valor por Ação: %u\n"), emp[i].nomeEmp, emp[i].numAcoes, emp[i].valorAcao);
	}
	_tprintf(_T("\n"));
	_tprintf(_T("\n"));
}

void stock(Empresa* emp, TCHAR* nomeEmpresa, DWORD precoAcao) {
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		if (_tcscmp(emp[i].nomeEmp, nomeEmpresa) == 0) {
			emp[i].valorAcao = precoAcao;
			_tprintf(_T("Valor alterado com sucesso! Empresa: %s Valor (autualizado): %i"), emp[i].nomeEmp, emp[i].valorAcao);
		}
	}
}

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
	//Empresa emp[MAX_EMPRESAS];
	DWORD i = 0, pos = 0, contador = 0;
	HANDLE hMap;
	Empresa* pEmpresas;
	//ArrayEmpresa empresas;

	do {
		//geraEmpresa(emp);
		WaitForSingleObject(td->hEv, INFINITE);
		hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, _T("Empresa"));
		if (hMap == NULL) {
			_tprintf(_T("Erro ao abrir FileMapping.\n"));
			return 1;
		}
		pEmpresas = (Empresa*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Empresa));
		if (pEmpresas == NULL) {
			_tprintf(_T("Erro ao mapear a visão do arquivo.\n"));
			CloseHandle(hMap);
			return 1;
		}
		WaitForSingleObject(td->hMutex, INFINITE);
		CopyMemory(pEmpresas, &td->empresas, sizeof(Empresa) * NEMPRESAS_DISPLAY);
		ReleaseMutex(td->hMutex);
		UnmapViewOfFile(pEmpresas);
		Sleep(2000);
	} while (td->continua);

	//_tprintf(_T("\nA Thread atualizou %d\n"), contador);

	return 0;
}
int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMap, hThread;
	TDATA td;
	TCHAR inputLine[256];
	//TCHAR str[40];
	TCHAR* context = NULL;
	TCHAR* token;
	TCHAR nomeEmpresa[TAM_STR];
	DWORD numeroAcoes = 0;
	DWORD precoAcao = 0;
	//Empresa emp[MAX_EMPRESAS];
	ArrayEmpresa empArray;
	DWORD* numEmpresas = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	td.hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	td.hEv = CreateEvent(NULL, TRUE, TRUE, _T("Event"));
	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TDATA), _T("Empresa"));
	if (hMap == NULL) {
		_tprintf(_T("Erro ao criar o mapeamento do arquivo.\n"));
		return 1;
	}

	_tprintf(_T("\nBOLSA A COMEÇAR... Escreva 'close' para terminar...\n"));
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, comunicacaoBoard, &td, 0, NULL);
	
	do {
		_tprintf(_T("Digite um comando: ")); // Prompt para entrada do usuário
		_fgetts(inputLine, sizeof(inputLine) / sizeof(inputLine[0]), stdin);

		// Remover a nova linha no final da entrada
		size_t ln = _tcslen(inputLine) - 1;
		if (inputLine[ln] == '\n')
			inputLine[ln] = '\0';

		token = _tcstok_s(inputLine, _T(" "), &context);
		if (token != NULL && _tcscmp(token, _T("addc")) == 0) {
			token = _tcstok_s(NULL, _T(" "), &context);
			if (token != NULL) {
				_tcscpy_s(nomeEmpresa, sizeof(nomeEmpresa) / sizeof(nomeEmpresa[0]), token);
				token = _tcstok_s(NULL, _T(" "), &context);
				if (token != NULL) {
					numeroAcoes = _tstoi(token);
					token = _tcstok_s(NULL, _T(" "), &context);
					if (token != NULL) {
						precoAcao = _tstoi(token);
						//_tprintf(_T("Empresa adicionada: %s, Ações: %lu, Preço por Ação: %lu\n"), nomeEmpresa, numeroAcoes, precoAcao);
						addc(td.empresas, &numEmpresas, nomeEmpresa, precoAcao, numeroAcoes);
					}
				}
			}
			
		}
		else {
			if (token != NULL && _tcscmp(token, _T("listc")) == 0) {
				listc(td.empresas, &numEmpresas);
			}
		}
	} while (_tcscmp(inputLine, _T("close")) != 0);
	td.continua = FALSE;
	_tprintf(_T("\nBolsa fechando...\n"));
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(td.hEv);
	CloseHandle(td.hMutex);
	CloseHandle(hMap);
	return 0;
}