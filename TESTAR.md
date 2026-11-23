# Testar no emulador

Requer Arduino CLI, o pacote `arduino:avr`, a biblioteca `Arduboy2` e o
[Ardens oficial](https://github.com/tiberiusbrown/Ardens/releases).
Nesta instalação, os executáveis do Ardens já estão em `.tools/`.

No PowerShell, dentro desta pasta:

```powershell
.\tools\Build.ps1 -Mode demo
.\tools\Play.ps1 -Mode demo
```

O firmware fica em `build/demo/ardudino.ino.hex` e também pode ser aberto
manualmente no Ardens. Em outro computador, informe o emulador com
`-Emulator C:\caminho\ArdensPlayer.exe` no comando `Play.ps1`.

## Controles

- Setas: esquerda/direita alternam as colunas; cima/baixo navegam os ícones e opções.
- A ou Z: botão A do Arduboy, para iniciar/voltar. Na tela do bichinho, salva imediatamente.
- S, X ou B: botão B do Arduboy, para confirmar.
- F8: reinicia o emulador. Clique na janela para dar foco ao teclado.

Na tela inicial da demo, B alterna os cenários e A inicia o escolhido:
continuar, bichinho novo, T-Rex, Bronto, Tricera, cuidados pendentes e hora de dormir.
Iniciar um cenário substitui o estado da demo. A versão normal usa outro arquivo de save.

Na demo, **1 segundo real = 1 minuto do jogo**. Na versão normal, os minutos
passam em tempo real. O relógio avança apenas com o jogo rodando, depois da
tela inicial. Não há RTC nem simulação do tempo desligado.

```powershell
.\tools\Build.ps1 -Mode normal
.\tools\Play.ps1 -Mode normal
.\tools\Test.ps1
```

Os testes precisam de `.tools/Ardens.exe`, ou de `-Emulator C:\caminho\Ardens.exe`.
São 51 verificações no ATmega32U4 emulado, com resultado em `build/test/emulator.log`.
O script retorna erro se algum teste falhar ou o relatório não for produzido.
O firmware de teste escreve apenas na EEPROM descartável do emulador.

A compilação prepara uma cópia em `build/sketch/ardudino` para manter o nome do sketch
compatível com o Arduino CLI mesmo quando a pasta principal se chama `Ardudino`.

## Roteiro de teste manual

1. Inicie o cenário New pet. Dê água e comida, confira os indicadores em Status e
   veja que a animação termina e devolve o controle ao jogador.
2. Use os cenários T-Rex, Bronto e Tricera para ver os sprites. Em Status/Growth,
   confira a espécie e a fase. No crescimento normal, a espécie aparece aos 2 dias
   e ganha extensão de cauda adulta aos 5 dias.
3. No cenário Needs care, veja o aviso de remédio e a sujeira. Use Heal para curar,
   Shower para limpar, Drink e Food para repor necessidades e Climate para reduzir
   a temperatura. O remédio não substitui esses cuidados.
4. No cenário Bedtime, tente alimentar: deve recusar. Abra Light e apague a luz.
   Status e Light continuam disponíveis; a luz volta automaticamente às 9h.
5. Em Play, jogue cinco rodadas. Y mostra suas vitórias, D as do dino. Ele precisa
   ganhar pelo menos três para receber felicidade. Empates não contam como vitória.
6. Na tela do bichinho, pressione A para salvar. Feche e abra pelo mesmo Play.ps1;
   escolha Continue. Espécie, sujeira, saúde e relógio devem ser restaurados.

Autosave ocorre a cada cinco minutos reais. Use A na tela principal antes de fechar
para guardar as ações mais recentes. O Ardens guarda a EEPROM na sua pasta de dados,
em arquivos `absim_<hash do firmware>.save`. Demo e normal têm hashes diferentes.
Recompilar um firmware com alterações pode criar outro perfil no emulador;
no hardware, a EEPROM permanece entre uploads. O teste físico no Arduboy ainda precisa ser feito.
