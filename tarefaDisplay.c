#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "ssd1306.h"
#include "font.h"
#include "framesNumeros.h"
#include "ws2812.pio.h"

// Definição dos pinos utilizados
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define SSD1306_ADDR 0x3C

#define WS2812_PIN 7
#define NLEDS 25

#define BUTTON_A 5
#define BUTTON_B 6
#define LED_RGB_GREEN 11
#define LED_RGB_BLUE 12

// Variáveis globais para controle dos periféricos
static PIO pio;
static int sm;
static uint dma_chan;
static uint32_t fitaEd[NLEDS];

// Flags de eventos e estados dos LEDs
static volatile bool button_a_event = false;
static volatile bool button_b_event = false;
static volatile bool led_green_state = false;
static volatile bool led_blue_state = false;
static volatile uint32_t last_time = 0; 

ssd1306_t ssd;

// Declaração das funções
void setup_i2c();
void setup_ss1306();
void setup_buttons();
void setup_leds();
static void gpio_irq_handler(uint gpio, uint32_t events);
void display_state();
void ws2812_init();
void atualizaFita();
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void display_character(char c);
void display_number(int num);

// ============================ CONFIGURAÇÃO INICIAL ============================

int main() {
    // Inicializa a comunicação serial USB
    stdio_init_all();

    // Configuração dos periféricos
    setup_i2c();
    setup_ss1306();
    setup_buttons();
    setup_leds();
    ws2812_init();

    // Inicializa a matriz de LEDs WS2812 com todos apagados
    memset(fitaEd, 0, sizeof(fitaEd));
    atualizaFita();

    while (true) {
        // Verifica se há entrada no buffer serial
        if (stdio_usb_connected()) {
            int c;
            while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
                if (c == '\n' || c == '\r') {
                    continue; 
                }

                printf("Recebido: '%c'\n", c);

                // Exibir números na matriz WS2812
                if (c >= '0' && c <= '9') {
                    display_number(c - '0');  
                } 
                // Exibir caracteres no display OLED
                else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                    display_character(c);
                }
            }
        }

        // Processamento dos eventos dos botões
        if (button_a_event) {
            button_a_event = false;
            led_green_state = !led_green_state;
            gpio_put(LED_RGB_GREEN, led_green_state);
            printf("Botao A foi pressionado\n");
            display_state();
        }

        if (button_b_event) {
            button_b_event = false;
            led_blue_state = !led_blue_state;
            gpio_put(LED_RGB_BLUE, led_blue_state);
            printf("Botao B foi pressionado\n");
            display_state();
        }
    }
    return 0;
}

// ============================ FUNÇÕES DO DISPLAY OLED ============================

// Exibe um caractere no display OLED
void display_character(char c) {
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);

    char str[2] = {c, '\0'};
    ssd1306_draw_string(&ssd, str, 56, 26);
    ssd1306_send_data(&ssd);
}

// Exibe o estado dos LEDs no display OLED
void display_state() {
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);

    if (led_blue_state && led_green_state) {
        ssd1306_draw_string(&ssd, "LEDs ON", 30, 25);
    } else if (led_blue_state) {
        ssd1306_draw_string(&ssd, "LED Azul ON", 17, 25);
    } else if (led_green_state) {
        ssd1306_draw_string(&ssd, "LED Verde ON", 17, 25);
    } else {
        ssd1306_draw_string(&ssd, "LEDs OFF", 30, 25);
    }
    ssd1306_send_data(&ssd);
}

// ============================ CONFIGURAÇÃO DOS BOTÕES ============================

// Configura os botões com interrupções
void setup_buttons() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
}

// Interrupção dos botões com debounce
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000) {
        last_time = current_time;

        if (gpio == BUTTON_A) {
            button_a_event = true;
        } else if (gpio == BUTTON_B) {
            button_b_event = true;
        }
    }
}

// ============================ CONFIGURAÇÃO DOS LEDS ============================

// Configura os LEDs RGB
void setup_leds() {
    gpio_init(LED_RGB_GREEN);
    gpio_set_dir(LED_RGB_GREEN, GPIO_OUT);

    gpio_init(LED_RGB_BLUE);
    gpio_set_dir(LED_RGB_BLUE, GPIO_OUT);
}

// ============================ CONTROLE DOS WS2812 ============================

// Inicializa WS2812 no PIO
void ws2812_init() {
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
    dma_chan = dma_claim_unused_channel(true);
}

// Atualiza os LEDs usando DMA
void atualizaFita() {
    dma_channel_wait_for_finish_blocking(dma_chan);
    while (!pio_sm_is_tx_fifo_empty(pio, sm)) {
        sleep_us(10);
    }
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    dma_channel_configure(dma_chan, &c, &pio->txf[sm], fitaEd, NLEDS, true);
    sleep_us(300);
}

// Exibe um número na matriz de LEDs WS2812
void display_number(int num) {
    for (int i = 0; i < NLEDS; i++) {
        fitaEd[i] = numeros[num][i] ? urgb_u32(6,6,6) : urgb_u32(0, 0, 0);
    }
    atualizaFita();
}

// ============================ FUNÇÕES AUXILIARES ============================

// Configuração do barramento I2C
void setup_i2c() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Configuração do display SSD1306
void setup_ss1306() {
    ssd1306_init(&ssd, 128, 64, false, SSD1306_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Conversão de cor RGB para formato WS2812
inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(b) << 8);
}
