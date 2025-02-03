# Tarefa: Controle de LEDs e botões com interrupções

Este projeto demonstra o uso de interrupções (IRQs) no microcontrolador RP2040 para controlar LEDs e botões na placa BitDogLab. Ele implementa:

- Interrupções para os botões com tratamento de debounce via software.
- Controle de uma matriz 5x5 WS2812 para exibir números de 0 a 9.
- Piscar continuamente o LED RGB vermelho (GPIO 13) 5 vezes por segundo.

## Componentes utilizados:

- Matriz 5x5 WS2812 (GPIO 7).
- LED RGB (GPIOs 11, 12, 13).
- Botão A (GPIO 5) → Incrementa o número na matriz.
- Botão B (GPIO 6) → Decrementa o número na matriz.

Vídeo com a demonstração: https://drive.google.com/file/d/1UKhyTmheUV3u9Esu5L1-9YUhV2cKzb6F/view?usp=sharing
