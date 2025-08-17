#include <YSI_Data\y_iterate>
#include <streamer>
#include <YSI_Coding\y_hooks>
#include <YSI_Coding\y_timers>

#if !defined MAX_ENEX
    #define MAX_ENEX                (50)
#endif

#define TEXTLABEL_STREAMDISTANCE    (15.0)
#define COLOR_YELLOW   				(0xFFFF00FF)
#if !defined MSG_ENTRADA_FECHADA
    #define MSG_ENTRADA_FECHADA "Essa entrada está fechada no momento!"
#endif

static 
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

    // Outros
    Iterator: enexID<MAX_ENEX>,
    bool:enexFreezePlayer[MAX_ENEX],
    bool:enexDisabled[MAX_ENEX];

Float:GetXYInFrontOfEnex(Float:x, Float:y, Float:a, &Float:q, &Float:w, Float:distance)
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
                ex_vwid = 0, ex_intid = 0, bool:freeze = false, en_vwid = 0, en_intid = 0)
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
    
    entrancePickup[id] = CreateDynamicPickup(1318, 1, entX, entY, entZ, en_vwid, en_intid);
    exitPickup[id] = CreateDynamicPickup(1318, 1, exiX, exiY, exiZ, ex_vwid, ex_intid);
    
    CreateDynamic3DTextLabel(
        name, 
        COLOR_YELLOW, 
        entX, entY, entZ, 
        15.0, 
        .testlos = 1, 
        .streamdistance = TEXTLABEL_STREAMDISTANCE
    );

    CreateDynamic3DTextLabel(
        "Saída", 
        COLOR_YELLOW, 
        exiX, exiY, exiZ, 
        15.0, 
        .testlos = 1, 
        .streamdistance = TEXTLABEL_STREAMDISTANCE
    );
    return id;
}

stock Enex_Open(id){
    if(!enexDisabled[id])
        return 0;

    enexDisabled[id] = false;
    return 1;
}

stock Enex_Close(id){
    if(enexDisabled[id])
        return 0;

    enexDisabled[id] = true;
    return 1;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys){
    foreach(new id : enexID){
        if(newkeys & KEY_SECONDARY_ATTACK){
            if(IsPlayerInRangeOfPoint(playerid, 1.0, enX[id], enY[id], enZ[id]))
            {
                if(enexDisabled[id])
                    return SendClosedMessage(playerid);

                #if defined OnPlayerEnterEnEx
                    if(!OnPlayerEnterEnEx(playerid, id)) return 1;
                #endif

                if(enexFreezePlayer[id]) {
                    TogglePlayerControllable(playerid, false);
                    defer UnfreezePlayer(playerid);
                }
                // Indo da entrada -> saída
                new Float:newX, Float:newY;
                GetXYInFrontOfEnex(exX[id], exY[id], exAng[id], newX, newY, 0.5);
                SetPlayerPosEnEx(playerid, newX, newY, exZ[id], exAng[id], exVirtualID[id], exInteriorID[id]);

                return 1;
            }
            if(IsPlayerInRangeOfPoint(playerid, 1.0, exX[id], exY[id], exZ[id]))
            {
                if(enexDisabled[id])
                    return 1;

                #if defined OnPlayerExitEnEx
                    if(!OnPlayerExitEnEx(playerid, id)) return 1;
                #endif

                if(enexFreezePlayer[id]) {
                    TogglePlayerControllable(playerid, false);
                    defer UnfreezePlayer(playerid);
                }
                // Indo da saída -> entrada
                new Float:newX, Float:newY;
                GetXYInFrontOfEnex(enX[id], enY[id], enAng[id], newX, newY, 0.5);
                SetPlayerPosEnEx(playerid, newX, newY, enZ[id], enAng[id], enVirtualID[id], enInteriorID[id]);
                return 1;
            }
        }
    }
    return 1;
}

timer UnfreezePlayer[1000](playerid) {
    TogglePlayerControllable(playerid, true);
}

stock SendClosedMessage(playerid){
    return SendClientMessage(playerid, 0xFF0000FF, MSG_ENTRADA_FECHADA);
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