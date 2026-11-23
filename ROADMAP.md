# Melhorias do Ardudino

Prioridades baseadas no RakuRaku Dinokun, respeitando o display de 128x64,
os 28 KB de flash disponíveis ao sketch e os 2,5 KB de RAM do Arduboy.

## Implementado nesta revisão

- [x] Dieta na fase bebê determina T-Rex, Bronto ou Tricera aos 2 dias.
- [x] Fases bebê, jovem e adulto, com sprites por espécie e cauda adulta.
- [x] Sujeira acumulada, indicador visual e banho com efeito real.
- [x] Doença persistente por descuido, período de tolerância e remédio.
- [x] Avisos de água, comida, banho, clima, brincadeira e luz ao dormir.
- [x] Sono das 21h às 9h, sem esvaziar as necessidades a cada três segundos.
- [x] Bebês precisam comer com mais frequência.
- [x] Páginas de higiene, evolução e saúde no Status.
- [x] Persistência dos novos estados e migração do formato anterior.
- [x] Proteção da EEPROM do sistema, salvamento manual e autosave moderado.
- [x] Carinho aumenta felicidade; ações marcam o estado para salvar.
- [x] Minijogo exige três vitórias do dino e mostra placar dentro da tela.
- [x] Submenus bloqueados durante animações e correção de confirmação duplicada.
- [x] Captura de toques curtos entre quadros para melhorar a resposta dos controles.
- [x] Compilação reproduzível, testes no AVR emulado e demo com cenários.

## Próximas sugestões, em ordem

1. **Ciclo de vida completo:** morte por descuido prolongado, confirmação para
   novo ovo e histórico de gerações. Definir a tolerância antes de adicionar
   uma consequência permanente ao salvamento.
2. **Relógio ajustável e pausa:** configurar hora inicial, pausa explícita e ritmo.
   Tempo offline exigiria RTC no hardware ou suporte do host.
3. **Climatização automática:** ar-condicionado ligado/desligado com deriva
   previsível, em vez dos passos manuais de temperatura atuais.
4. **Personalidade visual:** olhos fechados para os dinossauros evoluídos ao dormir,
   recusa por alimento e animações próprias de doença e evolução.
5. **Educação com consequências:** estudo conforme humor, pequenas travessuras
   e disciplina afetando o comportamento.
6. **Balanceamento com sessões longas:** medir frequência de cuidados, idade,
   peso e duração das fases antes de adicionar eventos aleatórios e mais jogos.

## Referências e limites

- [Instruções históricas](https://www.virtualpet.com/vp/farm/raku/raku.htm):
  água, comida, banho, estudo, clima e três vitórias do dino em cinco rodadas.
- [Pesquisa por testes do aparelho](https://tasvideos.org/HomePages/mugg/DinkieDino):
  sono, necessidades e comportamento do Dinokun.
- [Guia de cuidados](https://gotchi-garden.blogspot.com/p/dinkie-dino-care-sheet.html):
  relação entre alimentação e evolução e luz durante o sono.
- [EEPROM do Arduboy2](https://mlxxxp.github.io/documents/Arduino/libraries/Arduboy2/Doxygen/html/Arduboy2_8h.html):
  os primeiros 16 bytes são reservados para o sistema.

Intervalos, empates de dieta, tolerância de doença e desenhos são decisões do
Ardudino. Esta adaptação não reproduz todas as particularidades do brinquedo original.
