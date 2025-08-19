#include <a_samp>

// easyEnex.pwn - Exemplo de personalizações que você pode fazer no EasyEnex
#define MSG_ENTRADA_FECHADA 	"Altere aqui a mensagem de entrada fechada padrão!" // Você pode alterar a mensagem de entrada fechada padrão aqui
#define DEFAULT_PICKUPID 		1273 												// ID do pickup de saída, você pode alterar para outro ID de pickup se desejar
//#define DEFAULT_KEY 			KEY_SPRINT											// Tecla padrão para abrir o EnEx, por padrão é a tecla F. Nesse exemplo alteramos para espaço.
#include <easyEnex>
#include <zcmd>

new cjhouse, armario_cj, sweet_house;
new bool:permissao;

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
		"Armario do CJ", 						// Nome do EnEx
		2492.35, -1708.39, 1018.33, 0.0,    	// Entrada (X, Y, Z, Ângulo)
		256.90, -41.65, 1002.02, 180.0,     	// Saída   (X, Y, Z, Ângulo)
		0, 14, true, 0, 3, .pickupid = 19197	// VirtualWorld, Interior, Freeze, virtualentrada, interiorentrada, pickupid
	);

	sweet_house = Enex_Create(
		"Casa do Sweet", 						// Nome do EnEx
		2523.03, -1679.25, 15.49, 87.19,    	// Entrada (X, Y, Z, Ângulo)
		256.90, -41.65, 1002.02, 180.0,     	// Saída   (X, Y, Z, Ângulo)
		0, 1, true, .pickupid = 19606,			// VirtualWorld, Interior, Freeze, virtualentrada, interiorentrada, pickupid
		.closedMsg = "O sweet nao esta em casa!"// Mensagem personalizada quando a casa estiver fechada
	);
	Enex_Close(sweet_house);
}

public OnPlayerSpawn(playerid){
	SetPlayerPosEnEx(playerid, 2495.26, -1684.67, 13.51, 179.31);
}

forward OnPlayerDeniedEnEx(playerid, enexid);
public OnPlayerDeniedEnEx(playerid, enexid){
	if(enexid == sweet_house){
		SetPlayerWantedLevel(playerid, 6);
		return GameTextForPlayer(playerid, "~w~A policia te flagrou invadindo a casa do Sweet!", 500, 1);
	}
	return 1;
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

forward OnPlayerAttemptEnEx(playerid, enexid);
public OnPlayerAttemptEnEx(playerid, enexid){
	if(enexid == cjhouse && !permissao)
	{
		SendClientMessage(playerid, -1, "Voce nao tem permissao para entrar na casa do CJ! Use /permissao para obter a permissao.");
		return 0; // Bloqueia o uso do EnEx
	}
	return 1;
}

CMD:permissao(playerid){
	if(permissao)
	{
		permissao = false;
		SendClientMessage(playerid, -1, "Permissao para entrar na casa do CJ foi removida.");
	}
	else
	{
		permissao = true;
		SendClientMessage(playerid, -1, "Permissao para entrar na casa do CJ foi concedida.");
	}
	return 1;
}

CMD:trancar(playerid){
	switch(Enex_IsClosed(cjhouse)){
		case true:{
			SendClientMessage(playerid, -1, "A casa do CJ foi destrancada.");
			Enex_Open(cjhouse);
		}
		case false:
		{
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