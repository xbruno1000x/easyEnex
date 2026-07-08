/*
 * easyEnex.inc - Biblioteca de entradas/saídas (Enex) para SA-MP
 *
 * Descrição:
 *   Biblioteca leve para gerenciar entradas/saídas (enex) no servidor SA-MP.
 *   Recursos principais:
 *     - Criação e remoção de enexs com pickups dinâmicos
 *     - Labels 3D dinâmicos para entradas
 *     - Controle de permissões por jogador (abrir/fechar/enex por jogador)
 *     - Agendamento de funcionamento, taxa de entrada e requisito de nível
 *     - Integração com YSI (iterators/hooks/timers) e Streamer
 *
 * Uso:
 *   - Inclua esta biblioteca antes de módulos que dependem dela:
 *       #include <easyEnex>
 *   - Para customizar o sistema monetário do seu gamemode, defina as macros
 *     Enex_GetPlayerMoney/Enex_GivePlayerMoney antes de incluir a biblioteca.
 *
 * Dependências:
 *   - YSI (y_iterate, y_hooks, y_timers)
 *   - Streamer (para CreateDynamicPickup/CreateDynamic3DTextLabel)
 *
 * Autor: xBruno1000x
 * Versão: 1.8.0
 * Licença: MIT (veja LICENSE)
 *
 * Observações:
 *   - Projetada para ser compatível com módulos opcionais (ex.: easyHouses.inc).
 *   - Alterações nesta biblioteca podem afetar módulos que gerenciam labels/pickups.
 */

#include <YSI_Data\y_iterate>
#include <streamer>
#include <YSI_Coding\y_hooks>
#include <YSI_Coding\y_timers>

#if !defined MAX_ENEX
    #define MAX_ENEX                (200)
#endif

#define TEXTLABEL_STREAMDISTANCE    (15.0)
#define ENEX_YELLOW   				(0xFFFF00FF)
#if !defined MSG_ENTRADA_FECHADA
    #define MSG_ENTRADA_FECHADA "Essa entrada esta fechada no momento!"
#endif
#if !defined DEFAULT_PICKUPID
    #define DEFAULT_PICKUPID 1318 // ID default do pickup do enex
#endif
#if !defined DEFAULT_KEY
    #define DEFAULT_KEY KEY_SECONDARY_ATTACK // Tecla padrão para ativar a entrada/saída
#endif

#if !defined ENEX_SEND_CLOSED_MESSAGE
    #define ENEX_SEND_CLOSED_MESSAGE 1
#endif

// ============================================================================
// FUNÇÕES DE DINHEIRO PERSONALIZÁVEIS
// ============================================================================

// Define estas funções no seu gamemode ANTES de incluir easyEnex para customizar
// Exemplo:
// #define Enex_GetPlayerMoney(%0)    MinhaFuncaoGetMoney(%0)
// #define Enex_GivePlayerMoney(%0,%1) MinhaFuncaoDarDinheiro(%0,%1)
// #include <easyEnex>

#if !defined Enex_GetPlayerMoney
    #define Enex_GetPlayerMoney(%0) GetPlayerMoney(%0)
#endif

#if !defined Enex_GivePlayerMoney
    #define Enex_GivePlayerMoney(%0,%1) GivePlayerMoney(%0,%1)
#endif

// ============================================================================
// CONSTANTES
// ============================================================================

#define REASON_CLOSED 0
#define REASON_LEVEL  1
#define REASON_MONEY  2

static 
    // Strings
    enexName[MAX_ENEX][32],
    enexClosedMsg[MAX_ENEX][64],
    Text3D:entranceLabel[MAX_ENEX],
    entranceMapIcon[MAX_ENEX],

    // Entrada
    entrancePickup[MAX_ENEX],
    Float: enX[MAX_ENEX],
    Float: enY[MAX_ENEX],
    Float: enZ[MAX_ENEX],
    Float: enAng[MAX_ENEX],
    enVirtualID[MAX_ENEX],
    enInteriorID[MAX_ENEX],

    // Saida
    exitPickup[MAX_ENEX],
    Float: exX[MAX_ENEX],
    Float: exY[MAX_ENEX],
    Float: exZ[MAX_ENEX],
    Float: exAng[MAX_ENEX],
    exVirtualID[MAX_ENEX],
    exInteriorID[MAX_ENEX],

    // Agendamento
    enexStartHour[MAX_ENEX],
    enexEndHour[MAX_ENEX],
    bool:enexHasSchedule[MAX_ENEX],

    // Outros
    Iterator: enexID<MAX_ENEX>,
    bool:enexFreezePlayer[MAX_ENEX],
    bool:enexDisabled[MAX_ENEX],
    bool:enexOpenForPlayer[MAX_ENEX][MAX_PLAYERS],
    enexMinLevel[MAX_ENEX],   // nível mínimo para entrar
    enexEntryFee[MAX_ENEX],   // taxa de entrada em dinheiro
    bool:enexSendClosedMessage = true, // enviar mensagem ao tentar entrar em enex fechado
    PlayerLastEnex[MAX_PLAYERS] = {-1, ...}; // Armazena o ID do último enex usado pelo jogador

Float:GetXYInFrontOfEnEx(Float:x, Float:y, Float:a, &Float:q, &Float:w, Float:distance)
{
    q = x;
    w = y;
    q += (distance * floatsin(-a, degrees));
    w += (distance * floatcos(-a, degrees));
    return a;
}

stock Enex_Create(const name[], 
                Float:entX, Float:entY, Float:entZ, Float:entAng, 
                Float:exiX, Float:exiY, Float:exiZ, Float:exiAng,
                ex_vwid = 0, ex_intid = 0, bool:freeze = false, en_vwid = 0, en_intid = 0,
                pickupid = DEFAULT_PICKUPID, const closedMsg[] = MSG_ENTRADA_FECHADA,
                minLevel = 0, entryFee = 0)
{
    new id = Iter_Free(enexID);
    if(id == -1) {
        print("[easyEnEx] ERRO: Limite de entradas/saidas atingido!");
        return -1;
    }
    
    Iter_Add(enexID, id);
    
    enX[id] = entX; enY[id] = entY; enZ[id] = entZ; enAng[id] = entAng;
    exX[id] = exiX; exY[id] = exiY; exZ[id] = exiZ; exAng[id] = exiAng;
    exVirtualID[id] = ex_vwid;
    exInteriorID[id] = ex_intid;
    enVirtualID[id] = en_vwid;
    enInteriorID[id] = en_intid;
    enexFreezePlayer[id] = freeze;
    enexMinLevel[id] = minLevel;
    enexEntryFee[id] = entryFee;

    format(enexName[id], sizeof(enexName), "%s", name);
    format(enexClosedMsg[id], sizeof(enexClosedMsg), "%s", closedMsg);
    
    entrancePickup[id] = CreateDynamicPickup(pickupid, 1, entX, entY, entZ, en_vwid, en_intid);
    exitPickup[id] = CreateDynamicPickup(pickupid, 1, exiX, exiY, exiZ, ex_vwid, ex_intid);
    
    entranceLabel[id] = CreateDynamic3DTextLabel(
        name, 
        ENEX_YELLOW, 
        entX, entY, entZ, 
        15.0, 
        .testlos = 1, 
        .streamdistance = TEXTLABEL_STREAMDISTANCE
    );

    CreateDynamic3DTextLabel(
        "Saída", 
        ENEX_YELLOW, 
        exiX, exiY, exiZ, 
        15.0, 
        .testlos = 1, 
        .streamdistance = TEXTLABEL_STREAMDISTANCE
    );

    UpdateEnexLabel(id);
    return id;
}

stock Enex_Open(id){
    if(!enexDisabled[id])
        return 0;

    enexDisabled[id] = false;
    UpdateEnexLabel(id);
    return 1;
}

stock Enex_OpenForPlayer(playerid, enexid)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;

    enexOpenForPlayer[enexid][playerid] = true;
    return 1;
}

stock Enex_Close(id){
    if(enexDisabled[id])
        return 0;

    enexDisabled[id] = true;
    UpdateEnexLabel(id);
    return 1;
}

stock Enex_CloseForPlayer(playerid, enexid)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;

    enexOpenForPlayer[enexid][playerid] = false;
    return 1;
}

stock Enex_DestroyEntranceLabel(enexid)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;
    
    if(entranceLabel[enexid] != Text3D:INVALID_3DTEXT_ID)
    {
        DestroyDynamic3DTextLabel(entranceLabel[enexid]);
        entranceLabel[enexid] = Text3D:INVALID_3DTEXT_ID;
    }
    return 1;
}

stock bool:Enex_IsClosed(id){
    return enexDisabled[id];
}

stock Enex_SetSchedule(enexid, startHour, endHour)
{
    if(enexid < 0 || enexid >= MAX_ENEX)
        return 0;

    enexStartHour[enexid] = startHour;
    enexEndHour[enexid] = endHour;
    enexHasSchedule[enexid] = true;
    UpdateEnexLabel(enexid);
    return 1;
}

stock Enex_SetSendClosedMessage(bool:flag)
{
    enexSendClosedMessage = flag;
    return 1;
}

stock Enex_SetIcon(enexid, iconid, color = -1, MAPICON:style = MAPICON_LOCAL, Float:streamdistance = STREAMER_MAP_ICON_SD)
{
    if(!Iter_Contains(enexID, enexid))
        return 0;

    if(entranceMapIcon[enexid])
        DestroyDynamicMapIcon(entranceMapIcon[enexid]);

    entranceMapIcon[enexid] = CreateDynamicMapIcon(
        enX[enexid],
        enY[enexid],
        enZ[enexid],
        iconid,
        color,
        enVirtualID[enexid],
        enInteriorID[enexid],
        -1,
        streamdistance,
        style
    );
    return 1;
}

stock Enex_CheckSchedule(enexid)
{
    new hour, minute, second;
    gettime(hour, minute, second);

    if(!enexHasSchedule[enexid])
        return 1;

    new start = enexStartHour[enexid];
    new end   = enexEndHour[enexid];

    if(start < end)
    {
        if(hour >= start && hour < end)
            return 1;
    }
    else if(start > end)
    {
        if(hour >= start || hour < end)
            return 1;
    }
    return 0;
}

stock Enex_SetMinLevel(enexid, level)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;
    enexMinLevel[enexid] = level;
    UpdateEnexLabel(enexid);
    return 1;
}

stock Enex_SetEntryFee(enexid, fee)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;
    enexEntryFee[enexid] = fee;
    UpdateEnexLabel(enexid);
    return 1;
}

stock UpdateEnexLabel(enexid)
{
    if(enexid < 0 || enexid >= MAX_ENEX) return 0;

    new msg[256];
    new statusText[32];
    new scheduleText[32];

    if(enexDisabled[enexid])
        format(statusText, sizeof(statusText), "Fechado");
    else
        format(statusText, sizeof(statusText), "Aberto");

    if(enexHasSchedule[enexid])
    {
        new start = enexStartHour[enexid];
        new end   = enexEndHour[enexid];
        if(start < end)
            format(scheduleText, sizeof(scheduleText), "\nHorario: %02d:00 - %02d:00", start, end);
        else
            format(scheduleText, sizeof(scheduleText), "\nHorario: %02d:00 - %02d:00", start, end + 24);
    }

    if(enexMinLevel[enexid] > 0 && enexEntryFee[enexid] > 0)
        format(msg, sizeof(msg), "%s \nStatus: %s \nNivel minimo: %d \nTaxa: $%d%s",
               enexName[enexid], statusText, enexMinLevel[enexid], enexEntryFee[enexid], scheduleText);
    else if(enexMinLevel[enexid] > 0)
        format(msg, sizeof(msg), "%s \nStatus: %s \nNivel minimo: %d%s",
               enexName[enexid], statusText, enexMinLevel[enexid], scheduleText);
    else if(enexEntryFee[enexid] > 0)
        format(msg, sizeof(msg), "%s \nStatus: %s \nTaxa: $%d%s",
               enexName[enexid], statusText, enexEntryFee[enexid], scheduleText);
    else
        format(msg, sizeof(msg), "%s \nStatus: %s%s",
               enexName[enexid], statusText, scheduleText);

    UpdateDynamic3DTextLabelText(entranceLabel[enexid], ENEX_YELLOW, msg);
    return 1;
}

hook OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys){
    foreach(new id : enexID){
        if(newkeys & DEFAULT_KEY){
            if(IsPlayerInRangeOfPoint(playerid, 1.0, enX[id], enY[id], enZ[id]))
            {
                #if defined OnPlayerAttemptEnEx
                    if(CallLocalFunction("OnPlayerAttemptEnEx", "dd", playerid, id) == 0)
                    {
                        return 0;
                    }
                #endif
                if (enexDisabled[id] && !enexOpenForPlayer[id][playerid])
                {
                    #if defined OnPlayerDeniedEnEx
                        if (!OnPlayerDeniedEnEx(playerid, id, REASON_CLOSED)) return 0;
                    #endif
                    #if ENEX_SEND_CLOSED_MESSAGE
                        if (enexSendClosedMessage) return SendClosedMessage(playerid, id);
                        return 1;
                    #else
                        return 1;
                    #endif
                }

                if(enexHasSchedule[id] && !Enex_CheckSchedule(id)) {
                    #if defined OnPlayerDeniedEnEx
                        OnPlayerDeniedEnEx(playerid, id, REASON_CLOSED);
                    #endif
                    return SendScheduleMessage(playerid, id);
                }

                if(GetPlayerScore(playerid) < enexMinLevel[id])
                {
                    #if defined OnPlayerDeniedEnEx
                        OnPlayerDeniedEnEx(playerid, id, REASON_LEVEL);
                    #endif
                    return 1;
                }

                if(enexEntryFee[id] > 0)
                {
                    if(Enex_GetPlayerMoney(playerid) < enexEntryFee[id])
                    {
                        #if defined OnPlayerDeniedEnEx
                            OnPlayerDeniedEnEx(playerid, id, REASON_MONEY);
                        #endif
                        return 1;
                    }
                    Enex_GivePlayerMoney(playerid, -enexEntryFee[id]); 
                }

                #if defined OnPlayerEnterEnEx
                    if(!OnPlayerEnterEnEx(playerid, id)) return 1;
                #endif

                if(enexFreezePlayer[id]) {
                    TogglePlayerControllable(playerid, false);
                    defer UnfreezePlayer(playerid);
                }
                // Indo da entrada -> saída
                new Float:newX, Float:newY;
                GetXYInFrontOfEnEx(exX[id], exY[id], exAng[id], newX, newY, 0.5);
                SetPlayerPosEnEx(playerid, newX, newY, exZ[id], exAng[id], exVirtualID[id], exInteriorID[id]);
                PlayerLastEnex[playerid] = id;

                return 1;
            }
            
            // Verifica saída usando o enex armazenado (evita conflitos de VW)
            if(PlayerLastEnex[playerid] != -1)
            {
                new lastid = PlayerLastEnex[playerid];
                
                // Verifica se o jogador está na posição de saída do enex que ele entrou
                if(IsPlayerInRangeOfPoint(playerid, 1.0, exX[lastid], exY[lastid], exZ[lastid]))
                {
                    // Verifica se está no mundo virtual correto
                    if(GetPlayerVirtualWorld(playerid) == exVirtualID[lastid])
                    {
                        #if defined OnPlayerExitEnEx
                            if(!OnPlayerExitEnEx(playerid, lastid)) return 1;
                        #endif

                        if(enexFreezePlayer[lastid]) {
                            TogglePlayerControllable(playerid, false);
                            defer UnfreezePlayer(playerid);
                        }
                        // Indo da saída -> entrada
                        new Float:newX, Float:newY;
                        GetXYInFrontOfEnEx(enX[lastid], enY[lastid], enAng[lastid], newX, newY, 0.5);
                        SetPlayerPosEnEx(playerid, newX, newY, enZ[lastid], enAng[lastid], enVirtualID[lastid], enInteriorID[lastid]);
                        
                        // Limpa o registro após a saída
                        PlayerLastEnex[playerid] = -1;
                        
                        return 1;
                    }
                }
            }
            
            // Fallback: busca por coordenadas (para compatibilidade com uso direto de SetPlayerPos)
            if(IsPlayerInRangeOfPoint(playerid, 1.0, exX[id], exY[id], exZ[id]))
            {
                // Verifica se está no mundo virtual correto
                if(GetPlayerVirtualWorld(playerid) == exVirtualID[id])
                {
                    #if defined OnPlayerExitEnEx
                        if(!OnPlayerExitEnEx(playerid, id)) return 1;
                    #endif

                    if(enexFreezePlayer[id]) {
                        TogglePlayerControllable(playerid, false);
                        defer UnfreezePlayer(playerid);
                    }
                    // Indo da saída -> entrada
                    new Float:newX, Float:newY;
                    GetXYInFrontOfEnEx(enX[id], enY[id], enAng[id], newX, newY, 0.5);
                    SetPlayerPosEnEx(playerid, newX, newY, enZ[id], enAng[id], enVirtualID[id], enInteriorID[id]);
                    
                    // Limpa o registro
                    PlayerLastEnex[playerid] = -1;
                    
                    return 1;
                }
            }
        }
    }
    return 1;
}

timer UnfreezePlayer[1000](playerid) {
    TogglePlayerControllable(playerid, true);
}

hook OnPlayerDisconnect(playerid, reason) {
    PlayerLastEnex[playerid] = -1;
    return 1;
}

stock SendClosedMessage(playerid, enexid){
    return SendClientMessage(playerid, 0xFF0000FF, enexClosedMsg[enexid]);
}

stock SendScheduleMessage(playerid, enexid)
{
    new msg[96];
    new start = enexStartHour[enexid];
    new end   = enexEndHour[enexid];

    if (start < end)
    {
        format(msg, sizeof(msg), "Entrada fechada! Horário de funcionamento: %02d:00 - %02d:00", start, end);
    } else {
        format(msg, sizeof(msg), "Entrada fechada! Horário de funcionamento: %02d:00 - %02d:00", start, end + 24);
    }
    return SendClientMessage(playerid, 0xFF0000FF, msg);
}

stock SetPlayerPosEnEx(playerid, Float:x, Float:y, Float:z, Float:angle, vwid = 0, intid = 0)
{
	SetPlayerPos(playerid, x, y, z);
	SetPlayerFacingAngle(playerid, angle);
    SetPlayerInterior(playerid, intid);
    SetPlayerVirtualWorld(playerid, vwid);
    SetCameraBehindPlayer(playerid);
	return 1;
}

/**
 * Enex_UpdateEntrancePickup
 *
 * Atualiza o modelo do pickup de entrada de um enex
 *
 * @param enexid      ID do enex
 * @param pickupModel Novo modelo do pickup
 * @return 1 em sucesso, 0 em falha
 */
stock Enex_UpdateEntrancePickup(enexid, pickupModel)
{
    if(!Iter_Contains(enexID, enexid))
        return 0;

    DestroyDynamicPickup(entrancePickup[enexid]);
    entrancePickup[enexid] = CreateDynamicPickup(pickupModel, 1, enX[enexid], enY[enexid], enZ[enexid], enVirtualID[enexid], enInteriorID[enexid]);
    return 1;
}
