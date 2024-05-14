#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX_EMPRESAS 30
#define TAM_CLIENTES 20
#define NEMPRESAS_DISPLAY 10
#define TAM_STR 100
#define NOME_SM		    _T("memória")
#define NOME_MUTEX_IN   _T("mutex_in")
#define NOME_MUTEX_OUT  _T("mutex_out")


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
	BOOL continua;
	HANDLE hMutexMemPar, hEvMemPar;
	Empresa empresas[MAX_EMPRESAS];
	DWORD pauseComando;
} TDATA;


int isStockArrayInitialized(Empresa* stocks, DWORD numEmpresas) {
	for (DWORD i = 0; i < numEmpresas; i++) {
		if (stocks[i].valorAcao > 0) {
			return 1; 
		}
	}
	return 0;
}


void sortStocks(Empresa* empresas, DWORD numEmpresas) {
	DWORD i, j;
	Empresa temp;
	for (i = 0; i < numEmpresas - 1; i++) {
		for (j = 0; j < numEmpresas - i - 1; j++) {
			if (empresas[j].valorAcao > empresas[j + 1].valorAcao) {
				temp = empresas[j];
				empresas[j] = empresas[j + 1];
				empresas[j + 1] = temp;
			}
		}
	}
}

void lerFicheiroClientes(Cliente* clientes, TCHAR* nomeFich, DWORD* numClientes) {
	FILE* file;
	errno_t err;

	err = _tfopen_s(&file, nomeFich, _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf(_T("Erro ao abrir o arquivo!\n"));
		return;
	}

	TCHAR buffer[100];
	int i = 0;
	while (_fgetts(buffer, sizeof(buffer) / sizeof(TCHAR), file) != NULL) {
		_stscanf_s(buffer, _T("%99s %99s %u"), clientes[i].userName, 100, clientes[i].password, 100, &clientes[i].saldo);
		i++;
	}

	*numClientes = i + 1;

	fclose(file);
}

void lerFicheiroEmpresas(Empresa* empresas, TCHAR* nomeFich, DWORD* numEmpresas) {
	FILE* file;
	errno_t err;

	err = _tfopen_s(&file, nomeFich, _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf(_T("Erro ao abrir o arquivo!\n"));
		return;
	}

	TCHAR buffer[100];
	int i = 0;
	while (_fgetts(buffer, sizeof(buffer) / sizeof(TCHAR), file) != NULL) {
		_stscanf_s(buffer, _T("%99s %d %u"), empresas[i].nomeEmp, 100, &empresas[i].numAcoes, &empresas[i].valorAcao);
		i++;
	}
	
	*numEmpresas = i+1;

	fclose(file);
}

void addc(Empresa* emp, DWORD* numEmpresas, TCHAR* nomeEmpresa, DWORD precoAcao, DWORD numAcoes) {

	_tcscpy_s(emp[*numEmpresas].nomeEmp, sizeof(emp[*numEmpresas].nomeEmp)/ sizeof(emp[*numEmpresas].nomeEmp[0]), nomeEmpresa);
	
	emp[*numEmpresas].numAcoes = numAcoes;
	emp[*numEmpresas].valorAcao = precoAcao;

	(*numEmpresas)++;

	_tprintf(_T("\nEmpresa adicionada: %s, Ações: %lu, Preço por Ação: %lu\n"), nomeEmpresa, numAcoes, precoAcao);
}

void listc(Empresa* emp, DWORD* numEmpresas) {
	_tprintf(_T("\nLista de Empresas Registadas:\n"));
	for (DWORD i = 0; i < *numEmpresas - 1; i++) {
			_tprintf_s(_T("Empresa: %s, Ações: %u, Valor por Ação: %u\n"), emp[i].nomeEmp, emp[i].numAcoes, emp[i].valorAcao);
	}
	_tprintf(_T("\n"));
	_tprintf(_T("\n"));
}

void stock(Empresa* emp, TCHAR* nomeEmpresa, DWORD precoAcao, DWORD* numEmpresas) {
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		if (_tcscmp(emp[i].nomeEmp, nomeEmpresa) == 0) {
			emp[i].valorAcao = precoAcao;
			_tprintf(_T("\nValor alterado com sucesso! Empresa: %s Valor (autualizado): %u\n"), emp[i].nomeEmp, emp[i].valorAcao);
		}
	}
}

void users(Cliente* cli, DWORD* numCli) {
	//NÃO MOSTRAR PASSWORD!!
	_tprintf(_T("\nLista de Utilizadores: \n"));
	for (DWORD i = 0; i < *numCli - 1; i++)
	{
		/*if (cli[i].estado == TRUE) {
			_tprintf(_T("User: %s, Saldo: %u, Online?: Ativo"), cli[i].userName, cli[i].saldo);
		}
		_tprintf(_T("User: %s, Saldo: %u, Online?: Inativo"), cli[i].userName, cli[i].saldo);*/
		_tprintf(_T("User: %s, Saldo: %u, PassWord: %s\n"), cli[i].userName, cli[i].saldo, cli[i].password);
		
	}
	_tprintf(_T("\n"));
	_tprintf(_T("\n"));
}

DWORD WINAPI comunicacaoBoard(LPVOID data) {

	TDATA* td = (TDATA*)data;
	DWORD i = 0, pos = 0, contador = 0;
	HANDLE hMap;
	Empresa* pEmpresas;

	do {
		if (isStockArrayInitialized(td->empresas, sizeof(td->empresas) / sizeof(td->empresas[0]))) {
			sortStocks(td->empresas, sizeof(td->empresas) / sizeof(td->empresas[0]));
		}

		if (td->pauseComando != 0) {
			Sleep(td->pauseComando * 1000);
			_tprintf(_T("\nBoard em pausa de %u segundos\n"), td->pauseComando);
		}

		td->pauseComando = 0;

		WaitForSingleObject(td->hEvMemPar, INFINITE);
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
		WaitForSingleObject(td->hMutexMemPar, INFINITE);
		CopyMemory(pEmpresas, &td->empresas, sizeof(Empresa) * NEMPRESAS_DISPLAY);
		ReleaseMutex(td->hMutexMemPar);
		UnmapViewOfFile(pEmpresas);
		Sleep(2000);
	} while (td->continua);

	return 0;
}
int _tmain(int argc, TCHAR* argv[]) {

	//Variáveis para tratar do mapeamento e da Thread
	HANDLE hMap, hThreadMemPar;
	TDATA td;
	//Varíaveis para tratar de comandos
	TCHAR inputLine[256];
	TCHAR* context = NULL;
	TCHAR* token;
	//Variáveis para tratar dos comandos addc, lerEmpresas,...
	TCHAR nomeEmpresa[TAM_STR];
	TCHAR nomeFich[TAM_STR];
	DWORD numeroAcoes = 0;
	DWORD precoAcao = 0;
	DWORD* numEmpresas = 0;
	DWORD* numClientes = 0;
	//Array de Clientes
	Cliente clientes[TAM_CLIENTES];
	td.pauseComando = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	td.hMutexMemPar = CreateMutex(NULL, FALSE, NOME_MUTEX_IN);
	td.hEvMemPar = CreateEvent(NULL, TRUE, TRUE, _T("Event"));
	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TDATA), _T("Empresa"));
	if (hMap == NULL) {
		_tprintf(_T("Erro ao criar o mapeamento do arquivo.\n"));
		return 1;
	}

	_tprintf(_T("\nBOLSA A COMEÇAR... Escreva 'close' para terminar...\n"));
	td.continua = TRUE;
	hThreadMemPar = CreateThread(NULL, 0, comunicacaoBoard, &td, 0, NULL);
	
	do {
		_tprintf(_T("[BOLSA]> Digite um comando: "));
		_fgetts(inputLine, sizeof(inputLine) / sizeof(inputLine[0]), stdin);

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
						addc(td.empresas, &numEmpresas, nomeEmpresa, precoAcao, numeroAcoes);
					}
				}
			}
			
		}
		else {
			if (token != NULL && _tcscmp(token, _T("listc")) == 0) {
				listc(td.empresas, &numEmpresas);
			}
			else {
				if (token != NULL && _tcscmp(token, _T("stock")) == 0) {
					token = _tcstok_s(NULL, _T(" "), &context);
					if (token != NULL) {
						_tcscpy_s(nomeEmpresa, sizeof(nomeEmpresa) / sizeof(nomeEmpresa[0]), token);
						token = _tcstok_s(NULL, _T(" "), &context);
						if (token != NULL) {
								precoAcao = _tstoi(token);
								stock(td.empresas, nomeEmpresa, precoAcao, &numEmpresas);
							
						}
					}

				}
				else {
					if (token != NULL && _tcscmp(token, _T("lerEmpresas")) == 0) {
						token = _tcstok_s(NULL, _T(" "), &context);
						if (token != NULL) {
							_tcscpy_s(nomeFich, sizeof(nomeFich) / sizeof(nomeFich[0]), token);
							lerFicheiroEmpresas(td.empresas, nomeFich, &numEmpresas);
							
						}

					}
					else {
						if (token != NULL && _tcscmp(token, _T("lerClientes")) == 0) {
							token = _tcstok_s(NULL, _T(" "), &context);
							if (token != NULL) {
								_tcscpy_s(nomeFich, sizeof(nomeFich) / sizeof(nomeFich[0]), token);
								lerFicheiroClientes(clientes, nomeFich, &numClientes);
							}

						}

						else {
							if (token != NULL && _tcscmp(token, _T("users")) == 0) {
								users(clientes, &numClientes);
							}
							else {
								if (token != NULL && _tcscmp(token, _T("pause")) == 0) {
									token = _tcstok_s(NULL, _T(" "), &context);
									if (token != NULL) {
										td.pauseComando = _tstoi(token);
									}

								}
								else {
									if (token != NULL && _tcscmp(token, _T("cls")) == 0) {
										system("cls");
									}
								}
							}
						}
					}
				}
			}
		}
	} while (_tcscmp(inputLine, _T("close")) != 0);
	td.continua = FALSE;
	_tprintf(_T("\nBolsa fechando...\n"));
	WaitForSingleObject(hThreadMemPar, INFINITE);
	CloseHandle(td.hEvMemPar);
	CloseHandle(td.hMutexMemPar);
	CloseHandle(hMap);
	return 0;
}
