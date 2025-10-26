/*
 * easyHouses.inc - Sistema de casas para SA-MP
 * Módulo opcional para easyEnex
 * 
 * Descrição: Sistema completo de gerenciamento de casas com compra, venda,
 *            aluguel, spawn e customização de interiores.
 * 
 * Dependências: easyEnex.inc (obrigatório), DOF2.inc (opcional para salvamento)
 * 
 * Autor: xBruno1000x
 * Versão: 1.3.0
 */

#include <YSI_Coding\y_hooks>

// Incluir DOF2 se disponível (para salvamento padrão)
#if !defined _dof2_included
    #tryinclude <DOF2>
#endif

// DOF2 detection for automatic save/load
// Se DOF2 não estiver incluído, implemente OnHouseSave/OnHouseLoad para persistência customizada

#if defined _easyHouses_included
    #endinput
#endif
#define _easyHouses_included

// Verificar se easyEnex foi incluído
#if !defined MAX_ENEX
    #error "easyHouses.inc requer easyEnex.inc - inclua easyEnex.inc antes de easyHouses.inc"
#endif

// ============================================================================
// CONFIGURAÇÕES
// ============================================================================

#if !defined MAX_HOUSES
    #define MAX_HOUSES              (500)
#endif

#if !defined MAX_HOUSE_NAME
    #define MAX_HOUSE_NAME          (32)
#endif

#if !defined MAX_HOUSES_PER_PLAYER
    #define MAX_HOUSES_PER_PLAYER   (3) // Máximo de casas que um jogador pode possuir
#endif

#if !defined HOUSE_FILE_PATH
    #define HOUSE_FILE_PATH         "houses/%d.ini"
#endif

#if !defined HOUSE_PICKUP_ID
    #define HOUSE_PICKUP_ID         (1273) // Ícone de casa
#endif

#if !defined HOUSE_RENT_PICKUP_ID
    #define HOUSE_RENT_PICKUP_ID    (1272) // Ícone de aluguel
#endif

#if !defined HOUSE_OWNED_PICKUP_ID
    #define HOUSE_OWNED_PICKUP_ID   (19522) // Ícone de casa ocupada
#endif

#if !defined HOUSE_LABEL_COLOR_SALE
    #define HOUSE_LABEL_COLOR_SALE  (0x00FF00FF) // Verde - À venda
#endif

#if !defined HOUSE_LABEL_COLOR_OWNED
    #define HOUSE_LABEL_COLOR_OWNED (0xFF0000FF) // Vermelho - Ocupada
#endif

#if !defined HOUSE_LABEL_COLOR_RENT
    #define HOUSE_LABEL_COLOR_RENT  (0xFFFF00FF) // Amarelo - Para alugar
#endif

// ============================================================================
// ENUMERADORES
// ============================================================================

enum E_HOUSE_DATA {
    houseEnexID,              // ID do enex associado
    housePrice,               // Preço de compra
    houseRentPrice,           // Preço de aluguel (0 = não disponível)
    houseInterior,            // ID do interior
    houseName[MAX_HOUSE_NAME],// Nome da casa
    houseOwner[MAX_PLAYER_NAME], // Nome do dono
    bool:houseOwned,          // Se a casa está ocupada
    bool:houseLocked,         // Se a casa está trancada
    housePickup,              // DEPRECATED - Não usado (Enex gerencia pickups)
    Text3D:houseLabel,        // Label 3D
    Float:houseX,             // Posição X da entrada
    Float:houseY,             // Posição Y da entrada
    Float:houseZ,             // Posição Z da entrada
    houseVirtualWorld,        // Virtual World único da casa
    houseWeapons[10],         // Armas guardadas
    houseAmmo[10],            // Munição das armas
    houseMoney,               // Dinheiro guardado
    // Sistema de aluguel
    houseRenter[MAX_PLAYER_NAME], // Nome do inquilino
    bool:houseRented,         // Se a casa está alugada
    houseRentDay,             // Timestamp do último pagamento de aluguel
    houseRentDaysOwed         // Dias de aluguel devidos ao dono
}

// ============================================================================
// VARIÁVEIS
// ============================================================================

static 
    HouseData[MAX_HOUSES][E_HOUSE_DATA],
    Iterator:HouseIterator<MAX_HOUSES>,
    PlayerHouses[MAX_PLAYERS][MAX_HOUSES_PER_PLAYER] = {-1, ...}, // Array de casas do jogador
    PlayerHousesCount[MAX_PLAYERS]; // Quantidade de casas que o jogador possui

// Array público para acesso externo (comandos, etc)
new PlayerRentHouseID[MAX_PLAYERS] = {-1, ...};

// ============================================================================
// FORWARDS (Opcionais - implemente no seu gamemode se necessário)
// ============================================================================

#if defined _ALS_OnHouseCreated
    #undef OnHouseCreated
#else
    #define _ALS_OnHouseCreated
#endif
#define OnHouseCreated eHouse_OnHouseCreated
#if defined eHouse_OnHouseCreated
    forward eHouse_OnHouseCreated(houseid);
#endif

#if defined _ALS_OnHouseDeleted
    #undef OnHouseDeleted
#else
    #define _ALS_OnHouseDeleted
#endif
#define OnHouseDeleted eHouse_OnHouseDeleted
#if defined eHouse_OnHouseDeleted
    forward eHouse_OnHouseDeleted(houseid);
#endif

#if defined _ALS_OnPlayerBuyHouse
    #undef OnPlayerBuyHouse
#else
    #define _ALS_OnPlayerBuyHouse
#endif
#define OnPlayerBuyHouse eHouse_OnPlayerBuyHouse
#if defined eHouse_OnPlayerBuyHouse
    forward eHouse_OnPlayerBuyHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerSellHouse
    #undef OnPlayerSellHouse
#else
    #define _ALS_OnPlayerSellHouse
#endif
#define OnPlayerSellHouse eHouse_OnPlayerSellHouse
#if defined eHouse_OnPlayerSellHouse
    forward eHouse_OnPlayerSellHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerEnterHouse
    #undef OnPlayerEnterHouse
#else
    #define _ALS_OnPlayerEnterHouse
#endif
#define OnPlayerEnterHouse eHouse_OnPlayerEnterHouse
#if defined eHouse_OnPlayerEnterHouse
    forward eHouse_OnPlayerEnterHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerExitHouse
    #undef OnPlayerExitHouse
#else
    #define _ALS_OnPlayerExitHouse
#endif
#define OnPlayerExitHouse eHouse_OnPlayerExitHouse
#if defined eHouse_OnPlayerExitHouse
    forward eHouse_OnPlayerExitHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerRentHouse
    #undef OnPlayerRentHouse
#else
    #define _ALS_OnPlayerRentHouse
#endif
#define OnPlayerRentHouse eHouse_OnPlayerRentHouse
#if defined eHouse_OnPlayerRentHouse
    forward eHouse_OnPlayerRentHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerUnrentHouse
    #undef OnPlayerUnrentHouse
#else
    #define _ALS_OnPlayerUnrentHouse
#endif
#define OnPlayerUnrentHouse eHouse_OnPlayerUnrentHouse
#if defined eHouse_OnPlayerUnrentHouse
    forward eHouse_OnPlayerUnrentHouse(playerid, houseid);
#endif

#if defined _ALS_OnPlayerTransferHouse
    #undef OnPlayerTransferHouse
#else
    #define _ALS_OnPlayerTransferHouse
#endif
#define OnPlayerTransferHouse eHouse_OnPlayerTransferHouse
#if defined eHouse_OnPlayerTransferHouse
    forward eHouse_OnPlayerTransferHouse(playerid, targetid, houseid);
#endif

// Callbacks de salvamento/carregamento (opcional - implemente para controle total)
#if defined _ALS_OnHouseSave
    #undef OnHouseSave
#else
    #define _ALS_OnHouseSave
#endif
#define OnHouseSave eHouse_OnHouseSave
#if defined eHouse_OnHouseSave
    forward eHouse_OnHouseSave(houseid);
#endif

#if defined _ALS_OnHouseLoad
    #undef OnHouseLoad
#else
    #define _ALS_OnHouseLoad
#endif
#define OnHouseLoad eHouse_OnHouseLoad
#if defined eHouse_OnHouseLoad
    forward eHouse_OnHouseLoad(houseid);
#endif

// ============================================================================
// FUNÇÕES PRINCIPAIS
// ============================================================================

/**
 * House_GetPlayerHouseCount
 * 
 * Retorna quantas casas o jogador possui
 * 
 * @param playerid  ID do jogador
 * @return Quantidade de casas
 */
stock House_GetPlayerHouseCount(playerid)
{
    return PlayerHousesCount[playerid];
}

/**
 * House_GetPlayerHouseBySlot
 * 
 * Retorna o ID da casa em um slot específico do jogador
 * 
 * @param playerid  ID do jogador
 * @param slot      Slot (0 a MAX_HOUSES_PER_PLAYER-1)
 * @return ID da casa ou -1 se inválido
 */
stock House_GetPlayerHouseBySlot(playerid, slot)
{
    if(slot < 0 || slot >= PlayerHousesCount[playerid])
        return -1;
    
    return PlayerHouses[playerid][slot];
}

/**
 * House_PlayerOwnsHouse
 * 
 * Verifica se o jogador possui uma casa específica
 * 
 * @param playerid  ID do jogador
 * @param houseid   ID da casa
 * @return 1 se possui, 0 caso contrário
 */
stock House_PlayerOwnsHouse(playerid, houseid)
{
    for(new i = 0; i < PlayerHousesCount[playerid]; i++)
    {
        if(PlayerHouses[playerid][i] == houseid)
            return 1;
    }
    return 0;
}

/**
 * House_Create
 * 
 * Cria uma nova casa
 * 
 * @param name          Nome da casa
 * @param price         Preço de compra
 * @param rentPrice     Preço de aluguel (0 para desabilitar)
 * @param Float:entX    Posição X da entrada
 * @param Float:entY    Posição Y da entrada
 * @param Float:entZ    Posição Z da entrada
 * @param Float:entAng  Ângulo da entrada
 * @param Float:intX    Posição X do interior
 * @param Float:intY    Posição Y do interior
 * @param Float:intZ    Posição Z do interior
 * @param Float:intAng  Ângulo do interior
 * @param interiorID    ID do interior da casa
 * @param entranceInteriorID  ID do interior onde a entrada está (padrão 0 = exterior)
 * 
 * @return ID da casa ou -1 em caso de erro
 */
stock House_Create(const name[], price, rentPrice, 
                   Float:entX, Float:entY, Float:entZ, Float:entAng,
                   Float:intX, Float:intY, Float:intZ, Float:intAng,
                   interiorID = 0, entranceInteriorID = 0)
{
    new houseid = Iter_Free(HouseIterator);
    if(houseid == -1) {
        print("[easyHouses] ERRO: Limite de casas atingido!");
        return -1;
    }

    // Criar enex associado com virtual world único
    new vw = houseid + 1000; // Virtual world único para cada casa
    
    // Determinar qual pickup usar (prioridade: aluguel > venda)
    new pickupToUse = (rentPrice > 0) ? HOUSE_RENT_PICKUP_ID : HOUSE_PICKUP_ID;
    
    new enexid = Enex_Create(
        name,
        entX, entY, entZ, entAng,
        intX, intY, intZ, intAng,
        vw, interiorID, // saída com VW único e interior da casa
        false, // freeze
        0, entranceInteriorID, // entrada no mundo 0 com interior especificado
        pickupToUse // Pickup correto baseado em aluguel/venda
    );

    if(enexid == -1) {
        print("[easyHouses] ERRO: Falha ao criar enex para a casa!");
        return -1;
    }

    // Fechar o enex (casas começam trancadas para não-donos)
    Enex_Close(enexid);
    
    // Destruir a label amarela do enex (vamos criar nossa própria label branca)
    Enex_DestroyEntranceLabel(enexid);

    // Inicializar dados da casa
    HouseData[houseid][houseEnexID] = enexid;
    HouseData[houseid][housePrice] = price;
    HouseData[houseid][houseRentPrice] = rentPrice;
    HouseData[houseid][houseInterior] = interiorID;
    HouseData[houseid][houseOwned] = false;
    HouseData[houseid][houseLocked] = true;
    HouseData[houseid][houseX] = entX;
    HouseData[houseid][houseY] = entY;
    HouseData[houseid][houseZ] = entZ;
    HouseData[houseid][houseVirtualWorld] = vw;
    HouseData[houseid][houseMoney] = 0;
    
    format(HouseData[houseid][houseName], MAX_HOUSE_NAME, "%s", name);
    HouseData[houseid][houseOwner][0] = EOS;

    // Limpar armas
    for(new i = 0; i < 10; i++) {
        HouseData[houseid][houseWeapons][i] = 0;
        HouseData[houseid][houseAmmo][i] = 0;
    }

    // Nota: Não criamos pickups aqui pois o Enex_Create já criou os pickups de entrada/saída
    // com os virtual worlds e interiores corretos
    HouseData[houseid][housePickup] = -1; // Não usado (Enex gerencia os pickups)

    HouseData[houseid][houseLabel] = CreateDynamic3DTextLabel(
        "Carregando...",
        HOUSE_LABEL_COLOR_SALE,
        entX, entY, entZ + 0.5,
        20.0, .testlos = 1, .streamdistance = 20.0
    );

    Iter_Add(HouseIterator, houseid);
    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);

    #if defined eHouse_OnHouseCreated
        OnHouseCreated(houseid);
    #endif

    return houseid;
}

/**
 * House_Delete
 * 
 * Deleta uma casa
 * 
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Delete(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    // Nota: Não destruímos o pickup pois o Enex gerencia seus próprios pickups
    DestroyDynamic3DTextLabel(HouseData[houseid][houseLabel]);

    Iter_Remove(HouseIterator, houseid);

    #if defined eHouse_OnHouseDeleted
        OnHouseDeleted(houseid);
    #endif

    return 1;
}

/**
 * House_Buy
 * 
 * Permite que um jogador compre uma casa
 * 
 * @param playerid  ID do jogador
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Buy(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(HouseData[houseid][houseOwned])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa já possui dono!");

    if(PlayerHousesCount[playerid] >= MAX_HOUSES_PER_PLAYER)
    {
        new string[96];
        format(string, sizeof(string), "{FF0000}[Erro] {FFFFFF}Você já possui o máximo de casas permitido (%d)!", MAX_HOUSES_PER_PLAYER);
        return SendClientMessage(playerid, -1, string);
    }

    if(Enex_GetPlayerMoney(playerid) < HouseData[houseid][housePrice])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não tem dinheiro suficiente!");

    Enex_GivePlayerMoney(playerid, -HouseData[houseid][housePrice]);
    
    GetPlayerName(playerid, HouseData[houseid][houseOwner], MAX_PLAYER_NAME);
    HouseData[houseid][houseOwned] = true;
    HouseData[houseid][houseLocked] = false;
    
    // Adicionar casa ao array do jogador
    PlayerHouses[playerid][PlayerHousesCount[playerid]] = houseid;
    PlayerHousesCount[playerid]++;

    // Abrir enex para o dono
    Enex_OpenForPlayer(playerid, HouseData[houseid][houseEnexID]);

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você comprou a casa '%s' por {00FF00}$%d{FFFFFF}! ({FFFF00}%d/%d casas{FFFFFF})", 
           HouseData[houseid][houseName], HouseData[houseid][housePrice], PlayerHousesCount[playerid], MAX_HOUSES_PER_PLAYER);
    SendClientMessage(playerid, -1, string);

    #if defined eHouse_OnPlayerBuyHouse
        OnPlayerBuyHouse(playerid, houseid);
    #endif

    return 1;
}

/**
 * House_Sell
 * 
 * Permite que um jogador venda sua casa
 * 
 * @param playerid  ID do jogador
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Sell(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!HouseData[houseid][houseOwned])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não está ocupada!");

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    new sellPrice = floatround(HouseData[houseid][housePrice] * 0.7); // 70% do valor
    Enex_GivePlayerMoney(playerid, sellPrice);

    HouseData[houseid][houseOwner][0] = EOS;
    HouseData[houseid][houseOwned] = false;
    HouseData[houseid][houseLocked] = true;
    HouseData[houseid][houseMoney] = 0;
    
    // Remover casa do array do jogador
    for(new i = 0; i < PlayerHousesCount[playerid]; i++)
    {
        if(PlayerHouses[playerid][i] == houseid)
        {
            // Shift array para remover elemento
            for(new j = i; j < PlayerHousesCount[playerid] - 1; j++)
            {
                PlayerHouses[playerid][j] = PlayerHouses[playerid][j + 1];
            }
            PlayerHouses[playerid][PlayerHousesCount[playerid] - 1] = -1;
            PlayerHousesCount[playerid]--;
            break;
        }
    }

    // Limpar armas
    for(new i = 0; i < 10; i++) {
        HouseData[houseid][houseWeapons][i] = 0;
        HouseData[houseid][houseAmmo][i] = 0;
    }

    // Fechar enex
    Enex_CloseForPlayer(playerid, HouseData[houseid][houseEnexID]);

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você vendeu sua casa por {00FF00}$%d{FFFFFF}!", sellPrice);
    SendClientMessage(playerid, -1, string);

    #if defined eHouse_OnPlayerSellHouse
        OnPlayerSellHouse(playerid, houseid);
    #endif

    return 1;
}

/**
 * House_ToggleLock
 * 
 * Tranca/Destranca uma casa
 * 
 * @param playerid  ID do jogador
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_ToggleLock(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    // Verificar se é dono OU inquilino
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    new bool:isOwner = bool:House_PlayerOwnsHouse(playerid, houseid);
    new bool:isRenter = (HouseData[houseid][houseRented] && strcmp(HouseData[houseid][houseRenter], name, false) == 0);
    
    if(!isOwner && !isRenter)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não é dono ou inquilino desta casa!");

    HouseData[houseid][houseLocked] = !HouseData[houseid][houseLocked];

    if(HouseData[houseid][houseLocked]) {
        // Trancar casa - fechar para todos
        Enex_Close(HouseData[houseid][houseEnexID]);
        
        // Dar permissão apenas ao dono e inquilino (se houver)
        foreach(new i : Player)
        {
            if(House_PlayerOwnsHouse(i, houseid))
            {
                Enex_OpenForPlayer(i, HouseData[houseid][houseEnexID]);
            }
            else if(HouseData[houseid][houseRented])
            {
                new renterName[MAX_PLAYER_NAME];
                GetPlayerName(i, renterName, MAX_PLAYER_NAME);
                if(!strcmp(HouseData[houseid][houseRenter], renterName, false))
                {
                    Enex_OpenForPlayer(i, HouseData[houseid][houseEnexID]);
                }
            }
        }
        
        SendClientMessage(playerid, -1, "{00FF00}[Casa] {FFFFFF}Casa trancada!");
    } else {
        // Destrancar casa - abrir para todos
        Enex_Open(HouseData[houseid][houseEnexID]);
        
        // Remover permissões específicas
        foreach(new i : Player)
        {
            Enex_CloseForPlayer(i, HouseData[houseid][houseEnexID]);
        }
        
        SendClientMessage(playerid, -1, "{00FF00}[Casa] {FFFFFF}Casa destrancada!");
    }

    House_UpdateLabel(houseid);
    House_Save(houseid);
    return 1;
}

/**
 * House_Transfer
 * 
 * Transfere a propriedade de uma casa de um jogador para outro
 * 
 * @param playerid  ID do jogador dono atual
 * @param targetid  ID do jogador que receberá a casa
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Transfer(playerid, targetid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Jogador não conectado!");

    if(playerid == targetid)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não pode transferir uma casa para si mesmo!");

    // Verificar se o jogador alvo tem espaço para mais casas
    if(PlayerHousesCount[targetid] >= MAX_HOUSES_PER_PLAYER)
    {
        new string[128];
        format(string, sizeof(string), "{FF0000}[Erro] {FFFFFF}O jogador já possui o máximo de casas permitido (%d)!", MAX_HOUSES_PER_PLAYER);
        return SendClientMessage(playerid, -1, string);
    }

    // Remover casa do jogador atual
    for(new i = 0; i < PlayerHousesCount[playerid]; i++)
    {
        if(PlayerHouses[playerid][i] == houseid)
        {
            // Shift array
            for(new j = i; j < PlayerHousesCount[playerid] - 1; j++)
            {
                PlayerHouses[playerid][j] = PlayerHouses[playerid][j + 1];
            }
            PlayerHouses[playerid][PlayerHousesCount[playerid] - 1] = -1;
            PlayerHousesCount[playerid]--;
            break;
        }
    }

    // Adicionar casa ao novo dono
    PlayerHouses[targetid][PlayerHousesCount[targetid]] = houseid;
    PlayerHousesCount[targetid]++;

    // Atualizar dados da casa
    new oldOwner[MAX_PLAYER_NAME], newOwner[MAX_PLAYER_NAME];
    GetPlayerName(playerid, oldOwner, MAX_PLAYER_NAME);
    GetPlayerName(targetid, newOwner, MAX_PLAYER_NAME);
    
    format(HouseData[houseid][houseOwner], MAX_PLAYER_NAME, "%s", newOwner);

    // Cancelar aluguel se houver
    if(HouseData[houseid][houseRented])
    {
        HouseData[houseid][houseRented] = false;
        HouseData[houseid][houseRenter][0] = EOS;
        HouseData[houseid][houseRentDay] = 0;
        HouseData[houseid][houseRentDaysOwed] = 0;
    }

    // Ajustar permissões do enex
    Enex_CloseForPlayer(playerid, HouseData[houseid][houseEnexID]);
    Enex_OpenForPlayer(targetid, HouseData[houseid][houseEnexID]);

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    // Mensagens
    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você transferiu a casa '{FFFF00}%s{FFFFFF}' para {FFD700}%s{FFFFFF}!", 
           HouseData[houseid][houseName], newOwner);
    SendClientMessage(playerid, -1, string);

    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você recebeu a casa '{FFFF00}%s{FFFFFF}' de {FFD700}%s{FFFFFF}!", 
           HouseData[houseid][houseName], oldOwner);
    SendClientMessage(targetid, -1, string);

    #if defined eHouse_OnPlayerTransferHouse
        OnPlayerTransferHouse(playerid, targetid, houseid);
    #endif

    return 1;
}

/**
 * House_Rent
 * 
 * Aluga uma casa para um jogador
 * 
 * @param playerid  ID do jogador que vai alugar
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Rent(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!HouseData[houseid][houseOwned])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não possui dono para alugar!");

    if(HouseData[houseid][houseRentPrice] <= 0)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não está disponível para aluguel!");

    if(House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não pode alugar sua própria casa!");

    if(HouseData[houseid][houseRented])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa já está alugada!");

    if(PlayerRentHouseID[playerid] != -1)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você já está alugando uma casa! Use /desalugar primeiro.");

    if(Enex_GetPlayerMoney(playerid) < HouseData[houseid][houseRentPrice])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não tem dinheiro suficiente para o primeiro pagamento!");

    // Cobrar primeiro pagamento
    Enex_GivePlayerMoney(playerid, -HouseData[houseid][houseRentPrice]);

    // Configurar aluguel
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    format(HouseData[houseid][houseRenter], MAX_PLAYER_NAME, "%s", name);
    HouseData[houseid][houseRented] = true;
    HouseData[houseid][houseRentDay] = gettime();
    HouseData[houseid][houseRentDaysOwed] = HouseData[houseid][houseRentPrice]; // Primeiro pagamento acumulado

    PlayerRentHouseID[playerid] = houseid;

    // Dar acesso à casa
    Enex_OpenForPlayer(playerid, HouseData[houseid][houseEnexID]);

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você alugou a casa '{FFFF00}%s{FFFFFF}' por {00FF00}$%d/dia{FFFFFF}!", 
           HouseData[houseid][houseName], HouseData[houseid][houseRentPrice]);
    SendClientMessage(playerid, -1, string);

    #if defined eHouse_OnPlayerRentHouse
        OnPlayerRentHouse(playerid, houseid);
    #endif

    return 1;
}

/**
 * House_Unrent
 * 
 * Cancela o aluguel de uma casa
 * 
 * @param playerid  ID do jogador inquilino
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Unrent(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!HouseData[houseid][houseRented])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não está alugada!");

    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);

    if(strcmp(HouseData[houseid][houseRenter], name, false))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não está alugando esta casa!");

    // Resetar aluguel
    HouseData[houseid][houseRented] = false;
    HouseData[houseid][houseRenter][0] = EOS;
    HouseData[houseid][houseRentDay] = 0;
    // Manter houseRentDaysOwed para o dono receber

    PlayerRentHouseID[playerid] = -1;

    // Remover acesso
    Enex_CloseForPlayer(playerid, HouseData[houseid][houseEnexID]);

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você cancelou o aluguel da casa '{FFFF00}%s{FFFFFF}'!", 
           HouseData[houseid][houseName]);
    SendClientMessage(playerid, -1, string);

    #if defined eHouse_OnPlayerUnrentHouse
        OnPlayerUnrentHouse(playerid, houseid);
    #endif

    return 1;
}

/**
 * House_EvictRenter
 * 
 * Despeja o inquilino (apenas o dono pode usar)
 * 
 * @param playerid  ID do jogador dono
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_EvictRenter(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    if(!HouseData[houseid][houseRented])
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não está alugada!");

    new renterName[MAX_PLAYER_NAME];
    format(renterName, sizeof(renterName), "%s", HouseData[houseid][houseRenter]);

    // Resetar aluguel
    HouseData[houseid][houseRented] = false;
    HouseData[houseid][houseRenter][0] = EOS;
    HouseData[houseid][houseRentDay] = 0;

    // Remover acesso do inquilino se estiver online
    foreach(new i : Player)
    {
        new name[MAX_PLAYER_NAME];
        GetPlayerName(i, name, MAX_PLAYER_NAME);
        
        if(!strcmp(name, renterName, false))
        {
            PlayerRentHouseID[i] = -1;
            Enex_CloseForPlayer(i, HouseData[houseid][houseEnexID]);
            
            new string[128];
            format(string, sizeof(string), "{FF0000}[Casa] {FFFFFF}Você foi despejado da casa '{FFFF00}%s{FFFFFF}'!", 
                   HouseData[houseid][houseName]);
            SendClientMessage(i, -1, string);
            break;
        }
    }

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você despejou {FFD700}%s{FFFFFF} da casa '{FFFF00}%s{FFFFFF}'!", 
           renterName, HouseData[houseid][houseName]);
    SendClientMessage(playerid, -1, string);

    return 1;
}

/**
 * House_SetRentPrice
 * 
 * Define o preço de aluguel diário da casa
 * 
 * @param playerid  ID do jogador dono
 * @param houseid   ID da casa
 * @param price     Preço do aluguel por dia (0 para desativar)
 * @return          1 em sucesso, 0 em falha
 */
stock House_SetRentPrice(playerid, houseid, price)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return 0;

    HouseData[houseid][houseRentPrice] = price;

    // Se definir preço 0 e tem inquilino, despejar
    if(price == 0 && HouseData[houseid][houseRented])
    {
        House_EvictRenter(playerid, houseid);
    }

    House_UpdateLabel(houseid);
    House_UpdatePickup(houseid);
    House_Save(houseid);

    return 1;
}

/**
 * House_GetRentPrice
 * 
 * Obtém o preço de aluguel diário da casa
 * 
 * @param houseid   ID da casa
 * @return          Preço do aluguel por dia
 */
stock House_GetRentPrice(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    return HouseData[houseid][houseRentPrice];
}

/**
 * House_CollectRent
 * 
 * Coleta o aluguel acumulado de uma casa
 * 
 * @param playerid  ID do jogador dono
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_CollectRent(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    if(HouseData[houseid][houseRentDaysOwed] <= 0)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Não há aluguel para coletar nesta casa!");

    new amount = HouseData[houseid][houseRentDaysOwed];
    Enex_GivePlayerMoney(playerid, amount);

    new string[128];
    format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Você coletou {00FF00}$%d{FFFFFF} de aluguel da casa '{FFFF00}%s{FFFFFF}'!", 
           amount, HouseData[houseid][houseName]);
    SendClientMessage(playerid, -1, string);

    HouseData[houseid][houseRentDaysOwed] = 0;
    House_Save(houseid);

    return 1;
}

/**
 * House_UpdateLabel
 * 
 * Atualiza a label 3D da casa
 * 
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_UpdateLabel(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    new string[256];
    new color = HOUSE_LABEL_COLOR_SALE;

    if(HouseData[houseid][houseOwned]) {
        color = HOUSE_LABEL_COLOR_OWNED;
        
        // Casa alugada
        if(HouseData[houseid][houseRented]) {
            format(string, sizeof(string),
                "{FFFFFF}Casa: {00FF00}%s\n\
                {FFFFFF}Dono: {FFD700}%s\n\
                {FFFFFF}Inquilino: {FFFF00}%s\n\
                {FFFFFF}Status: %s",
                HouseData[houseid][houseName],
                HouseData[houseid][houseOwner],
                HouseData[houseid][houseRenter],
                HouseData[houseid][houseLocked] ? ("{FF0000}Trancada") : ("{00FF00}Aberta")
            );
        }
        // Casa ocupada, disponível para aluguel
        else if(HouseData[houseid][houseRentPrice] > 0) {
            color = HOUSE_LABEL_COLOR_RENT;
            format(string, sizeof(string),
                "{FFFFFF}Casa: {00FF00}%s\n\
                {FFFFFF}Dono: {FFD700}%s\n\
                {FFFFFF}Aluguel: {FFFF00}$%d/dia\n\
                {FFFFFF}Status: %s\n\
                {FFFFFF}Digite {FFD700}/alugarcasa",
                HouseData[houseid][houseName],
                HouseData[houseid][houseOwner],
                HouseData[houseid][houseRentPrice],
                HouseData[houseid][houseLocked] ? ("{FF0000}Trancada") : ("{00FF00}Aberta")
            );
        }
        // Casa ocupada sem aluguel
        else {
            format(string, sizeof(string),
                "{FFFFFF}Casa: {00FF00}%s\n\
                {FFFFFF}Dono: {FFD700}%s\n\
                {FFFFFF}Status: %s",
                HouseData[houseid][houseName],
                HouseData[houseid][houseOwner],
                HouseData[houseid][houseLocked] ? ("{FF0000}Trancada") : ("{00FF00}Aberta")
            );
        }
    } else {
        if(HouseData[houseid][houseRentPrice] > 0) {
            color = HOUSE_LABEL_COLOR_RENT;
            format(string, sizeof(string),
                "{FFFFFF}Casa: {00FF00}%s\n\
                {FFFFFF}Preço: {00FF00}$%d\n\
                {FFFFFF}Aluguel: {FFFF00}$%d/dia\n\
                {FFFFFF}Digite {FFD700}/comprarcasa {FFFFFF}ou {FFD700}/alugarcasa",
                HouseData[houseid][houseName],
                HouseData[houseid][housePrice],
                HouseData[houseid][houseRentPrice]
            );
        } else {
            format(string, sizeof(string),
                "{FFFFFF}Casa: {00FF00}%s\n\
                {FFFFFF}Preço: {00FF00}$%d\n\
                {FFFFFF}Digite {FFD700}/comprarcasa",
                HouseData[houseid][houseName],
                HouseData[houseid][housePrice]
            );
        }
    }

    UpdateDynamic3DTextLabelText(HouseData[houseid][houseLabel], color, string);
    return 1;
}

/**
 * House_UpdatePickup
 * 
 * Atualiza o pickup de entrada da casa baseado no status
 * 
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_UpdatePickup(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    new pickupModel;
    
    // Determinar qual modelo de pickup usar
    if(HouseData[houseid][houseOwned]) {
        // Casa ocupada - usar ícone de propriedade
        pickupModel = HOUSE_OWNED_PICKUP_ID;
    } else {
        // Casa disponível - verificar se tem aluguel
        if(HouseData[houseid][houseRentPrice] > 0) {
            pickupModel = HOUSE_RENT_PICKUP_ID; // Aluguel disponível
        } else {
            pickupModel = HOUSE_PICKUP_ID; // Apenas venda
        }
    }
    
    // Atualizar o pickup do enex
    Enex_UpdateEntrancePickup(HouseData[houseid][houseEnexID], pickupModel);
    
    return 1;
}

/**
 * House_GetNearby
 * 
 * Retorna o ID da casa mais próxima do jogador
 * 
 * @param playerid  ID do jogador
 * @param Float:range Distância máxima
 * @return ID da casa ou -1 se não encontrada
 */
stock House_GetNearby(playerid, Float:range = 3.0)
{
    foreach(new i : HouseIterator) {
        if(IsPlayerInRangeOfPoint(playerid, range, 
           HouseData[i][houseX], HouseData[i][houseY], HouseData[i][houseZ]))
            return i;
    }
    return -1;
}

/**
 * House_IsOwner
 * 
 * Verifica se o jogador é dono da casa
 * 
 * @param playerid  ID do jogador
 * @param houseid   ID da casa
 * @return 1 se é dono, 0 caso contrário
 */
stock House_IsOwner(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    return !strcmp(HouseData[houseid][houseOwner], name, false);
}

/**
 * House_Save
 * 
 * Salva os dados da casa em arquivo usando DOF2
 * NOTA: Se você implementar OnHouseSave, este salvamento padrão não será usado
 * 
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Save(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    // Se o gamemode implementar callback customizado, usar ele
    #if defined eHouse_OnHouseSave
        return OnHouseSave(houseid);
    #else
        #if defined _dof2_included
            // Salvamento com DOF2
            new filepath[64];
            format(filepath, sizeof(filepath), HOUSE_FILE_PATH, houseid);

            // Criar arquivo se não existir
            if(!DOF2_FileExists(filepath))
                DOF2_CreateFile(filepath);

            // Salvar dados básicos
            // Salvar dados básicos
            new key_Name[] = "Name", key_Owner[] = "Owner", key_Price[] = "Price";
            new key_RentPrice[] = "RentPrice", key_Interior[] = "Interior";
            new key_Owned[] = "Owned", key_Locked[] = "Locked", key_Money[] = "Money";
            
            DOF2_SetString(filepath, key_Name, HouseData[houseid][houseName]);
            DOF2_SetString(filepath, key_Owner, HouseData[houseid][houseOwner]);
            DOF2_SetInt(filepath, key_Price, HouseData[houseid][housePrice]);
            DOF2_SetInt(filepath, key_RentPrice, HouseData[houseid][houseRentPrice]);
            DOF2_SetInt(filepath, key_Interior, HouseData[houseid][houseInterior]);
            DOF2_SetBool(filepath, key_Owned, HouseData[houseid][houseOwned]);
            DOF2_SetBool(filepath, key_Locked, HouseData[houseid][houseLocked]);
            DOF2_SetInt(filepath, key_Money, HouseData[houseid][houseMoney]);
            
            // Salvar posições
            new key_PosX[] = "PosX", key_PosY[] = "PosY", key_PosZ[] = "PosZ", key_VW[] = "VirtualWorld";
            DOF2_SetFloat(filepath, key_PosX, HouseData[houseid][houseX]);
            DOF2_SetFloat(filepath, key_PosY, HouseData[houseid][houseY]);
            DOF2_SetFloat(filepath, key_PosZ, HouseData[houseid][houseZ]);
            DOF2_SetInt(filepath, key_VW, HouseData[houseid][houseVirtualWorld]);
            
            // Salvar dados de aluguel
            new key_Renter[] = "Renter", key_Rented[] = "Rented", key_RentDay[] = "RentDay", key_RentDays[] = "RentDaysOwed";
            DOF2_SetString(filepath, key_Renter, HouseData[houseid][houseRenter]);
            DOF2_SetBool(filepath, key_Rented, HouseData[houseid][houseRented]);
            DOF2_SetInt(filepath, key_RentDay, HouseData[houseid][houseRentDay]);
            DOF2_SetInt(filepath, key_RentDays, HouseData[houseid][houseRentDaysOwed]);
            
            // Salvar armas guardadas
            for(new i = 0; i < 10; i++)
            {
                new key[16];
                format(key, sizeof(key), "Weapon%d", i);
                DOF2_SetInt(filepath, key, HouseData[houseid][houseWeapons][i]);
                
                format(key, sizeof(key), "Ammo%d", i);
                DOF2_SetInt(filepath, key, HouseData[houseid][houseAmmo][i]);
            }

            return 1;
        #else
            // Salvamento padrão sem DOF2 (básico com fwrite)
            new filepath[64];
            format(filepath, sizeof(filepath), HOUSE_FILE_PATH, houseid);

            new File:file = fopen(filepath, io_write);
            if(!file) return 0;

            new string[256];
            
            format(string, sizeof(string), "Name=%s\r\n", HouseData[houseid][houseName]);
            fwrite(file, string);
            
            format(string, sizeof(string), "Owner=%s\r\n", HouseData[houseid][houseOwner]);
            fwrite(file, string);
            
            format(string, sizeof(string), "Price=%d\r\n", HouseData[houseid][housePrice]);
            fwrite(file, string);
            
            format(string, sizeof(string), "RentPrice=%d\r\n", HouseData[houseid][houseRentPrice]);
            fwrite(file, string);
            
            format(string, sizeof(string), "Owned=%d\r\n", HouseData[houseid][houseOwned]);
            fwrite(file, string);
            
            format(string, sizeof(string), "Locked=%d\r\n", HouseData[houseid][houseLocked]);
            fwrite(file, string);
            
            format(string, sizeof(string), "Money=%d\r\n", HouseData[houseid][houseMoney]);
            fwrite(file, string);

            fclose(file);
            return 1;
        #endif
    #endif
}

/**
 * House_Load
 * 
 * Carrega os dados da casa de um arquivo usando DOF2
 * NOTA: Se você implementar OnHouseLoad, este carregamento padrão não será usado
 * 
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_Load(houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    // Se o gamemode implementar callback customizado, usar ele
    #if defined eHouse_OnHouseLoad
        return OnHouseLoad(houseid);
    #else
        #if defined _dof2_included
            // Carregamento com DOF2
            new filepath[64];
            format(filepath, sizeof(filepath), HOUSE_FILE_PATH, houseid);

            if(!DOF2_FileExists(filepath))
                return 0;

            // Carregar dados básicos
            new key_Owner[] = "Owner", key_Price[] = "Price", key_RentPrice[] = "RentPrice";
            new key_Owned[] = "Owned", key_Locked[] = "Locked", key_Money[] = "Money";
            
            DOF2_GetStringEx(filepath, key_Owner, HouseData[houseid][houseOwner], MAX_PLAYER_NAME);
            HouseData[houseid][housePrice] = DOF2_GetInt(filepath, key_Price);
            HouseData[houseid][houseRentPrice] = DOF2_GetInt(filepath, key_RentPrice);
            HouseData[houseid][houseOwned] = DOF2_GetBool(filepath, key_Owned);
            HouseData[houseid][houseLocked] = DOF2_GetBool(filepath, key_Locked);
            HouseData[houseid][houseMoney] = DOF2_GetInt(filepath, key_Money);
            
            // Carregar dados de aluguel
            new key_Renter[] = "Renter", key_Rented[] = "Rented";
            new key_RentDay[] = "RentDay", key_RentDaysOwed[] = "RentDaysOwed";
            
            DOF2_GetStringEx(filepath, key_Renter, HouseData[houseid][houseRenter], MAX_PLAYER_NAME);
            HouseData[houseid][houseRented] = DOF2_GetBool(filepath, key_Rented);
            HouseData[houseid][houseRentDay] = DOF2_GetInt(filepath, key_RentDay);
            HouseData[houseid][houseRentDaysOwed] = DOF2_GetInt(filepath, key_RentDaysOwed);
            
            // Carregar armas guardadas
            for(new i = 0; i < 10; i++)
            {
                new key[16];
                format(key, sizeof(key), "Weapon%d", i);
                HouseData[houseid][houseWeapons][i] = DOF2_GetInt(filepath, key);
                
                format(key, sizeof(key), "Ammo%d", i);
                HouseData[houseid][houseAmmo][i] = DOF2_GetInt(filepath, key);
            }

            // Atualizar label e status do enex
            House_UpdateLabel(houseid);
            House_UpdatePickup(houseid);
            
            // Configurar estado do enex baseado em locked e owned
            if(HouseData[houseid][houseOwned]) {
                if(HouseData[houseid][houseLocked]) {
                    // Casa ocupada e trancada - fechar para todos
                    Enex_Close(HouseData[houseid][houseEnexID]);
                    
                    // Dar permissão ao dono e inquilino se estiverem online
                    foreach(new i : Player)
                    {
                        if(House_PlayerOwnsHouse(i, houseid))
                        {
                            Enex_OpenForPlayer(i, HouseData[houseid][houseEnexID]);
                        }
                        else if(HouseData[houseid][houseRented])
                        {
                            new name[MAX_PLAYER_NAME];
                            GetPlayerName(i, name, MAX_PLAYER_NAME);
                            if(!strcmp(HouseData[houseid][houseRenter], name, false))
                            {
                                Enex_OpenForPlayer(i, HouseData[houseid][houseEnexID]);
                            }
                        }
                    }
                } else {
                    // Casa ocupada mas destrancada - abrir para todos
                    Enex_Open(HouseData[houseid][houseEnexID]);
                }
            } else {
                // Casa livre - abrir para todos
                Enex_Open(HouseData[houseid][houseEnexID]);
            }

            return 1;
        #else
            // Carregamento padrão sem DOF2
            new filepath[64];
            format(filepath, sizeof(filepath), HOUSE_FILE_PATH, houseid);

            if(!fexist(filepath))
                return 0;

            // Implementação básica - requer OnHouseLoad para funcionalidade completa
            printf("[easyHouses] Aviso: DOF2 não disponível. Implemente OnHouseLoad para carregar casa %d", houseid);
            
            return 0;
        #endif
    #endif
}

/**
 * House_LoadPlayerHouses
 * 
 * Carrega todas as casas de um jogador (chamado automaticamente no OnPlayerConnect)
 * Esta função percorre todas as casas e adiciona as que pertencem ao jogador
 * 
 * @param playerid   ID do jogador
 * @return Quantidade de casas carregadas
 */
stock House_LoadPlayerHouses(playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    PlayerHousesCount[playerid] = 0;
    
    foreach(new houseid : HouseIterator)
    {
        // Verificar se a casa pertence ao jogador E se está ocupada
        if(HouseData[houseid][houseOwned] && !strcmp(HouseData[houseid][houseOwner], name, false))
        {
            if(PlayerHousesCount[playerid] < MAX_HOUSES_PER_PLAYER)
            {
                PlayerHouses[playerid][PlayerHousesCount[playerid]] = houseid;
                PlayerHousesCount[playerid]++;
                
                // Abrir enex para o dono
                Enex_OpenForPlayer(playerid, HouseData[houseid][houseEnexID]);
            }
        }
    }
    
    return PlayerHousesCount[playerid];
}

/**
 * House_SaveAll
 * 
 * Salva todas as casas existentes
 * Útil para usar no OnGameModeExit ou periodicamente
 * 
 * @return Quantidade de casas salvas
 */
stock House_SaveAll()
{
    new count = 0;
    foreach(new houseid : HouseIterator)
    {
        if(House_Save(houseid))
            count++;
    }
    
    if(count > 0)
        printf("[easyHouses] %d casas salvas com sucesso!", count);
    
    return count;
}

/**
 * House_LoadAll
 * 
 * Carrega todas as casas de arquivos existentes
 * Útil para usar no OnGameModeInit
 * NOTA: As casas devem ser criadas primeiro com House_Create,
 *       esta função apenas carrega os dados salvos (owner, locked, etc)
 * 
 * @return Quantidade de casas carregadas
 */
stock House_LoadAll()
{
    new count = 0;
    foreach(new houseid : HouseIterator)
    {
        if(House_Load(houseid))
            count++;
    }
    
    if(count > 0)
        printf("[easyHouses] %d casas carregadas com sucesso!", count);
    
    return count;
}

/**
 * House_ProcessRentPayment
 * 
 * Processa pagamento de aluguel de uma casa alugada
 * Chamado quando o inquilino conecta
 * 
 * @param playerid  ID do jogador inquilino
 * @param houseid   ID da casa
 * @return 1 em sucesso, 0 em falha
 */
stock House_ProcessRentPayment(playerid, houseid)
{
    if(!Iter_Contains(HouseIterator, houseid))
        return 0;

    if(!HouseData[houseid][houseRented])
        return 0;

    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);

    if(strcmp(HouseData[houseid][houseRenter], name, false))
        return 0;

    // Calcular dias passados desde o último pagamento
    new currentTime = gettime();
    new daysPassed = (currentTime - HouseData[houseid][houseRentDay]) / 86400; // 86400 segundos = 1 dia

    if(daysPassed > 0)
    {
        new amountDue = daysPassed * HouseData[houseid][houseRentPrice];
        
        // Verificar se o jogador tem dinheiro
        if(Enex_GetPlayerMoney(playerid) >= amountDue)
        {
            // Cobrar aluguel
            Enex_GivePlayerMoney(playerid, -amountDue);
            HouseData[houseid][houseRentDay] = currentTime;
            HouseData[houseid][houseRentDaysOwed] += amountDue; // Acumular valor pago
            
            new string[128];
            format(string, sizeof(string), "{FFFF00}[Casa] {FFFFFF}Aluguel pago: {00FF00}$%d{FFFFFF} (%d dias) - Casa '{FFFF00}%s{FFFFFF}'", 
                   amountDue, daysPassed, HouseData[houseid][houseName]);
            SendClientMessage(playerid, -1, string);
            
            House_Save(houseid);
            return 1;
        }
        else
        {
            // Não tem dinheiro - despejar
            new string[128];
            format(string, sizeof(string), "{FF0000}[Casa] {FFFFFF}Você foi despejado da casa '{FFFF00}%s{FFFFFF}' por falta de pagamento!", 
                   HouseData[houseid][houseName]);
            SendClientMessage(playerid, -1, string);
            
            HouseData[houseid][houseRented] = false;
            HouseData[houseid][houseRenter][0] = EOS;
            HouseData[houseid][houseRentDay] = 0;
            
            PlayerRentHouseID[playerid] = -1;
            Enex_CloseForPlayer(playerid, HouseData[houseid][houseEnexID]);
            
            House_UpdateLabel(houseid);
            House_UpdatePickup(houseid);
            House_Save(houseid);
            
            return 0;
        }
    }

    return 1;
}

// ============================================================================
// COMANDOS DE EXEMPLO
// ============================================================================

/*
CMD:comprarcasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_Buy(playerid, houseid);
    return 1;
}

CMD:vendercasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");
        
    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não é sua!");

    House_Sell(playerid, houseid);
    return 1;
}

CMD:trancarcasa(playerid)
{
    new houseid = House_GetNearby(playerid);
    if(houseid == -1)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");
        
    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Esta casa não é sua!");

    House_ToggleLock(playerid, houseid);
    return 1;
}

CMD:minhascasas(playerid)
{
    new count = House_GetPlayerHouseCount(playerid);
    if(count == 0)
        return SendClientMessage(playerid, -1, "{FF0000}[Erro] {FFFFFF}Você não possui nenhuma casa!");
    
    new string[128];
    format(string, sizeof(string), "{00FF00}=== Suas Casas ({FFFF00}%d/%d{00FF00}) ===", count, MAX_HOUSES_PER_PLAYER);
    SendClientMessage(playerid, -1, string);
    
    for(new i = 0; i < count; i++)
    {
        new houseid = House_GetPlayerHouseBySlot(playerid, i);
        if(houseid != -1)
        {
            format(string, sizeof(string), "{FFFFFF}%d. {00FF00}%s {FFFFFF}| Preço: {00FF00}$%d {FFFFFF}| Status: %s",
                i + 1,
                HouseData[houseid][houseName],
                HouseData[houseid][housePrice],
                HouseData[houseid][houseLocked] ? "{FF0000}Trancada" : "{00FF00}Aberta"
            );
            SendClientMessage(playerid, -1, string);
        }
    }
    return 1;
}
*/

// ============================================================================
// HOOKS
// ============================================================================

hook OnPlayerConnect(playerid)
{
    // Resetar array de casas
    PlayerHousesCount[playerid] = 0;
    for(new i = 0; i < MAX_HOUSES_PER_PLAYER; i++)
    {
        PlayerHouses[playerid][i] = -1;
    }
    PlayerRentHouseID[playerid] = -1;
    
    // Carregar casas do jogador (delay para garantir que o nome foi carregado)
    defer House_DelayedLoad(playerid);

    #if defined eHouse_OnPlayerConnect
        return eHouse_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}

timer House_DelayedLoad[500](playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    // Carregar casas próprias
    House_LoadPlayerHouses(playerid);
    
    // Verificar se tem casa alugada e processar pagamento
    foreach(new houseid : HouseIterator)
    {
        if(HouseData[houseid][houseRented] && !strcmp(HouseData[houseid][houseRenter], name, false))
        {
            PlayerRentHouseID[playerid] = houseid;
            
            // Processar pagamento de aluguel
            House_ProcessRentPayment(playerid, houseid);
            
            // Se ainda está alugada após processamento, dar acesso
            if(HouseData[houseid][houseRented])
            {
                Enex_OpenForPlayer(playerid, HouseData[houseid][houseEnexID]);
            }
            
            break; // Jogador só pode alugar uma casa
        }
    }
}

hook OnPlayerDisconnect(playerid, reason)
{
    // Salvar todas as casas do jogador
    for(new i = 0; i < PlayerHousesCount[playerid]; i++)
    {
        new houseid = PlayerHouses[playerid][i];
        if(houseid != -1)
            House_Save(houseid);
    }
    
    // Salvar casa alugada se houver
    if(PlayerRentHouseID[playerid] != -1)
    {
        House_Save(PlayerRentHouseID[playerid]);
    }
    
    PlayerHousesCount[playerid] = 0;
    PlayerRentHouseID[playerid] = -1;

    #if defined eHouse_OnPlayerDisconnect
        return eHouse_OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}

hook OnGameModeExit()
{
    // Salvar todas as casas ao fechar o servidor
    House_SaveAll();

    #if defined eHouse_OnGameModeExit
        return eHouse_OnGameModeExit();
    #else
        return 1;
    #endif
}