#include <a_samp>

#define MSG_ENTRADA_FECHADA "Altere aqui a mensagem de entrada fechada padrão!"
#include <easyEnex>
#include <zcmd>

new cjhouse, armario_cj;
new bool:trancada;

main() {}

public OnGameModeInit()
{
	DisableInteriorEnterExits();
	cjhouse = Enex_Create(
        "A Casa Dos Johnson",            // Nome do EnEx
        2495.3, -1690.3, 14.8, 0.0,   // Entrada (X, Y, Z, Ângulo)
        2496.05, -1695.17, 1014.74, 180.0,   // Saída   (X, Y, Z, Ângulo)
        0, 3, true                  // VirtualWorld, Interior, Freeze
    );

	armario_cj = Enex_Create(
		"Armario do CJ",                // Nome do EnEx
		2492.35, -1708.39, 1018.33, 0.0,   // Entrada (X, Y, Z, Ângulo)
		256.90, -41.65, 1002.02, 180.0,   // Saída   (X, Y, Z, Ângulo)
		0, 14, true, 0, 3                  // VirtualWorld, Interior, Freeze, virtualentrada, interiorentrada
	);
}

public OnPlayerSpawn(playerid){
	SetPlayerPosEnEx(playerid, 2495.26, -1684.67, 13.51, 179.31);
}

forward OnPlayerEnterEnEx(playerid, enexid);
public OnPlayerEnterEnEx(playerid, enexid)
{
	if(enexid == cjhouse)
		return GameTextForPlayer(playerid, "~w~Voce entrou na casa do CJ!", 500, 1);
	
	if(enexid == armario_cj)
		return GameTextForPlayer(playerid, "~w~Voce entrou no armario do CJ!", 500, 1);

    return 1;
}

forward OnPlayerExitEnEx(playerid, enexid);
public OnPlayerExitEnEx(playerid, enexid)
{
	if(enexid == cjhouse)
		return GameTextForPlayer(playerid, "~w~Voce saiu da casa do CJ!", 500, 1);
	
	if(enexid == armario_cj)
		return GameTextForPlayer(playerid, "~w~Voce saiu do armario!", 500, 1);
		
    return 1;
}

CMD:trancar(playerid){
	switch(trancada){
		case true:{
			trancada = false;
			SendClientMessage(playerid, -1, "A casa do CJ foi destrancada.");
			Enex_Open(cjhouse);
		}
		case false:
		{
			trancada = true;
			SendClientMessage(playerid, -1, "A casa do CJ foi trancada.");
			Enex_Close(cjhouse);
		}
	}
	return 1;
}

CMD:destrancar(playerid){
	return cmd_trancar(playerid);
}

CMD:pos(playerid, params[])
{
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    printf("%.2f, %.2f, %.2f, %.2f", x, y, z, a);

    SendClientMessage(playerid, -1, "Suas coordenadas foram enviadas para o console do servidor.");
    return 1;
}