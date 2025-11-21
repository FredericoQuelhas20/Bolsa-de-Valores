
# Bolsa de Valores

## Descrição do Projeto
Este projeto tem como objetivo implementar um sistema de simulação de uma Bolsa de Valores, desenvolvido em C no âmbito da unidade curricular de Sistemas Operativos II (SO2). O sistema é composto por múltiplos processos que comunicam entre si, incluindo uma Bolsa central, Clientes e um Board de visualização, utilizando mecanismos de sincronização e comunicação inter-processos do Windows.

## Equipa de Desenvolvimento
- Frederico Quelhas
- Diego Lima

## Tecnologias Utilizadas
- **C** (linguagem de programação)
- **Windows API** para gestão de processos e threads
- **Named Pipes** para comunicação inter-processos
- **Memória Partilhada** para sincronização de dados
- **Mutexes e Semáforos** para controlo de concorrência
- **Visual Studio** como IDE

## Funcionalidades
- Gestão centralizada de uma bolsa de valores com múltiplas empresas
- Sistema de transações de compra e venda de ações
- Comunicação entre múltiplos clientes e a bolsa
- Board de visualização em tempo real do estado da bolsa
- Sincronização de processos utilizando primitivas do Windows
- Gestão de carteiras de investimento dos clientes
- Sistema de ordens de compra/venda (limit orders e market orders)
- Atualização automática de preços de ações
- Notificações de transações realizadas

## Instalação

### Requisitos
- Sistema Operativo Windows (Windows 10 ou superior)
- Visual Studio 2019 ou superior
- Windows SDK instalado
- Compilador C compatível com Windows API

### Passos de Instalação

1. Clone o repositório do GitHub:
```bash
git clone https://github.com/FredericoQuelhas20/Bolsa-de-Valores.git
cd Bolsa-de-Valores
```

2. Abra a solução no Visual Studio:
```
TP_SO2/TP_SO2.sln
```

3. Configure o projeto para a arquitetura x64 (recomendado)

4. Compile a solução:
   - Build → Build Solution (Ctrl+Shift+B)

## Estrutura do Projeto
```
/TP_SO2              - Solução principal do Visual Studio
  ├── Bolsa/         - Código-fonte do servidor da Bolsa
  ├── Cliente/       - Código-fonte da aplicação Cliente
  ├── Board/         - Código-fonte do Board de visualização
  └── TP_SO2.sln     - Ficheiro de solução do Visual Studio
/Diagramas_Programas - Diagramas e esquemas do projeto
/so2-2324-tp-m1-*.pdf - Relatório do trabalho prático
/SO2 - 2324 - TP.pdf  - Enunciado do trabalho prático
README.md            - Documentação do projeto
```

## Como Utilizar

### Iniciar o Sistema

1. **Iniciar a Bolsa (Servidor)**:
```bash
cd TP_SO2/x64/Debug
Bolsa.exe
```

2. **Iniciar o Board (Visualização)**:
```bash
cd TP_SO2/x64/Debug
Board.exe
```

3. **Iniciar Cliente(s)**:
```bash
cd TP_SO2/x64/Debug
Cliente.exe
```

### Comandos do Cliente
O cliente suporta diversos comandos para interagir com a bolsa:

- **login** - Autenticar na bolsa
- **listcompany** - Listar empresas disponíveis
- **buy [empresa] [quantidade] [preço]** - Ordem de compra
- **sell [empresa] [quantidade] [preço]** - Ordem de venda
- **balance** - Consultar saldo e carteira
- **pause** - Pausar transações
- **exit** - Sair do sistema

### Arquitetura do Sistema

**Bolsa (Servidor)**:
- Processo principal que coordena todas as transações
- Mantém registo de todas as empresas e suas ações
- Processa ordens de compra e venda
- Sincroniza estado com o Board

**Cliente**:
- Interface de utilizador para investidores
- Comunica com a Bolsa via Named Pipes
- Mantém carteira de ações local
- Recebe notificações de transações

**Board**:
- Visualização em tempo real do estado da bolsa
- Atualização automática de preços
- Display de transações ativas
- Acesso via memória partilhada

## Compilação

### Via Visual Studio
1. Abrir `TP_SO2.sln`
2. Selecionar configuração (Debug/Release)
3. Selecionar plataforma (x64 recomendado)
4. Build → Build Solution

### Via Linha de Comandos (Developer Command Prompt)
```bash
cd TP_SO2
msbuild TP_SO2.sln /p:Configuration=Release /p:Platform=x64
```

## Mecanismos de Sincronização

O projeto utiliza diversos mecanismos de sincronização do Windows:

- **Named Pipes**: Comunicação entre Bolsa e Clientes
- **Memória Partilhada**: Sincronização entre Bolsa e Board
- **Mutexes**: Proteção de secções críticas
- **Semáforos**: Controlo de acesso a recursos partilhados
- **Events**: Notificação de eventos entre processos

## Funcionalidades Avançadas

- **Persistência de Dados**: Guardar estado da bolsa
- **Multi-threading**: Suporte para múltiplos clientes simultâneos
- **Transações Atómicas**: Garantia de consistência nas operações
- **Recuperação de Falhas**: Sistema robusto com tratamento de erros
- **Logs de Auditoria**: Registo de todas as transações

## Diagrama do Sistema

Consulte a pasta `/Diagramas_Programas` para visualizar:
- Arquitetura do sistema
- Fluxo de comunicação entre processos
- Diagramas de estados
- Modelos de dados

## Documentação Adicional

- **Relatório**: `so2-2324-tp-m1-DiegoLima-2022110909-FredericoQuelhas-2022135081.pdf`

## Testes

Para testar o sistema:

1. Inicie a Bolsa
2. Inicie o Board para visualização
3. Inicie múltiplos clientes em terminais separados
4. Execute transações de compra/venda
5. Verifique sincronização no Board
6. Teste cenários de concorrência

## Resolução de Problemas

**Erro de Named Pipe**:
- Certifique-se de que a Bolsa está em execução primeiro
- Verifique permissões do sistema

**Erro de Memória Partilhada**:
- Execute com privilégios de administrador se necessário
- Verifique se não há instâncias anteriores em execução

## Licença
Este projeto está licenciado sob a Licença MIT - veja o ficheiro LICENSE para mais detalhes.

## Contacto
Para questões:
- **Frederico Quelhas** - [@FredericoQuelhas20](https://github.com/FredericoQuelhas20)


---

**Nota:** Projeto académico desenvolvido para a unidade curricular de Sistemas Operativos II (SO2) - 2023/2024.
