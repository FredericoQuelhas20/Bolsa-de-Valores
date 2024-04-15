#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>

#define MAX_EMPRESAS 10

typedef struct {
    TCHAR nomeEmpresa[100];
    DWORD numAcoes;
    float valorAcao;
} InformacaoAcao;

typedef struct {
    InformacaoAcao acoes[MAX_EMPRESAS];
    int indiceUltimaTransacao;
} BufferCircular;

typedef struct {
    BufferCircular* memPar;
    HANDLE hSemEscrita;
    HANDLE hSemLeitura;
    HANDLE hMutex;
    int terminar;
    int id;
} DadosThreads;

int num_aleatorio(int min, int max) {
    return rand() % (max - min + 1) + min;
}

DWORD WINAPI ThreadAtualizaBolsa(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    InformacaoAcao infoAcao;
    int contador = 0;

    while (!dados->terminar) {
        _tcscpy_s(infoAcao.nomeEmpresa, sizeof(infoAcao.nomeEmpresa), _T("EmpresaX"));
        infoAcao.numAcoes = num_aleatorio(1, 100);
        infoAcao.valorAcao = num_aleatorio(10, 99) + 0.99;

        WaitForSingleObject(dados->hSemEscrita, INFINITE);
        WaitForSingleObject(dados->hMutex, INFINITE);

        int pos = dados->memPar->indiceUltimaTransacao;
        CopyMemory(&dados->memPar->acoes[pos], &infoAcao, sizeof(InformacaoAcao));
        dados->memPar->indiceUltimaTransacao = (pos + 1) % MAX_EMPRESAS;

        ReleaseMutex(dados->hMutex);
        ReleaseSemaphore(dados->hSemLeitura, 1, NULL);

        contador++;
        Sleep(num_aleatorio(1, 5) * 1000);
    }

    _tprintf(_T("Thread atualizou %d vezes.\n"), contador);
    return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
    HANDLE hFileMap;
    HANDLE hThread;
    DadosThreads dados;
    BOOL primeiroProcesso = FALSE;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif 

    srand((unsigned int)time(NULL));

    dados.hSemEscrita = CreateSemaphore(NULL, MAX_EMPRESAS, MAX_EMPRESAS, _T("BolsaSemEscrita"));
    dados.hSemLeitura = CreateSemaphore(NULL, 0, MAX_EMPRESAS, _T("BolsaSemLeitura"));
    dados.hMutex = CreateMutex(NULL, FALSE, _T("BolsaMutex"));

    hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("BolsaMemPartilhada"));
    if (hFileMap == NULL) {
        primeiroProcesso = TRUE;
        hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BufferCircular), _T("BolsaMemPartilhada"));
    }

    dados.memPar = (BufferCircular*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (primeiroProcesso) {
        ZeroMemory(dados.memPar, sizeof(BufferCircular));
    }

    dados.terminar = 0;
    hThread = CreateThread(NULL, 0, ThreadAtualizaBolsa, &dados, 0, NULL);

    _tprintf(_T("Pressione qualquer tecla para terminar...\n"));
    _gettchar();
    dados.terminar = 1;

    WaitForSingleObject(hThread, INFINITE);
    UnmapViewOfFile(dados.memPar);
    CloseHandle(hFileMap);
    CloseHandle(dados.hSemEscrita);
    CloseHandle(dados.hSemLeitura);
    CloseHandle(dados.hMutex);

    return 0;
}
