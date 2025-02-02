# Tarefa: Controle de LEDs e botões com interrupções

Este projeto demonstra o uso de interrupções (IRQs) no microcontrolador RP2040 para controlar LEDs e botões na placa BitDogLab. Ele implementa:

-Interrupções para os botões com tratamento de debounce via software.
-Controle de uma matriz 5x5 WS2812 para exibir números de 0 a 9.
-Piscar contínuo do LED vermelho (GPIO 13) a 5Hz.

##Componentes utilizados:

-Matriz 5x5 WS2812 (GPIO 7)
-LED RGB (GPIOs 11, 12, 13)
-Botão A (GPIO 5) → Incrementa o número na matriz
-Botão B (GPIO 6) → Decrementa o número na matriz

Vídeo com a demonstraçã é:
