# 📚 Funcionalidades - easyHouses

**Versão:** 1.2.0  
**Sistema completo de casas para SA-MP**

---

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Compra e Venda](#compra-e-venda)
3. [Sistema de Aluguel](#sistema-de-aluguel)
4. [Transferência de Propriedade](#transferência-de-propriedade)
5. [Múltiplas Casas](#múltiplas-casas)
6. [Virtual Worlds Únicos](#virtual-worlds-únicos)
7. [Salvamento Automático](#salvamento-automático)
8. [Funções API](#funções-api)
9. [Callbacks](#callbacks)
10. [Dados Salvos](#dados-salvos)

---

## 🎯 Visão Geral

O **easyHouses** é um sistema modular completo de gerenciamento de casas para SA-MP, construído sobre o easyEnex.

### Características Principais

✅ **Compra e Venda**
- Jogadores podem comprar casas disponíveis
- Venda retorna 70% do valor
- Limite configurável de casas por jogador

✅ **Sistema de Aluguel**
- Proprietários podem alugar casas
- Pagamento automático por dia
- Despejo por falta de pagamento
- Acumulação de aluguel para o dono

✅ **Transferência**
- Transferir propriedade entre jogadores
- Verificação de limites
- Cancelamento automático de aluguel

✅ **Salvamento**
- Automático com DOF2
- Customizável (MySQL, SQLite)
- Salva todos os dados (owner, locked, rent, etc)

✅ **Visual**
- Pickups dinâmicos (venda/aluguel)
- Labels 3D com informações
- Cores diferentes por status

---

## 🏠 Compra e Venda

### Compra de Casa

```pawn
House_Buy(playerid, houseid);
```

**Requisitos:**
- Casa não pode estar ocupada
- Jogador deve ter dinheiro suficiente
- Jogador não pode ter atingido o limite de casas

**Processo:**
1. Verifica requisitos
2. Cobra o preço da casa
3. Adiciona ao array `PlayerHouses[playerid][]`
4. Atualiza `houseOwner` com nome do jogador
5. Abre enex para o dono (`Enex_OpenForPlayer`)
6. Atualiza label (verde → vermelho)
7. Salva automaticamente

**Exemplo:**
```pawn
CMD:comprarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "Não há casa por perto!");
    
    House_Buy(playerid, houseid);
    return 1;
}
```

### Venda de Casa

```pawn
House_Sell(playerid, houseid);
```

**Características:**
- Retorna 70% do valor da casa
- Remove do array do jogador
- Reseta armas e dinheiro guardados
- Casa volta para status "À venda"

**Processo:**
1. Verifica se é dono
2. Calcula 70% do preço (`sellPrice = price * 0.7`)
3. Dá dinheiro ao jogador
4. Remove do `PlayerHouses[playerid][]`
5. Reseta `houseOwner` e `houseOwned`
6. Limpa armas guardadas
7. Fecha enex (`Enex_CloseForPlayer`)
8. Atualiza label
9. Salva

---

## 🔑 Sistema de Aluguel

### Visão Geral

Permite que **proprietários** aluguem suas casas para outros jogadores. O inquilino paga **por dia** (tempo real) e o dono recebe o pagamento acumulado.

### Alugar Casa

```pawn
House_Rent(playerid, houseid);
```

**Requisitos:**
- Casa deve ter dono (`houseOwned = true`)
- Casa deve ter `rentPrice > 0`
- Jogador não pode ser o dono
- Casa não pode estar já alugada
- Jogador não pode já estar alugando outra
- Jogador deve ter dinheiro para primeiro pagamento

**O que acontece:**
```pawn
// 1. Cobra primeiro dia
GivePlayerMoney(playerid, -houseRentPrice);

// 2. Configura aluguel
houseRenter = "NomeJogador"
houseRented = true
houseRentDay = gettime()  // Timestamp atual
houseRentDaysOwed = 1     // Primeiro dia pago

// 3. Dá acesso
Enex_OpenForPlayer(playerid, houseEnexID);

// 4. Atualiza label
"Dono: xBruno1000x"
"Inquilino: Player123"  ← NOVO!
```

### Pagamento Automático

Quando o inquilino **conecta** no servidor:

```pawn
// 1. Calcula dias passados
dias = (gettime() - houseRentDay) / 86400

// 2. Calcula valor devido
valorDevido = dias * houseRentPrice

// 3. Verifica dinheiro
if(GetPlayerMoney(playerid) >= valorDevido)
{
    // Paga
    GivePlayerMoney(playerid, -valorDevido);
    houseRentDay = gettime();
    houseRentDaysOwed += dias;
    
    // Mensagem
    "Aluguel pago: $X (Y dias)"
}
else
{
    // Despejado!
    houseRented = false;
    houseRenter = "";
    PlayerRentHouseID[playerid] = -1;
    Enex_CloseForPlayer(playerid);
    
    // Mensagem
    "Você foi despejado por falta de pagamento!"
}
```

### Coletar Aluguel (Dono)

```pawn
House_CollectRent(playerid, houseid);
```

**Exemplo de Fluxo:**
```
DIA 1: Jogador B aluga por $500/dia
  → Paga $500 (primeiro dia)
  → houseRentDaysOwed = 1

DIA 4: B desconecta
  → houseRentDaysOwed = 1 (dono pode coletar $500)

DIA 7: B conecta (3 dias depois)
  → Cobra: 3 × $500 = $1500
  → Se tem: paga e houseRentDaysOwed = 4
  → Se não tem: despejado

Dono coleta:
  → /coletaraluguel
  → Recebe: 4 × $500 = $2000
  → houseRentDaysOwed = 0
```

### Cancelar Aluguel

**Inquilino:**
```pawn
House_Unrent(playerid, houseid);
```
- Pode cancelar a qualquer momento
- Perde acesso imediato
- Dias devidos ficam para o dono

**Dono (Despejar):**
```pawn
House_EvictRenter(playerid, houseid);
```
- Pode despejar a qualquer momento
- Inquilino perde acesso
- Notifica se inquilino estiver online

---

## 🔄 Transferência de Propriedade

### Transferir Casa

```pawn
House_Transfer(playerid, targetid, houseid);
```

**Processo Completo:**
```pawn
// 1. Verificações
- É dono da casa?
- Jogador alvo está online?
- Não está transferindo para si mesmo?
- Alvo tem espaço para mais casas?

// 2. Remove do dono atual
PlayerHouses[playerid][slot] = -1
PlayerHousesCount[playerid]--

// 3. Adiciona ao novo dono
PlayerHouses[targetid][slot] = houseid
PlayerHousesCount[targetid]++

// 4. Atualiza dados
houseOwner = "NovoNome"

// 5. Cancela aluguel (se houver)
if(houseRented)
{
    houseRented = false
    houseRenter = ""
    houseRentDay = 0
    houseRentDaysOwed = 0
}

// 6. Ajusta permissões
Enex_CloseForPlayer(playerid)
Enex_OpenForPlayer(targetid)

// 7. Salva
House_Save(houseid)
```

**Uso Prático - Venda entre Jogadores:**
```pawn
CMD:venderpara(playerid, params[])
{
    new targetid, price;
    if(sscanf(params, "ud", targetid, price))
        return SendClientMessage(playerid, -1, "Uso: /venderpara [id] [preço]");
    
    // Verificar dinheiro
    if(GetPlayerMoney(targetid) < price)
        return SendClientMessage(playerid, -1, "Jogador sem dinheiro!");
    
    // Fazer transação
    GivePlayerMoney(targetid, -price);
    GivePlayerMoney(playerid, price);
    
    // Transferir casa
    new houseid = House_GetNearby(playerid);
    House_Transfer(playerid, targetid, houseid);
    
    new string[128];
    format(string, sizeof(string), "Casa vendida por $%d!", price);
    SendClientMessage(playerid, -1, string);
    return 1;
}
```

---

## 🏘️ Múltiplas Casas

### Sistema de Múltiplas Casas

O jogador pode ter até `MAX_HOUSES_PER_PLAYER` casas (padrão: 3).

**Armazenamento:**
```pawn
PlayerHouses[MAX_PLAYERS][MAX_HOUSES_PER_PLAYER]
PlayerHousesCount[MAX_PLAYERS]

// Exemplo:
PlayerHouses[0] = {5, 12, 23}  // Jogador 0 tem casas 5, 12 e 23
PlayerHousesCount[0] = 3        // Total: 3 casas
```

### Funções Úteis

```pawn
// Quantas casas o jogador tem?
new count = House_GetPlayerHouseCount(playerid);

// Pegar casa por slot
new houseid = House_GetPlayerHouseBySlot(playerid, 0); // Primeira casa
new houseid = House_GetPlayerHouseBySlot(playerid, 1); // Segunda casa

// Verificar se possui uma casa específica
if(House_PlayerOwnsHouse(playerid, houseid))
{
    // É dono!
}
```

### Exemplo: Listar Casas

```pawn
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
        if(houseid != -1)
        {
            format(string, sizeof(string), 
                "%d. %s | ID: %d | Aluguel: $%d/dia",
                i + 1,
                HouseData[houseid][houseName],
                houseid,
                HouseData[houseid][houseRentPrice]
            );
            SendClientMessage(playerid, -1, string);
        }
    }
    return 1;
}
```

---

## 🌍 Virtual Worlds Únicos

### Sistema de VW

Cada casa recebe um **Virtual World único** para evitar conflitos entre interiores.

**Cálculo:**
```pawn
VirtualWorld = houseid + 1000

// Exemplos:
Casa 0  → VW 1000
Casa 1  → VW 1001
Casa 50 → VW 1050
```

### Vantagens

✅ **Múltiplas casas podem usar o mesmo interior**
```pawn
// 3 casas diferentes, mesmo interior (Interior 3)
House_Create("Casa 1", ..., interior: 3); // VW 1000
House_Create("Casa 2", ..., interior: 3); // VW 1001
House_Create("Casa 3", ..., interior: 3); // VW 1002

// Cada uma isolada no seu VW!
```

✅ **Sem conflito de saída**
- Sistema rastreia qual enex o jogador entrou (`PlayerLastEnex`)
- Ao sair, usa o enex correto (mesmo com coordenadas iguais)

✅ **Privacidade**
- Jogadores em casas diferentes não se veem
- Mesmo que as casas usem o mesmo interior

---

## 💾 Salvamento Automático

### Com DOF2

Salvamento **totalmente automático** em:

| Evento | O que salva |
|--------|-------------|
| `House_Buy()` | Casa comprada |
| `House_Sell()` | Casa vendida |
| `House_ToggleLock()` | Status de trancado |
| `House_Rent()` | Dados de aluguel |
| `House_Transfer()` | Novo dono |
| `OnPlayerDisconnect` | Casas do jogador |
| `OnGameModeExit` | Todas as casas |

### Dados Salvos (Arquivo .ini)

```ini
; Informações básicas
Name=Casa dos Johnson
Owner=xBruno1000x
Price=50000
RentPrice=500
Interior=3
Owned=1
Locked=0
Money=1500

; Posição
PosX=2495.300048
PosY=-1690.300048
PosZ=14.800000
VirtualWorld=1000

; Aluguel
Renter=Player123
Rented=1
RentDay=1730000000
RentDaysOwed=5

; Armas guardadas
Weapon0=24
Ammo0=50
Weapon1=31
Ammo1=200
...
Weapon9=0
Ammo9=0
```

---

## 🛠️ Funções API

### Gerenciamento Básico

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_Create(...)` | Cria uma nova casa | ID da casa ou -1 |
| `House_Delete(houseid)` | Deleta uma casa | 1 = sucesso |
| `House_GetNearby(playerid, Float:range)` | Casa mais próxima | ID ou -1 |
| `House_IsOwner(playerid, houseid)` | Verifica se é dono | 1 = sim, 0 = não |
| `House_UpdateLabel(houseid)` | Atualiza label 3D | 1 = sucesso |

### Compra e Venda

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_Buy(playerid, houseid)` | Compra uma casa | 1 = sucesso |
| `House_Sell(playerid, houseid)` | Vende uma casa | 1 = sucesso |
| `House_ToggleLock(playerid, houseid)` | Tranca/destranca | 1 = sucesso |

### Aluguel

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_Rent(playerid, houseid)` | Aluga uma casa | 1 = sucesso |
| `House_Unrent(playerid, houseid)` | Cancela aluguel | 1 = sucesso |
| `House_EvictRenter(playerid, houseid)` | Despeja inquilino | 1 = sucesso |
| `House_CollectRent(playerid, houseid)` | Coleta aluguel | 1 = sucesso |
| `House_ProcessRentPayment(playerid, houseid)` | Processa pagamento | 1 = pago, 0 = despejado |

### Transferência

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_Transfer(playerid, targetid, houseid)` | Transfere propriedade | 1 = sucesso |

### Múltiplas Casas

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_GetPlayerHouseCount(playerid)` | Quantidade de casas | Número (0-MAX) |
| `House_GetPlayerHouseBySlot(playerid, slot)` | ID da casa no slot | ID ou -1 |
| `House_PlayerOwnsHouse(playerid, houseid)` | Verifica posse | 1 = possui |

### Salvamento

| Função | Descrição | Retorno |
|--------|-----------|---------|
| `House_Save(houseid)` | Salva uma casa | 1 = sucesso |
| `House_Load(houseid)` | Carrega uma casa | 1 = sucesso |
| `House_SaveAll()` | Salva todas | Quantidade |
| `House_LoadAll()` | Carrega todas | Quantidade |
| `House_LoadPlayerHouses(playerid)` | Carrega casas do jogador | Quantidade |

---

## 📡 Callbacks

### Compra/Venda

```pawn
public OnPlayerBuyHouse(playerid, houseid)
{
    // Jogador comprou uma casa
    return 1;
}

public OnPlayerSellHouse(playerid, houseid)
{
    // Jogador vendeu uma casa
    return 1;
}
```

### Aluguel

```pawn
public OnPlayerRentHouse(playerid, houseid)
{
    // Jogador alugou uma casa
    return 1;
}

public OnPlayerUnrentHouse(playerid, houseid)
{
    // Jogador cancelou aluguel
    return 1;
}
```

### Transferência

```pawn
public OnPlayerTransferHouse(playerid, targetid, houseid)
{
    // Casa foi transferida
    // playerid = dono antigo
    // targetid = novo dono
    return 1;
}
```

### Entrada/Saída

```pawn
public OnPlayerEnterHouse(playerid, houseid)
{
    // Jogador entrou na casa
    return 1;
}

public OnPlayerExitHouse(playerid, houseid)
{
    // Jogador saiu da casa
    return 1;
}
```

### Salvamento

```pawn
public OnHouseSave(houseid)
{
    // Use para salvamento customizado (MySQL, etc)
    // Se implementar, salvamento DOF2 não é usado
    return 1;
}

public OnHouseLoad(houseid)
{
    // Use para carregamento customizado
    return 1;
}
```

### Criação/Deleção

```pawn
public OnHouseCreated(houseid)
{
    // Casa criada
    return 1;
}

public OnHouseDeleted(houseid)
{
    // Casa deletada
    return 1;
}
```

---

## 📊 Dados Salvos

### Estrutura do Enum

```pawn
enum E_HOUSE_DATA {
    houseEnexID,              // ID do enex associado
    housePrice,               // Preço de compra
    houseRentPrice,           // Preço de aluguel/dia
    houseInterior,            // ID do interior
    houseName[32],            // Nome da casa
    houseOwner[MAX_PLAYER_NAME], // Nome do dono
    bool:houseOwned,          // Se está ocupada
    bool:houseLocked,         // Se está trancada
    Text3D:houseLabel,        // Label 3D
    Float:houseX,             // Posição X entrada
    Float:houseY,             // Posição Y entrada
    Float:houseZ,             // Posição Z entrada
    houseVirtualWorld,        // VW único
    houseWeapons[10],         // Armas guardadas
    houseAmmo[10],            // Munição
    houseMoney,               // Dinheiro guardado
    
    // Aluguel
    houseRenter[MAX_PLAYER_NAME], // Nome inquilino
    bool:houseRented,         // Se está alugada
    houseRentDay,             // Timestamp último pagamento
    houseRentDaysOwed         // Dias devidos ao dono
}
```

### Variáveis Globais

```pawn
HouseData[MAX_HOUSES][E_HOUSE_DATA]
Iterator:HouseIterator<MAX_HOUSES>
PlayerHouses[MAX_PLAYERS][MAX_HOUSES_PER_PLAYER]
PlayerHousesCount[MAX_PLAYERS]
PlayerRentHouseID[MAX_PLAYERS]
```

---

## 🎨 Customização Visual

### Pickups

```pawn
HOUSE_PICKUP_ID         1273  // Ícone casa à venda (verde)
HOUSE_RENT_PICKUP_ID    1272  // Ícone casa aluguel (azul)
```

**Sistema automático:**
- Casa com `rentPrice > 0` → Pickup azul (aluguel)
- Casa sem aluguel → Pickup verde (venda)

### Labels 3D

**Cores:**
```pawn
HOUSE_LABEL_COLOR_SALE  0x00FF00FF  // Verde (à venda)
HOUSE_LABEL_COLOR_OWNED 0xFF0000FF  // Vermelho (ocupada)
HOUSE_LABEL_COLOR_RENT  0xFFFF00FF  // Amarelo (aluguel)
```

**Textos:**
```
Casa À Venda:
  Casa: Nome da Casa
  Preço: $50000
  Digite /comprarcasa

Casa Para Alugar:
  Casa: Nome da Casa
  Preço: $50000
  Aluguel: $500/dia
  Digite /comprarcasa ou /alugarcasa

Casa Ocupada:
  Casa: Nome da Casa
  Dono: xBruno1000x
  Status: Aberta/Trancada

Casa Alugada:
  Casa: Nome da Casa
  Dono: xBruno1000x
  Inquilino: Player123
  Status: Aberta/Trancada
```

---

## ✅ Resumo de Funcionalidades

### ✨ Recursos Principais

- [x] Compra e venda de casas
- [x] Sistema de aluguel com pagamento por dia
- [x] Transferência de propriedade
- [x] Múltiplas casas por jogador (configurável)
- [x] Virtual Worlds únicos (sem conflitos)
- [x] Salvamento automático (DOF2)
- [x] Pickups e labels dinâmicos
- [x] Trancamento de portas
- [x] Armazenamento de armas e dinheiro
- [x] 11 callbacks customizáveis
- [x] 20+ funções API
- [x] Despejo automático (falta de pagamento)
- [x] Acumulação de aluguel para dono
- [x] Limite de casas por jogador
- [x] Compatível com sistemas de economia customizados

---

**Versão:** 1.2.0  
**Autor:** xBruno1000x  
**Documentação Completa**
