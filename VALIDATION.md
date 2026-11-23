# Validação desta revisão

- Data: 23/11/2026.
- Compilador: Arduino CLI 1.5.1, Arduino AVR Boards 1.8.6, Arduboy2 6.0.0.
- Alvo: arduino:avr:leonardo (ATmega32U4).
- Emulador: Ardens 0.25.12 para Windows.
- `tools/Test.ps1`: RESULT 51 checks, 0 failures, executados no AVR emulado.
- Firmware normal: 21868/28672 bytes de flash e 1550/2560 bytes de RAM estática.
- Firmware demo: 22226/28672 bytes de flash e 1551/2560 bytes de RAM estática.
- Conferência visual: tela inicial, seleção de cenário e sprite adulto do T-Rex.
- `git diff --check`: sem erros de whitespace.

As verificações cobrem dieta, três evoluções, overflow dos votos, higiene,
risco e persistência de doença, alertas, efeitos das ações, restrições de menus,
recompensa do minijogo, EEPROM reservada, save/load, corrupção, migração v1,
sono, despertar, evolução à meia-noite e cenários da demo.

Não foi feito teste em hardware físico nem uma sessão de balanceamento de vários dias.
