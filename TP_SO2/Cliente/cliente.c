#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define TAM 200
#define TAM_STR 20
#define MAX_UTILIZADORES 20
#define NOME_SM		    _T("memória")
#define NOME_MUTEX_IN   _T("mutex_in")
#define NOME_MUTEX_OUT  _T("mutex_out")
#define NOME_SEM_L	    _T("sem_livres")
#define NOME_SEM_O      _T("sem_ocupados")

typedef struct {
	TCHAR username[TAM_STR];
	TCHAR password[TAM_STR];
	DWORD saldo;
} Cliente;

typedef struct {
	DWORD in, out;
	Cliente clientes[TAM];
	DWORD indiceUltimaTransacao, numEmpresas;
} SDATA;

typedef struct {
	BOOL continua;
	DWORD nDisplayEmp;
	HANDLE hSemL, hMutexOut, hSemO;
	SDATA* shm;
} TDATA;

int _tmain(int argc, TCHAR* argv[]) {

	HKEY chave;
	DWORD queAconteceu, valorReg = 5;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//Criação ou Abertura do Registry com NCLIENTES
	LONG lResult = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\TPSO2\\NCLIENTES", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &queAconteceu);

	if (lResult != ERROR_SUCCESS) { 
		_tprintf(_TEXT("Erro ao criar/abrir chave no Registry (%d)\n"), GetLastError()); 
		return -1;
	}
	// Escrever o valor DWORD no registry
	lResult = RegSetValueExA(chave, "NCLIENTES", 0, REG_DWORD, (const BYTE*)&valorReg, sizeof(DWORD));
	if (lResult != ERROR_SUCCESS) {
		printf("Erro ao escrever valor DWORD no registry.\n");
		RegCloseKey(chave);
		return 1;
	}

	RegCloseKey(chave);


	return 0;
}