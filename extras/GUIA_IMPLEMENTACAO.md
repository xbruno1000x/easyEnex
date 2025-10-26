# 📘 Guia de Implementação - easyHouses

**Versão:** 1.2.0  
**Sistema completo de casas para SA-MP**

---

## 📋 Índice

1. [Requisitos](#requisitos)
2. [Instalação Rápida](#instalação-rápida)
3. [Configuração Básica](#configuração-básica)
4. [Criar Casas](#criar-casas)
5. [Sistema de Salvamento](#sistema-de-salvamento)
6. [Comandos](#comandos)
7. [Customização](#customização)
8. [Callbacks Opcionais](#callbacks-opcionais)
9. [Troubleshooting](#troubleshooting)

---

## 🔧 Requisitos

### Obrigatórios
- ✅ **YSI 5.x** (y_iterate, y_hooks, y_timers)
- ✅ **Streamer Plugin** (CreateDynamicPickup, Create3DTextLabel)
- ✅ **easyEnex.inc** (biblioteca principal)

### Opcionais
- 📦 **DOF2.inc** - Para salvamento automático (recomendado)
- 📦 **ZCMD** - Para comandos de exemplo
- 📦 **sscanf2** - Para comando de transferência

---

## ⚡ Instalação Rápida

### Passo 1: Incluir no Gamemode

```pawn
#include <a_samp>
#include <YSI_Data\y_iterate>
#include <streamer>
#include <YSI_Coding\y_hooks>
#include <DOF2>              // Opcional, mas recomendado
#include <easyEnex>          // Obrigatório
#include <extras/easyHouses> // Sistema de casas
```

### Passo 2: Criar Pasta de Dados

Crie a pasta para salvar os dados das casas:
```
samp-server/
└── scriptfiles/
    └── houses/   ← Criar esta pasta
```

### Passo 3: Criar Casas no OnGameModeInit

```pawn
public OnGameModeInit()
{
    // Criar casas
    House_Create(
        "Casa da Grove Street",  // Nome
        50000,                   // Preço de compra
        500,                     // Aluguel por dia (0 = desabilitado)
        2495.3, -1690.3, 14.8, 0.0,      // Entrada (exterior)
        2496.05, -1695.17, 1014.74, 180.0, // Interior
        3                        // ID do interior
    );

    // Carregar dados salvos
    House_LoadAll();
    
    return 1;
}
```

### Passo 4: Pronto!

O sistema já está funcionando com salvamento automático. Crie comandos para os jogadores interagirem.

---

## 🏗️ Configuração Básica

### Definir Limites (Opcional)

Antes de incluir `easyHouses.inc`:

```pawn
#define MAX_HOUSES              500  // Máximo de casas no servidor
#define MAX_HOUSE_NAME          32   // Tamanho do nome da casa
#define MAX_HOUSES_PER_PLAYER   3    // Máximo de casas por jogador
#define HOUSE_FILE_PATH         "houses/%d.ini" // Caminho dos arquivos

#include <extras/easyHouses>
```

### Customizar Pickups e Cores

```pawn
#define HOUSE_PICKUP_ID         1273  // Ícone de casa à venda
#define HOUSE_RENT_PICKUP_ID    1272  // Ícone de casa para alugar

#define HOUSE_LABEL_COLOR_SALE  0x00FF00FF // Verde (à venda)
#define HOUSE_LABEL_COLOR_OWNED 0xFF0000FF // Vermelho (ocupada)
#define HOUSE_LABEL_COLOR_RENT  0xFFFF00FF // Amarelo (aluguel)

#include <extras/easyHouses>
```

---

## 🏠 Criar Casas

### Sintaxe Completa

```pawn
new houseid = House_Create(
    const name[],                    // Nome da casa
    price,                           // Preço de compra
    rentPrice,                       // Aluguel/dia (0 = desabilitado)
    Float:entX, Float:entY, Float:entZ, Float:entAng,  // Entrada
    Float:intX, Float:intY, Float:intZ, Float:intAng,  // Interior
    interiorID,                      // ID do interior
    entranceInteriorID = 0           // Interior da entrada (0 = exterior)
);
```

### Exemplo: Casa CJ

```pawn
new cjhouse = House_Create(
    "Casa dos Johnson",
    50000,    // Preço
    500,      // $500 por dia de aluguel
    2495.6523, -1688.0239, 13.7656, 0.0,      // Grove Street
    2496.0493, -1695.1674, 1014.7422, 180.0,  // Interior CJ
    3         // Interior ID 3
);
```

### Exemplo: Casa em Interior

```pawn
// Casa dentro de um shopping (interior 1)
new shopHouse = House_Create(
    "Loja no Shopping",
    100000,
    0,        // Sem aluguel
    1000.0, 2000.0, 500.0, 90.0,  // Entrada no shopping
    500.0, 1500.0, 300.0, 0.0,    // Interior da loja
    10,       // Interior ID da loja
    1         // Entrada está no interior 1 (shopping)
);
```

### Exemplo: Múltiplas Casas

```pawn
public OnGameModeInit()
{
    // Casa 1 - Barata com aluguel
    House_Create("Casa Simples", 25000, 300,
        2486.37, -1644.88, 14.07, 180.91,
        512.92, -11.69, 1001.56, 180.0, 3);
    
    // Casa 2 - Média sem aluguel
    House_Create("Casa Confortável", 75000, 0,
        2523.03, -1679.25, 15.49, 87.19,
        2496.05, -1695.17, 1014.74, 180.0, 3);
    
    // Casa 3 - Mansão VIP com aluguel alto
    House_Create("Mansão VIP", 500000, 5000,
        2495.3, -1690.3, 14.8, 0.0,
        2324.33, -1149.54, 1050.71, 0.0, 12);
    
    House_LoadAll();
    return 1;
}
```

---

## 💾 Sistema de Salvamento

### Com DOF2 (Automático)

Se DOF2 estiver incluído, o salvamento é **totalmente automático**:

```pawn
#include <DOF2>
#include <easyEnex>
#include <extras/easyHouses>

// Pronto! Sistema salva automaticamente em:
// - Compra/venda de casa
// - Trancar/destrancar
// - Aluguel/transferência
// - Desconectar do servidor
// - Fechar servidor
```

### Sem DOF2 (Manual)

Implemente os callbacks:

```pawn
public OnHouseSave(houseid)
{
    // Seu sistema de salvamento (MySQL, SQLite, etc)
    new query[256];
    mysql_format(conexao, query, sizeof(query),
        "UPDATE houses SET owner='%s', locked=%d WHERE id=%d",
        HouseData[houseid][houseOwner],
        HouseData[houseid][houseLocked],
        houseid
    );
    mysql_tquery(conexao, query);
    return 1;
}

public OnHouseLoad(houseid)
{
    // Seu sistema de carregamento
    mysql_format(conexao, query, sizeof(query),
        "SELECT * FROM houses WHERE id=%d", houseid);
    mysql_tquery(conexao, query, "LoadHouseCallback", "d", houseid);
    return 1;
}
```

### Funções de Salvamento

```pawn
House_Save(houseid);      // Salva uma casa
House_Load(houseid);      // Carrega uma casa
House_SaveAll();          // Salva todas as casas
House_LoadAll();          // Carrega todas as casas
```

---

## 🎮 Comandos

### Comandos Básicos

```pawn
#include <zcmd>

CMD:comprarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "Você não está perto de uma casa!");
    
    House_Buy(playerid, houseid);
    return 1;
}

CMD:vendercasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1 || !House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "Esta não é sua casa!");
    
    House_Sell(playerid, houseid);
    return 1;
}

CMD:trancarcasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1 || !House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "Esta não é sua casa!");
    
    House_ToggleLock(playerid, houseid);
    return 1;
}

CMD:minhascasas(playerid)
{
    new count = House_GetPlayerHouseCount(playerid);
    if(count == 0)
        return SendClientMessage(playerid, -1, "Você não possui casas!");
    
    new string[128];
    format(string, sizeof(string), "=== Suas Casas (%d/%d) ===", 
        count, MAX_HOUSES_PER_PLAYER);
    SendClientMessage(playerid, -1, string);
    
    for(new i = 0; i < count; i++)
    {
        new houseid = House_GetPlayerHouseBySlot(playerid, i);
        // Mostrar informações da casa
    }
    return 1;
}
```

### Comandos de Aluguel

```pawn
CMD:alugarcasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "Você não está perto de uma casa!");
    
    House_Rent(playerid, houseid);
    return 1;
}

CMD:desalugar(playerid)
{
    if(PlayerRentHouseID[playerid] == -1)
        return SendClientMessage(playerid, -1, "Você não está alugando nada!");
    
    House_Unrent(playerid, PlayerRentHouseID[playerid]);
    return 1;
}

CMD:despejar(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1 || !House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "Esta não é sua casa!");
    
    House_EvictRenter(playerid, houseid);
    return 1;
}

CMD:coletaraluguel(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1 || !House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "Esta não é sua casa!");
    
    House_CollectRent(playerid, houseid);
    return 1;
}
```

### Comando de Transferência

```pawn
#include <sscanf2>

CMD:transferircasa(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, -1, "Uso: /transferircasa [id]");
    
    new houseid = House_GetNearby(playerid);
    if(houseid == -1 || !House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "Esta não é sua casa!");
    
    House_Transfer(playerid, targetid, houseid);
    return 1;
}
```

---

## 🎨 Customização

### Customizar Funções de Dinheiro

Por padrão usa `GetPlayerMoney` e `GivePlayerMoney`. Para usar sistema próprio:

```pawn
// Definir ANTES de incluir easyHouses
#define Enex_GetPlayerMoney(%0)     MinhaFuncaoGetMoney(%0)
#define Enex_GivePlayerMoney(%0,%1) MinhaFuncaoDarDinheiro(%0,%1)

#include <extras/easyHouses>
```

### Exemplo com Sistema de Economia

```pawn
// Seu sistema
stock GetMoney(playerid) {
    return PlayerInfo[playerid][pDinheiro];
}

stock GiveMoney(playerid, amount) {
    PlayerInfo[playerid][pDinheiro] += amount;
    return 1;
}

// Definir para easyHouses
#define Enex_GetPlayerMoney(%0)     GetMoney(%0)
#define Enex_GivePlayerMoney(%0,%1) GiveMoney(%0,%1)

#include <extras/easyHouses>
```

---

## 📡 Callbacks Opcionais

Implemente para adicionar lógica customizada:

```pawn
public OnHouseCreated(houseid)
{
    printf("[Sistema] Casa ID %d criada!", houseid);
    return 1;
}

public OnPlayerBuyHouse(playerid, houseid)
{
    // Anunciar para todos
    new string[128], name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    format(string, sizeof(string), "%s comprou uma casa!", name);
    SendClientMessageToAll(-1, string);
    return 1;
}

public OnPlayerSellHouse(playerid, houseid)
{
    // Dar bônus
    GivePlayerMoney(playerid, 5000);
    SendClientMessage(playerid, -1, "Bônus: +$5000 pela venda!");
    return 1;
}

public OnPlayerRentHouse(playerid, houseid)
{
    // Log de aluguel
    printf("Jogador %d alugou casa %d", playerid, houseid);
    return 1;
}

public OnPlayerTransferHouse(playerid, targetid, houseid)
{
    // Taxar transferência
    GivePlayerMoney(playerid, -10000);
    SendClientMessage(playerid, -1, "Taxa de transferência: $10000");
    return 1;
}
```

### Callbacks Disponíveis

- `OnHouseCreated(houseid)`
- `OnHouseDeleted(houseid)`
- `OnPlayerBuyHouse(playerid, houseid)`
- `OnPlayerSellHouse(playerid, houseid)`
- `OnPlayerEnterHouse(playerid, houseid)`
- `OnPlayerExitHouse(playerid, houseid)`
- `OnPlayerRentHouse(playerid, houseid)`
- `OnPlayerUnrentHouse(playerid, houseid)`
- `OnPlayerTransferHouse(playerid, targetid, houseid)`
- `OnHouseSave(houseid)`
- `OnHouseLoad(houseid)`

---

## 🐛 Troubleshooting

### Erro: "easyHouses requer easyEnex.inc"

**Solução:** Incluir easyEnex.inc antes de easyHouses.inc

```pawn
#include <easyEnex>
#include <extras/easyHouses> // Deve vir depois
```

### Casas não salvam

**Causas:**
1. Pasta `scriptfiles/houses/` não existe
2. DOF2 não incluído e callbacks não implementados

**Solução:**
```pawn
// 1. Criar pasta houses/
// 2. Incluir DOF2
#include <DOF2>

// OU implementar callbacks
public OnHouseSave(houseid) { ... }
public OnHouseLoad(houseid) { ... }
```

### Dados não carregam ao iniciar

**Solução:** Adicionar `House_LoadAll()` no OnGameModeInit

```pawn
public OnGameModeInit()
{
    // ... criar casas ...
    
    House_LoadAll(); // ← Adicionar esta linha
    return 1;
}
```

### Jogador não consegue entrar na casa

**Verificar:**
1. Casa está trancada? (`House_ToggleLock`)
2. Jogador é dono? (`House_PlayerOwnsHouse`)
3. Casa está alugada para outro jogador?

### Aluguel não cobra automaticamente

**Verificar:**
1. Casa tem `rentPrice > 0`?
2. Sistema DOF2 salvando `RentDay`?
3. Jogador realmente ficou dias offline?

**Debug:**
```pawn
public OnPlayerConnect(playerid)
{
    // Ver timestamp
    printf("Tempo atual: %d", gettime());
    printf("Último pagamento: %d", HouseData[houseid][houseRentDay]);
    return 1;
}
```

### Virtual World conflicts

**Já corrigido na versão 1.2.0!**

O sistema usa `PlayerLastEnex[playerid]` para rastrear qual enex o jogador entrou, evitando conflitos de VW.

---

## 📊 Exemplo Completo

```pawn
#include <a_samp>
#include <YSI_Data\y_iterate>
#include <streamer>
#include <YSI_Coding\y_hooks>
#include <DOF2>
#include <zcmd>
#include <sscanf2>

// Customização (opcional)
#define MAX_HOUSES_PER_PLAYER 5

#include <easyEnex>
#include <extras/easyHouses>

main() {}

public OnGameModeInit()
{
    SetGameModeText("Casas System");
    
    // Criar 3 casas
    House_Create("Casa 1", 50000, 500,
        2495.6, -1688.0, 13.7, 0.0,
        2496.0, -1695.1, 1014.7, 180.0, 3);
    
    House_Create("Casa 2", 75000, 0,
        2523.0, -1679.2, 15.4, 87.1,
        2496.0, -1695.1, 1014.7, 180.0, 3);
    
    House_Create("Casa 3", 25000, 300,
        2486.3, -1644.8, 14.0, 180.9,
        512.9, -11.6, 1001.5, 180.0, 3);
    
    // Carregar dados
    House_LoadAll();
    
    return 1;
}

public OnPlayerSpawn(playerid)
{
    SetPlayerPos(playerid, 2495.0, -1684.0, 13.5);
    GivePlayerMoney(playerid, 100000);
    return 1;
}

// Comandos
CMD:comprar(playerid)
{
    new h = House_GetNearby(playerid);
    if(h != -1) House_Buy(playerid, h);
    return 1;
}

CMD:vender(playerid)
{
    new h = House_GetNearby(playerid);
    if(h != -1) House_Sell(playerid, h);
    return 1;
}

CMD:trancar(playerid)
{
    new h = House_GetNearby(playerid);
    if(h != -1) House_ToggleLock(playerid, h);
    return 1;
}

CMD:alugar(playerid)
{
    new h = House_GetNearby(playerid);
    if(h != -1) House_Rent(playerid, h);
    return 1;
}

CMD:transferir(playerid, params[])
{
    new id;
    if(sscanf(params, "u", id)) return 1;
    new h = House_GetNearby(playerid);
    if(h != -1) House_Transfer(playerid, id, h);
    return 1;
}
```

---

## ✅ Checklist de Implementação

- [ ] YSI, Streamer, easyEnex incluídos
- [ ] DOF2 incluído (opcional)
- [ ] Pasta `scriptfiles/houses/` criada
- [ ] easyHouses incluído no gamemode
- [ ] Casas criadas no OnGameModeInit
- [ ] `House_LoadAll()` chamado
- [ ] Comandos implementados
- [ ] Testado: comprar, vender, trancar
- [ ] Testado: aluguel e transferência
- [ ] Sistema de dinheiro funcionando
- [ ] Salvamento funcionando

---

**Versão:** 1.2.0  
**Autor:** xBruno1000x  
**Documentação Completa**
