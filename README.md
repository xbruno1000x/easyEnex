# 🟦 easyEnEx

Projeto desenvolvido por:  
- **xBruno1000x**

---

### 📦 Como colocar a biblioteca em funcionamento

### 1️⃣ Instalação via sampctl

Instale no seu projeto:

```bash
sampctl install xbruno1000x/easyEnEx
```
(Ou apenas importe o arquivo .inc para sua pasta pawno->includes!)

Inclua no seu código:

```pawn
#include <easyEnEx>
```

Certifique-se de que os includes e plugins necessários estão disponíveis:

- YSI Includes (y_hooks, y_iterate, y_timers)
- Streamer Plugin

2️⃣ Criando entradas e saídas (EnEx)

Exemplo básico de criação de um EnEx:

```pawn
public OnGameModeInit()
{
    new id = Enex_Create(
        "Minha Casa",            // Nome do EnEx
        1234.0, -567.0, 20.0, 0.0,   // Entrada (X, Y, Z, Ângulo)
        200.0, 300.0, 10.0, 180.0,   // Saída   (X, Y, Z, Ângulo)
        0, 0, true                  // VirtualWorld, Interior, Freeze
    );
}
```

Funções principais:

- Enex_Create(name[], entX, entY, entZ, entAng, exX, exY, exZ, exAng, vwid = 0, intid = 0, freeze = false)
- Enex_Open(id) — abre uma entrada fechada
- Enex_Close(id) — fecha uma entrada
- Enex_IsClosed(id) - verifica se uma entrada está fechada
- SetPlayerPosEnEx(playerid, x, y, z, angle, vwid = 0, intid = 0) — teleport manual

3️⃣ Callbacks disponíveis

- OnPlayerEnterEnEx(playerid, enexid) — chamado quando player entra no enex.
- OnPlayerExitEnEx(playerid, enexid) — chamado quando player sai de um enex.
- OnPlayerDeniedEnEx(playerid, enexid) - chamado quando o player não pode entrar em um enex.

Exemplo:

```pawn
public OnPlayerEnterEnEx(playerid, enexid)
{
    SendClientMessage(playerid, -1, "Você entrou no EnEx com sucesso!");
    return 1;
}
```

4️⃣ Personalizações:

A mensagem exibida ao tentar entrar em EnEx fechado pode ser configurada.

padrão:
```pawn
#if !defined MSG_ENTRADA_FECHADA
    #define MSG_ENTRADA_FECHADA "Essa entrada está fechada no momento!"
#endif
```

mas você pode alterá-la fazendo como no exemplo abaixo:
```pawn
#define MSG_ENTRADA_FECHADA 	"Altere aqui a mensagem de entrada fechada padrão!" // Você pode alterar a mensagem de entrada fechada padrão aqui
#define DEFAULT_PICKUPID 		1273 												// ID do pickup de saída, você pode alterar para outro ID de pickup se desejar
#define DEFAULT_KEY 			KEY_SPRINT											// Tecla padrão para abrir o EnEx, por padrão é a tecla F. Nesse exemplo alteramos para espaço.
#include <easyEnex>
```
Esse procedimento se aplica ao pickup e key padrões também.

5️⃣ Testando a biblioteca

Para testar, execute no terminal do seu projeto:
```bash
sampctl build
```

Após compilar utilize o comando abaixo para iniciar o servidor
```bash
sampctl run
```