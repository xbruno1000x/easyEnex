# 🟦 easyEnEx

**Sistema modular de EnEx (Entradas e Saídas) para SA-MP**

Desenvolvido por: **xBruno1000x**  
Versão atual: **1.8.0** (easyEnex) | **1.3.0** (easyHouses)

---

## 📋 Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Características](#-características)
- [Instalação](#-instalação)
- [Uso Básico - easyEnex](#-uso-básico---easyenex)
- [Módulo easyHouses](#-módulo-easyhouses)
- [Documentação Completa](#-documentação-completa)
- [Dependências](#-dependências)
- [Exemplos](#-exemplos)
- [Contribuindo](#-contribuindo)
- [Licença](#-licença)

---

## 🎯 Sobre o Projeto

**easyEnex** é uma biblioteca completa e modular para gerenciamento de entradas e saídas (EnEx) em servidores SA-MP. Desenvolvida com foco em **simplicidade, performance e extensibilidade**, permite criar sistemas complexos de teleporte com apenas algumas linhas de código.

### Por que usar easyEnex?

✅ **Simples e Intuitivo** - API clara e fácil de usar  
✅ **Modular** - Use apenas o que precisa  
✅ **Extensível** - Callbacks e hooks para total customização  
✅ **Performance** - Otimizado com YSI Iterators  
✅ **Completo** - Inclui módulo de casas pronto para uso  
✅ **Bem Documentado** - Documentação detalhada e exemplos práticos  

---

## ⭐ Características

### easyEnex (Core)
- 🚪 Criação ilimitada de entradas e saídas
- 🔒 Sistema de abertura/fechamento de EnEx
- 🎯 Detecção automática de proximidade
- 🔑 Tecla customizável (padrão: F)
- 🎨 Pickups personalizáveis
- 📡 3 callbacks para total controle
- 🌍 Suporte a Virtual Worlds e Interiors
- ❄️ Opção de congelar player ao teleportar

### easyHouses (Módulo Opcional)
- 🏠 **Sistema completo de casas**
- 💰 Compra e venda com retorno de 70%
- 🔑 Aluguel com pagamento automático por dia
- 🔄 Transferência de propriedade entre jogadores
- 🏘️ Múltiplas casas por jogador (configurável)
- 🌐 Virtual Worlds únicos (sem conflitos)
- 💾 Salvamento automático (DOF2)
- 🔐 Sistema de trancamento
- 💼 Armazenamento de armas e dinheiro
- 📊 Labels 3D dinâmicas
- 🚨 Despejo automático por falta de pagamento
- 📈 Acumulação de aluguel para proprietário

---

## 📦 Instalação

### Via sampctl (Recomendado)

```bash
sampctl package install xbruno1000x/easyEnEx
```

### Manual

1. Baixe o repositório
2. Copie `easyEnex.inc` para sua pasta `pawno/include/`
3. (Opcional) Copie `extras/easyHouses.inc` para `pawno/include/extras/`
4. (Opcional) Copie `extras/DOF2.inc` para `pawno/include/` (para salvamento)

---

## 🚀 Uso Básico - easyEnex

### 1️⃣ Incluir no Gamemode

```pawn
#include <a_samp>
#include <YSI_Data\y_iterate>
#include <YSI_Coding\y_hooks>
#include <streamer>
#include <easyEnex>
```

### 2️⃣ Criar EnEx

```pawn
public OnGameModeInit()
{
    // Criar entrada/saída simples
    Enex_Create(
        "Minha Casa",                     // Nome
        1234.0, -567.0, 20.0, 0.0,        // Entrada (X, Y, Z, Ângulo)
        200.0, 300.0, 10.0, 180.0,        // Saída (X, Y, Z, Ângulo)
        0,                                 // Virtual World
        0,                                 // Interior
        true                               // Congelar ao teleportar
    );
    
    return 1;
}
```

### 3️⃣ Usar Callbacks (Opcional)

```pawn
public OnPlayerEnterEnEx(playerid, enexid)
{
    new string[64];
    format(string, sizeof(string), "Você entrou no EnEx ID: %d", enexid);
    SendClientMessage(playerid, -1, string);
    return 1;
}

public OnPlayerExitEnEx(playerid, enexid)
{
    SendClientMessage(playerid, -1, "Você saiu do EnEx!");
    return 1;
}

public OnPlayerDeniedEnEx(playerid, enexid)
{
    SendClientMessage(playerid, -1, "Esta entrada está fechada!");
    return 1;
}
```

### 4️⃣ Funções Principais

```pawn
// Gerenciamento
Enex_Create(name[], Float:entX, Float:entY, Float:entZ, Float:entAng, 
            Float:exX, Float:exY, Float:exZ, Float:exAng, 
            vwid = 0, intid = 0, bool:freeze = false)
Enex_Delete(enexid)
Enex_Exists(enexid)

// Abertura/Fechamento
Enex_Open(enexid)
Enex_Close(enexid)
Enex_IsClosed(enexid)

// Controle de Acesso
Enex_OpenForPlayer(playerid, enexid)
Enex_CloseForPlayer(playerid, enexid)
Enex_IsOpenForPlayer(playerid, enexid)

// Teleporte Manual
SetPlayerPosEnEx(playerid, Float:x, Float:y, Float:z, Float:angle, 
                 vwid = 0, intid = 0)
```

### 5️⃣ Personalização

```pawn
// Antes de incluir easyEnex
#define MSG_ENTRADA_FECHADA "Entrada trancada!"
#define DEFAULT_PICKUPID    1318  // Ícone diferente
#define DEFAULT_KEY         KEY_SECONDARY_ATTACK  // Botão direito do mouse

#include <easyEnex>
```

---

## 🏠 Módulo easyHouses

Sistema completo de casas construído sobre easyEnex.

### Instalação

```pawn
#include <a_samp>
#include <YSI_Data\y_iterate>
#include <YSI_Coding\y_hooks>
#include <YSI_Extra\y_timers_foreach>
#include <streamer>
#include <DOF2>              // Opcional (salvamento automático)
#include <zcmd>              // Opcional (comandos)
#include <sscanf2>           // Opcional (comando transferir)
#include <easyEnex>
#include <extras/easyHouses> // ← Módulo de casas
```

### Configuração Rápida

```pawn
// 1. Criar pasta para salvamento
// scriptfiles/houses/

// 2. Criar casas
public OnGameModeInit()
{
    House_Create(
        "Casa da Grove Street",
        50000,                        // Preço compra
        500,                          // Aluguel/dia (0 = sem aluguel)
        2495.5, -1688.5, 13.5, 0.0,  // Entrada
        2496.0, -1695.0, 1014.7, 0.0,// Interior
        3                             // Interior ID
    );
    
    House_LoadAll(); // Carregar casas salvas
    return 1;
}

// 3. Adicionar comandos
CMD:comprarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1) return SendClientMessage(playerid, -1, "Sem casa por perto!");
    House_Buy(playerid, houseid);
    return 1;
}

CMD:vendercasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(!House_IsOwner(playerid, houseid)) 
        return SendClientMessage(playerid, -1, "Você não é dono!");
    House_Sell(playerid, houseid);
    return 1;
}

CMD:trancar(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(!House_IsOwner(playerid, houseid)) 
        return SendClientMessage(playerid, -1, "Você não é dono!");
    House_ToggleLock(playerid, houseid);
    return 1;
}

// Aluguel
CMD:alugarcasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1) return SendClientMessage(playerid, -1, "Sem casa por perto!");
    House_Rent(playerid, houseid);
    return 1;
}

CMD:desalugar(playerid)
{
    new houseid = PlayerRentHouseID[playerid];
    if(houseid == -1) return SendClientMessage(playerid, -1, "Você não aluga nenhuma casa!");
    House_Unrent(playerid, houseid);
    return 1;
}

// Transferência (requer sscanf2)
CMD:transferircasa(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, -1, "Uso: /transferircasa [id]");
    
    new houseid = House_GetNearby(playerid);
    if(!House_IsOwner(playerid, houseid))
        return SendClientMessage(playerid, -1, "Você não é dono!");
    
    House_Transfer(playerid, targetid, houseid);
    return 1;
}
```

### Funcionalidades do easyHouses

#### 💰 Compra e Venda
- Jogadores compram casas disponíveis
- Venda retorna 70% do valor
- Limite de casas por jogador (padrão: 3)

#### 🔑 Sistema de Aluguel
- Proprietários definem preço de aluguel por dia
- **Pagamento automático** ao conectar no servidor
- Cálculo de dias: `(gettime() - lastPayment) / 86400`
- **Despejo automático** se não tiver dinheiro
- Dono coleta aluguel acumulado

**Exemplo de Fluxo:**
```
DIA 1: Player aluga por $500/dia
  → Paga $500 (primeiro dia)

DIA 4: Player conecta (3 dias depois)
  → Sistema cobra: 3 × $500 = $1500
  → Se tem: paga automaticamente
  → Se não tem: despejado

Dono coleta:
  → /coletaraluguel
  → Recebe todo aluguel acumulado
```

#### 🔄 Transferência
- Transferir propriedade entre jogadores
- Verifica limite de casas do destinatário
- Cancela aluguel automaticamente

#### 🏘️ Múltiplas Casas
- Cada jogador pode ter até 3 casas (configurável)
- Sistema de slots organizado
- Funções para listar e gerenciar

#### 🌐 Virtual Worlds Únicos
- Cada casa tem VW próprio (`houseid + 1000`)
- Permite usar mesmo interior em várias casas
- Sem conflito de saída

#### 💾 Salvamento Automático
- **DOF2** integrado (detecção automática)
- Salva em: `scriptfiles/houses/house_X.ini`
- Campos salvos: owner, locked, renter, rent days, armas, dinheiro
- Callbacks para sistemas customizados (MySQL, SQLite)

### API Completa easyHouses

```pawn
// Criação e Gerenciamento
House_Create(name[], price, rentPrice, Float:entX, entY, entZ, entAng, 
             Float:exX, exY, exZ, exAng, interiorID)
House_Delete(houseid)
House_GetNearby(playerid, Float:range = 3.0)
House_IsOwner(playerid, houseid)
House_UpdateLabel(houseid)

// Compra/Venda
House_Buy(playerid, houseid)
House_Sell(playerid, houseid)
House_ToggleLock(playerid, houseid)

// Aluguel
House_Rent(playerid, houseid)
House_Unrent(playerid, houseid)
House_EvictRenter(playerid, houseid)
House_CollectRent(playerid, houseid)
House_ProcessRentPayment(playerid, houseid)

// Transferência
House_Transfer(playerid, targetid, houseid)

// Múltiplas Casas
House_GetPlayerHouseCount(playerid)
House_GetPlayerHouseBySlot(playerid, slot)
House_PlayerOwnsHouse(playerid, houseid)

// Salvamento
House_Save(houseid)
House_Load(houseid)
House_SaveAll()
House_LoadAll()
House_LoadPlayerHouses(playerid)
```

### Callbacks easyHouses

```pawn
// Compra/Venda
OnPlayerBuyHouse(playerid, houseid)
OnPlayerSellHouse(playerid, houseid)

// Entrada/Saída
OnPlayerEnterHouse(playerid, houseid)
OnPlayerExitHouse(playerid, houseid)

// Aluguel
OnPlayerRentHouse(playerid, houseid)
OnPlayerUnrentHouse(playerid, houseid)

// Transferência
OnPlayerTransferHouse(playerid, targetid, houseid)

// Criação/Destruição
OnHouseCreated(houseid)
OnHouseDeleted(houseid)

// Salvamento Customizado
OnHouseSave(houseid)    // Implemente para MySQL/SQLite
OnHouseLoad(houseid)    // Retorne 0 para usar DOF2
```

---

## 📚 Documentação Completa

### easyEnex (Core)
- 📖 Documentação inline no código
- 🎮 [demo/demoenex.pwn](demo/demoenex.pwn) - Exemplo completo

### easyHouses (Módulo)
- 📘 [extras/GUIA_IMPLEMENTACAO.md](extras/GUIA_IMPLEMENTACAO.md) - Guia passo a passo
- 📗 [extras/FUNCIONALIDADES.md](extras/FUNCIONALIDADES.md) - Todas as funcionalidades explicadas
- 📝 [extras/CHANGELOG_HOUSES.md](extras/CHANGELOG_HOUSES.md) - Histórico de versões
- 🎮 [extras/demo_houses.pwn](extras/demo_houses.pwn) - Gamemode completo de exemplo

---

## 🔧 Dependências

### Obrigatórias (easyEnex)
- **YSI 5.x**
  - `YSI_Data\y_iterate`
  - `YSI_Coding\y_hooks`
- **Streamer Plugin** (SA-MP)

### Adicionais (easyHouses)
- **YSI_Extra\y_timers_foreach** (YSI 5.x)
- **DOF2** *(opcional - salvamento automático)*
- **ZCMD** *(opcional - comandos de exemplo)*
- **sscanf2** *(opcional - comando de transferência)*

### Instalação de Dependências

```bash
# Via sampctl
sampctl package ensure

# Manual
# Baixe: github.com/pawn-lang/YSI-Includes
# Baixe: github.com/samp-incognito/samp-streamer-plugin
# Baixe: github.com/ziggi/FCNPC (para DOF2)
```

---

## 💡 Exemplos

### Exemplo 1: Sistema Simples de TP

```pawn
#include <easyEnex>

public OnGameModeInit()
{
    // TP para LS
    Enex_Create("TP Los Santos", 
        1529.6, -1691.2, 13.3, 0.0,  // Entrada SF
        1642.9, -2335.5, 13.5, 0.0,  // Saída LS
        0, 0, false);
    
    // TP para LV
    Enex_Create("TP Las Venturas",
        1529.6, -1695.0, 13.3, 0.0,  // Entrada SF
        2503.0, 2763.5, 10.8, 0.0,   // Saída LV
        0, 0, false);
    
    return 1;
}
```

### Exemplo 2: Casa com Controle de Acesso

```pawn
new minhaCasa;

public OnGameModeInit()
{
    minhaCasa = Enex_Create("Minha Casa VIP",
        2495.0, -1688.0, 13.5, 0.0,
        2496.0, -1695.0, 1014.7, 0.0,
        0, 3, true);
    
    Enex_Close(minhaCasa); // Começa fechada
    return 1;
}

CMD:abrircasa(playerid)
{
    if(PlayerInfo[playerid][pVIP] > 0)
    {
        Enex_OpenForPlayer(playerid, minhaCasa);
        SendClientMessage(playerid, -1, "Casa VIP aberta!");
    }
    return 1;
}
```

### Exemplo 3: Sistema Completo de Casas

Veja o arquivo completo: [extras/demo_houses.pwn](extras/demo_houses.pwn)

---

## 🧪 Testando

### Com sampctl

```bash
# Compilar
sampctl package build

# Executar
sampctl package run
```

### Manual

```bash
# Compilar com pawncc
pawncc -;+ -\(+ -Z+ seu_gamemode.pwn

# Executar servidor
./samp-server.exe  # Windows
./samp03svr        # Linux
```

---

## 🤝 Contribuindo

Contribuições são bem-vindas! 

1. Fork o projeto
2. Crie sua feature branch (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Add: Minha nova feature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

### Diretrizes
- Mantenha a compatibilidade com versões anteriores
- Documente novas funcionalidades
- Teste antes de submeter
- Siga o estilo de código existente

---

## 📄 Licença

Este projeto está sob a licença especificada no arquivo [LICENSE](LICENSE).

---

## 🙏 Agradecimentos

- **Y_Less** - YSI Library
- **Incognito** - Streamer Plugin
- **Double-O-Seven** - DOF2
- Comunidade SA-MP Brasil

---

## 📞 Suporte

- **Issues:** [GitHub Issues](https://github.com/xbruno1000x/easyEnEx/issues)

---

## 📊 Status do Projeto

![GitHub release](https://img.shields.io/badge/release-v1.8.0-blue)
![GitHub last commit](https://img.shields.io/badge/last%20commit-october%202025-green)
![Status](https://img.shields.io/badge/status-active-success)

---

**Desenvolvido com ❤️ para a comunidade SA-MP**
