#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>

#define MAX_EMPRESAS 10

typedef struct {
    TCHAR nomeEmpresa[256];
    DWORD numAcoes;
    float valorAcao;
} InformacaoAcao;

typedef struct {
    InformacaoAcao acoes[MAX_EMPRESAS];
    DWORD indiceUltimaTransacao;
} BufferCircular;

int _tmain(int argc, TCHAR* argv[]) {
    HANDLE hFileMap;
    BufferCircular* memPar;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif 

    // Tentar abrir o mapeamento de arquivo existente
    hFileMap = OpenFileMapping(FILE_MAP_READ, FALSE, _T("BolsaMemPartilhada"));
    if (hFileMap == NULL) {
        _tprintf(_T("Erro ao abrir o mapeamento do arquivo: %d\n"), GetLastError());
        return 1;
    }

    // Mapear a vista da memória partilhada
    memPar = (BufferCircular*)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, sizeof(BufferCircular));
    if (memPar == NULL) {
        _tprintf(_T("Erro ao mapear a vista do arquivo: %d\n"), GetLastError());
        CloseHandle(hFileMap);
        return 1;
    }

    // Exibir dados continuamente
    while (TRUE) {
        system("cls");  // Limpa a tela para uma melhor visualização
        _tprintf(_T("Ações Mais Valiosas e Última Transação:\n"));
        _tprintf(_T("-------------------------------------------------------\n"));

        for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
            if (_tcslen(memPar->acoes[i].nomeEmpresa) > 0) {  // Verifica se há dados na posição
                _tprintf(_T("Empresa: %s, Ações: %d, Valor por Ação: %.2f\n"), memPar->acoes[i].nomeEmpresa, memPar->acoes[i].numAcoes, memPar->acoes[i].valorAcao);
            }
        }

        _tprintf(_T("-------------------------------------------------------\n"));
        _tprintf(_T("Última Transação: %s, Ações: %d, Valor: %.2f\n"),
            memPar->acoes[memPar->indiceUltimaTransacao].nomeEmpresa,
            memPar->acoes[memPar->indiceUltimaTransacao].numAcoes,
            memPar->acoes[memPar->indiceUltimaTransacao].valorAcao);

        Sleep(5000);  // Atualiza a cada 5 segundos
    }

    // Limpeza, raramente alcançada devido ao loop infinito
    UnmapViewOfFile(memPar);
    CloseHandle(hFileMap);

    return 0;
}
