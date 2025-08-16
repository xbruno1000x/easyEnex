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
- Enex_EnableFreeze(id, bool:status) — habilita/desabilita freeze ao teleportar
- Enex_Disable(id, bool:status) — desabilita/ativa EnEx para teleporte
- SetPlayerPosEnEx(playerid, x, y, z, angle, vwid = 0, intid = 0) — teleport manual

3️⃣ Callbacks disponíveis

- OnPlayerEnterEnEx(playerid, enexid) — chamado quando player entra ou sai

Exemplo:

```pawn
public OnPlayerEnteredEnEx(playerid, enexid)
{
    SendClientMessage(playerid, -1, "Você entrou no EnEx com sucesso!");
    return 1;
}
```

4️⃣ Mensagem de entrada fechada

A mensagem exibida ao tentar entrar em EnEx fechado pode ser configurada:

```pawn
#if !defined MSG_ENTRADA_FECHADA
    #define MSG_ENTRADA_FECHADA "Essa entrada está fechada no momento!"
#endif
```

5️⃣ Testando a biblioteca

Para testar, execute no terminal do seu projeto:

sampctl package run