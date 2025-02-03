#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define BUTTON_A 5
#define BUTTON_B 6
#define RED_LED_PIN 13  // Alterado para o GPIO 13 para o LED vermelho
#define GREEN_LED_PIN 12
#define BLUE_LED_PIN 11
#define DEBOUNCE_DELAY_MS 200  // Tempo de debounce

#define IS_RGBW false
#define STACK_SIZE 10  // Tamanho máximo da pilha de cores

PIO pio = pio0;
uint sm = 0;

volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_b_time = 0;

// Pilha para armazenar os números anteriores
int number_stack[STACK_SIZE];
int stack_top = -1;  // Índice do topo da pilha (-1 indica que está vazia)

int current_number = 0;  // Variável para armazenar o número atual

// Função para empilhar um número
void push_number(int number) {
    if (stack_top < STACK_SIZE - 1) {
        number_stack[++stack_top] = number;
    }
}

// Função para desempilhar um número
int pop_number() {
    if (stack_top >= 0) {
        return number_stack[stack_top--];
    }
    return 0; // Retorna 0 se a pilha estiver vazia
}

// Função para configurar um pixel da matriz de LEDs
void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

// Função para inverter a matriz 5x5 em 180 graus
void invert_matrix_180(uint8_t *number) {
    uint8_t inverted[NUM_PIXELS];

    // Inverter as linhas
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            inverted[(4 - row) * 5 + col] = number[row * 5 + col];
        }
    }

    // Copiar a matriz invertida de volta
    for (int i = 0; i < NUM_PIXELS; i++) {
        number[i] = inverted[i];
    }
}

// Mapeamento dos números de 0 a 9 para a matriz 5x5
const uint8_t numbers[10][NUM_PIXELS] = {
    // Número 0
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 1
    {0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 0},
    // Número 2
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0},
    // Número 3
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 4
    {0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0},
    // Número 5
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 6
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 7
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},
    // Número 8
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 9
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0}
};

// Função para exibir um número na matriz de LEDs com brilho reduzido
void display_number(int number) {
    uint8_t temp[NUM_PIXELS];

    // Copiar o número para uma variável temporária para inverter
    for (int i = 0; i < NUM_PIXELS; i++) {
        temp[i] = numbers[number][i];
    }

    invert_matrix_180(temp);  // Inverte a matriz 180 graus

    // Fator de atenuação (controle de brilho)
    float brightness_factor = 0.2;  // 20% de brilho

    // Exibir os LEDs da matriz com brilho ajustado
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int index = row * 5 + col;
            if (temp[index] == 1) {
                // Aplicando a atenuação na cor azul
                uint32_t green_color = 0xFF0000;
                uint8_t r = (green_color >> 16) & 0xFF;
                uint8_t g = (green_color >> 8) & 0xFF;
                uint8_t b = green_color & 0xFF;

                // Aplicando o fator de brilho
                r = (uint8_t)(r * brightness_factor);
                g = (uint8_t)(g * brightness_factor);
                b = (uint8_t)(b * brightness_factor);

                // Recria a cor ajustada e define o pixel
                uint32_t dimmed_color = (r << 16) | (g << 8) | b;
                set_pixel(index, dimmed_color);
            } else {
                set_pixel(index, 0x000000);  // Apaga o LED
            }
        }
    }
}

void set_rgb_led(bool r, bool g, bool b) {
    gpio_put(RED_LED_PIN, r);
    gpio_put(GREEN_LED_PIN, g);
    gpio_put(BLUE_LED_PIN, b);
}

//Função de interrupção dos botões
void button_isr(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A) {
        if (current_time - last_button_a_time > DEBOUNCE_DELAY_MS) {  // Verifica debounce
            last_button_a_time = current_time;
            push_number(current_number);
            current_number = (current_number + 1) % 10;
            display_number(current_number);
        }
    } 
    else if (gpio == BUTTON_B) {
        if (current_time - last_button_b_time > DEBOUNCE_DELAY_MS) {  // Verifica debounce
            last_button_b_time = current_time;
            if (stack_top >= 0) {
                current_number = pop_number();
                display_number(current_number);
            }
        }
    }
}

int main() {
    stdio_init_all();

    // Configuração dos LEDs RGB
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Configuração dos botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configura as interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_isr);

    // Inicializar a configuração do PIO
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);

    // Exibir o número inicial
    current_number = 0;
    display_number(current_number);

    uint64_t last_time = 0;
    bool red_led_on = false;

    while (1) {
        // Pisca o LED vermelho continuamente 5 vezes por segundo
        uint64_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_time >= 100) {  // Alterna o estado a cada 100ms (5Hz)
            red_led_on = !red_led_on;
            gpio_put(RED_LED_PIN, red_led_on);  // Pisca o LED vermelho
            last_time = current_time;
        }
    }

    return 0;
}
