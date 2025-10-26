/*
 * demo_houses.pwn - Demonstração do módulo easyHouses
 * 
 * Este arquivo mostra como usar o sistema de casas integrado com easyEnex
 * Inclui salvamento automático com DOF2
 * Novas funcionalidades: Aluguel e Transferência
 */

#include <a_samp>
#include <DOF2>
#include <easyEnex>
#include <extras/easyHouses>
#include <sscanf2>
#include <zcmd>

// Variáveis das casas
new cjhouse, sweet_house, og_loc_house;

main() 
{
    print("\n========================================");
    print(" easyHouses - Demonstracao");
    print(" Sistema de Casas v1.1 com DOF2");
    print(" Multiplas casas por jogador");
    print(" Salvamento automatico habilitado");
    print("========================================\n");
}

public OnGameModeInit()
{
    SetGameModeText("easyHouses Demo");
    DisableInteriorEnterExits();
    
    // Casa 1 - Casa do CJ (Grove Street) - Pode ser comprada
    cjhouse = House_Create(
        "Casa dos Johnson",          // Nome
        50000,                       // Preço de compra
        500,                         // Aluguel por dia
        2495.3, -1690.3, 14.8, 0.0, // Entrada (exterior)
        2496.05, -1695.17, 1014.74, 180.0, // Interior
        3                            // ID do interior (CJ's house)
    );

    // Casa 2 - Casa do Sweet - Mais cara, sem aluguel
    sweet_house = House_Create(
        "Casa do Sweet",
        75000,                       // Preço de compra
        0,                           // Sem aluguel (apenas venda)
        2523.03, -1679.25, 15.49, 87.19, // Entrada
        2496.05, -1695.17, 1014.74, 180.0, // Mesmo interior do CJ (família)
        3
    );

    // Casa 3 - Casa do OG Loc - Barata com aluguel disponível
    og_loc_house = House_Create(
        "Casa do OG Loc",
        25000,                       // Preço de compra
        300,                         // Aluguel por dia
        2486.37, -1644.88, 14.07, 180.91, // Entrada
        512.92, -11.69, 1001.56, 180.0, // Interior diferente
        3
    );

    // Carregar dados salvos das casas (owners, locked, etc)
    House_LoadAll();

    print("========================================");
    print(" 3 casas criadas na Grove Street");
    print(" Dados salvos carregados com DOF2");
    print(" Use /ajuda para ver os comandos");
    print("========================================");
    
    return 1;
}

public OnGameModeExit()
{
    // Salvar dados das casas antes de sair
    House_SaveAll();
    DOF2_Exit();
    print("Dados das casas salvos com DOF2.");
    return 1;
}

public OnPlayerSpawn(playerid)
{
    // Dar dinheiro inicial para testes
    new money = 50000 + random(500000);
    SetPlayerPosEnEx(playerid, 2495.26, -1684.67, 13.51, 179.31); // Grove Street
    GivePlayerMoney(playerid, money);
    SetPlayerScore(playerid, random(10));
    
    // Mensagem de boas-vindas
    SendClientMessage(playerid, 0xFFFF00FF, "=== easyHouses - Sistema de Casas ===");
    SendClientMessage(playerid, -1, "Use /ajuda para ver os comandos disponíveis.");
    SendClientMessage(playerid, -1, "Você recebeu dinheiro inicial para comprar casas!");
    SendClientMessage(playerid, 0x00FF00FF, "Você pode ter até 3 casas simultaneamente!");
    
    return 1;
}

// ============================================================================
// COMANDOS DE CASAS
// ============================================================================

CMD:comprarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_Buy(playerid, houseid);
    return 1;
}

CMD:vendercasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    House_Sell(playerid, houseid);
    return 1;
}

CMD:trancarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a sua casa!");

    // Verificar se é dono OU inquilino
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    new bool:isOwner = bool:House_PlayerOwnsHouse(playerid, houseid);
    new bool:isRenter = (PlayerRentHouseID[playerid] == houseid);
    
    if(!isOwner && !isRenter)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não é dono ou inquilino desta casa!");

    House_ToggleLock(playerid, houseid);
    return 1;
}

// Alias para trancar
CMD:tc(playerid)
{
    return callcmd::trancarcasa(playerid);
}

CMD:alugarcasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_Rent(playerid, houseid);
    return 1;
}

CMD:desalugar(playerid)
{
    if(PlayerRentHouseID[playerid] == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está alugando nenhuma casa!");
    
    House_Unrent(playerid, PlayerRentHouseID[playerid]);
    return 1;
}

CMD:despejar(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_EvictRenter(playerid, houseid);
    return 1;
}

CMD:coletaraluguel(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_CollectRent(playerid, houseid);
    return 1;
}

CMD:transferircasa(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Uso] {FFFFFF}/transferircasa [id do jogador]");
    
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    House_Transfer(playerid, targetid, houseid);
    return 1;
}

CMD:definirvalor(playerid, params[])
{
    new valor;
    if(sscanf(params, "d", valor))
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Uso] {FFFFFF}/definirvalor [valor] (0 para desativar aluguel)");
    
    if(valor < 0)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Valor não pode ser negativo!");
    
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    if(!House_PlayerOwnsHouse(playerid, houseid))
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Esta não é sua casa!");

    // Usar a função da API para definir o valor
    if(!House_SetRentPrice(playerid, houseid, valor))
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Não foi possível definir o valor!");
    
    new string[128];
    if(valor > 0)
    {
        format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Valor de aluguel definido para {00FF00}$%d/dia{FFFFFF}!", valor);
    }
    else
    {
        format(string, sizeof(string), "{00FF00}[Casa] {FFFFFF}Aluguel desativado! Casa disponível apenas para compra.");
    }
    SendClientMessage(playerid, -1, string);
    
    return 1;
}

CMD:ajuda(playerid)
{
    SendClientMessage(playerid, 0x00FF00FF, "========== Comandos de Casas ==========");
    SendClientMessage(playerid, -1, "/comprarcasa - Compra uma casa (próximo a ela)");
    SendClientMessage(playerid, -1, "/vendercasa - Vende sua casa (70% do valor)");
    SendClientMessage(playerid, -1, "/trancarcasa ou /tc - Tranca/Destranca casa (dono/inquilino)");
    SendClientMessage(playerid, -1, "/minhascasas - Lista todas as suas casas");
    SendClientMessage(playerid, -1, "/infocasa - Informações da casa próxima");
    SendClientMessage(playerid, -1, "{FFFF00}=== Comandos de Aluguel (Inquilino) ===");
    SendClientMessage(playerid, -1, "/alugarcasa - Aluga uma casa próxima");
    SendClientMessage(playerid, -1, "/desalugar - Cancela o aluguel da casa");
    SendClientMessage(playerid, -1, "{FFFF00}=== Comandos de Aluguel (Dono) ===");
    SendClientMessage(playerid, -1, "/definirvalor [valor] - Define preço de aluguel (0 = desativar)");
    SendClientMessage(playerid, -1, "/despejar - Despeja inquilino");
    SendClientMessage(playerid, -1, "/coletaraluguel - Coleta aluguel acumulado");
    SendClientMessage(playerid, -1, "{FFFF00}=== Comandos de Transferência ===");
    SendClientMessage(playerid, -1, "/transferircasa [id] - Transfere casa para jogador");
    SendClientMessage(playerid, -1, "{00FF00}=== Outros ===");
    SendClientMessage(playerid, -1, "/dinheiro - Adiciona $50.000 para testes");
    SendClientMessage(playerid, 0x00FF00FF, "=========================================");
    return 1;
}

CMD:minhascasas(playerid)
{
    new count = House_GetPlayerHouseCount(playerid);
    if(count == 0)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não possui nenhuma casa!");
    
    new string[128];
    format(string, sizeof(string), "{00FF00}=== Suas Casas ({FFFF00}%d/%d{00FF00}) ===", count, MAX_HOUSES_PER_PLAYER);
    SendClientMessage(playerid, -1, string);
    
    for(new i = 0; i < count; i++)
    {
        new houseid = House_GetPlayerHouseBySlot(playerid, i);
        if(houseid != -1)
        {
            new housename[MAX_HOUSE_NAME];
            // Pegar o nome da casa (usando IDs conhecidos)
            if(houseid == cjhouse) housename = "Casa dos Johnson";
            else if(houseid == sweet_house) housename = "Casa do Sweet";
            else if(houseid == og_loc_house) housename = "Casa do OG Loc";
            else format(housename, sizeof(housename), "Casa #%d", houseid);
            
            format(string, sizeof(string), "{FFFFFF}%d. {00FF00}%s {FFFFFF}| ID: {FFFF00}%d",
                i + 1, housename, houseid
            );
            SendClientMessage(playerid, -1, string);
        }
    }
    
    SendClientMessage(playerid, 0xFFFF00FF, "Use /infocasa próximo a uma casa para mais detalhes");
    return 1;
}

CMD:infocasa(playerid)
{
    new houseid = House_GetNearby(playerid, 3.0);
    if(houseid == -1)
        return SendClientMessage(playerid, 0xFF0000FF, "{FF0000}[Erro] {FFFFFF}Você não está próximo a nenhuma casa!");

    new string[256];
    if(House_PlayerOwnsHouse(playerid, houseid))
    {
        format(string, sizeof(string),
            "{00FF00}=== Sua Casa ===\n\
            {FFFFFF}Você é o proprietário desta casa\n\
            {FFFFFF}Use /tc para trancar/destrancar\n\
            {FFFFFF}Use /vendercasa para vender (70%% do valor)\n\
            {FFFFFF}Use /minhascasas para ver todas as suas casas"
        );
    }
    else
    {
        format(string, sizeof(string),
            "{FFFF00}=== Casa Disponível ===\n\
            {FFFFFF}Esta casa está disponível\n\
            {FFFFFF}Use /comprarcasa para adquirir\n\
            {FFFFFF}Veja o preço na label 3D acima da entrada\n\
            {FFFFFF}Você pode ter até %d casas", MAX_HOUSES_PER_PLAYER
        );
    }
    SendClientMessage(playerid, -1, string);
    return 1;
}

// Comando de teste para dar dinheiro
CMD:dinheiro(playerid)
{
    GivePlayerMoney(playerid, 50000);
    SendClientMessage(playerid, 0x00FF00FF, "{00FF00}[Teste] {FFFFFF}Você recebeu $50.000!");
    return 1;
}

// Comando para ver posição (útil para desenvolvimento)
CMD:pos(playerid)
{
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    printf("%.2f, %.2f, %.2f, %.2f", x, y, z, a);
    SendClientMessage(playerid, -1, "{00FF00}[Debug] {FFFFFF}Coordenadas enviadas para o console.");
    return 1;
}

// ============================================================================
// CALLBACKS CUSTOMIZADOS - EASYHOUSES
// ============================================================================

public OnPlayerBuyHouse(playerid, houseid)
{
    // Anunciar para todos
    new string[128], name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    new count = House_GetPlayerHouseCount(playerid);
    format(string, sizeof(string), 
        "{FFD700}** [Imóveis] {FFFFFF}%s comprou uma casa na Grove Street! (%d/%d casas)", 
        name, count, MAX_HOUSES_PER_PLAYER);
    SendClientMessageToAll(0xFFD700FF, string);
    
    // Game text estilizado
    GameTextForPlayer(playerid, "~g~~h~PARABENS!~n~~w~Voce comprou uma casa!", 3000, 3);
    
    // Dar bônus de boas-vindas
    SendClientMessage(playerid, 0x00FF00FF, 
        "{00FF00}[Casa] {FFFFFF}Parabéns! Você ganhou $5.000 de bônus de boas-vindas!");
    GivePlayerMoney(playerid, 5000);
    
    return 1;
}

public OnPlayerSellHouse(playerid, houseid)
{
    // Anunciar venda
    new string[128], name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
    
    new count = House_GetPlayerHouseCount(playerid);
    format(string, sizeof(string), 
        "{FF8800}** [Imóveis] {FFFFFF}%s vendeu uma casa. (%d/%d casas restantes)", 
        name, count, MAX_HOUSES_PER_PLAYER);
    SendClientMessageToAll(0xFF8800FF, string);
    
    GameTextForPlayer(playerid, "~y~Casa vendida!", 2000, 3);
    
    return 1;
}

public OnPlayerEnterHouse(playerid, houseid)
{
    // Mensagem de boas-vindas personalizada por casa
    if(houseid == cjhouse)
    {
        GameTextForPlayer(playerid, "~w~Bem-vindo a ~g~Casa dos Johnson!", 3000, 1);
        SendClientMessage(playerid, -1, 
            "{00FF00}[Casa] {FFFFFF}Você entrou na casa dos Johnson. Use /tc para trancar.");
    }
    else if(houseid == sweet_house)
    {
        GameTextForPlayer(playerid, "~w~Bem-vindo a ~b~Casa do Sweet!", 3000, 1);
        SendClientMessage(playerid, -1, 
            "{00FF00}[Casa] {FFFFFF}Esta é a casa do Sweet. Cuidado com a polícia!");
    }
    else if(houseid == og_loc_house)
    {
        GameTextForPlayer(playerid, "~w~Bem-vindo a ~p~Casa do OG Loc!", 3000, 1);
        SendClientMessage(playerid, -1, 
            "{00FF00}[Casa] {FFFFFF}Casa do OG Loc. Um lugar simples mas acolhedor.");
    }
    
    return 1;
}

public OnPlayerExitHouse(playerid, houseid)
{
    GameTextForPlayer(playerid, "~w~Voce saiu da casa", 2000, 1);
    SendClientMessage(playerid, 0x00FF00FF, 
        "{00FF00}[Casa] {FFFFFF}Você saiu da casa. Volte sempre!");
    return 1;
}

// ============================================================================
// CALLBACK PERSONALIZADO - NEGAÃƒâ€¡ÃƒÆ’O DE ENTRADA
// ============================================================================

forward OnPlayerDeniedEnEx(playerid, enexid, reason);
public OnPlayerDeniedEnEx(playerid, enexid, reason)
{
    // Feedback visual para diferentes razÃƒÂµes de negaÃƒÂ§ÃƒÂ£o
    switch(reason)
    {
        case REASON_CLOSED:
        {
            GameTextForPlayer(playerid, "~r~Casa trancada!", 2000, 1);
            SendClientMessage(playerid, 0xFF0000FF, 
                "{FF0000}[Casa] {FFFFFF}Esta casa estÃƒÂ¡ trancada. Apenas o dono pode entrar.");
        }
        case REASON_LEVEL:
        {
            GameTextForPlayer(playerid, "~r~Nivel insuficiente!", 2000, 1);
            SendClientMessage(playerid, 0xFF0000FF, 
                "{FF0000}[Casa] {FFFFFF}VocÃƒÂª precisa de um nÃƒÂ­vel maior para entrar aqui.");
        }
        case REASON_MONEY:
        {
            GameTextForPlayer(playerid, "~r~Dinheiro insuficiente!", 2000, 1);
            SendClientMessage(playerid, 0xFF0000FF, 
                "{FF0000}[Casa] {FFFFFF}VocÃƒÂª nÃƒÂ£o tem dinheiro suficiente para entrar.");
        }
    }
    
    return 1;
}
