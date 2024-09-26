/*
Conforme os conteúdos vistos em aula o Micrcontrolador RP2040, presente na Raspberry Pi Pico (w), 
possui entre suas interfaces de comunicação os periféricos SPI e I2C,
sendo assim escreva um firmware que:
Leia os dados da Interface SPI do termopar e os exiba no display I2C
Caso, não possua o móudo SPI do termopar, a temperatura exibida pode ser a temperatura do núcleo do RP2040
Os códigos desenvolvidos devem ser colocados em uma conta do github e somente seu link deve ser enviado como resposta.
*/

// Inclui a biblioteca padrão do SDK e a biblioteca do Wi-Fi
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

// Constantes de conversão de temperatura
#define CANAL_ADC_SENSOR_TEMP 4    // Canal 4 para o sensor de temperatura
#define REFERENCIA_TENSAO 3.3      // Referência de tensão para o ADC (Pico usa 3.3V)
#define VALOR_MAX_ADC 4095.0       // Valor máximo do ADC de 12 bits
#define TIMER_MS 3000              // Intervalo do timer em milissegundos

//flag que controla se o timer foi disparado ou não
volatile bool timer_disparado = false;
bool led_status = false;

#define LCD_LIMPA_TELA     0x01
#define LCD_INICIA         0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20

#define LCD_INICIO_ESQUERDA 0x02

#define LCD_LIGA_DISPLAY 0x04

#define LCD_16x2 0x08

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE_BIT 0x04

// By default these LCD display drivers are on bus address 0x27
#define BUS_ADDR 0x27

// Modes for lcd_envia_byte
#define LCD_CARACTER  1
#define LCD_COMANDO    0

#define MAX_LINES      2
#define MAX_CHARS      16

#define DELAY_US 600

//Protótipos
bool per_timer_callback(struct repeating_timer *t);
float ler_temperatura();
void lcd_envia_comando(uint8_t val);
void lcd_pulsa_enable(uint8_t val);
void lcd_envia_byte(uint8_t val, int modo);
void lcd_limpa_tela(void);
void lcd_posiciona_cursor(int linha, int coluna);
void lcd_envia_caracter(char caractere);
void lcd_envia_string(const char *s);
void lcd_init();

int main() {

    //inicia as funções de I/O padrão, no caso via Serial-USB
    stdio_init_all();
    adc_init();

    // Habilita o sensor de temperatura interno
    adc_set_temp_sensor_enabled(true);

    // Inicializa a biblioteca Wi-Fi (necessária para controlar o LED do Pico W)
    if (cyw43_arch_init()) {
        return -1; // Retorna erro se não conseguir inicializar
    }

    // Inicializa a comunicação I2C
    i2c_init(i2c0, 100 * 1000);  // Inicializa a I2C no i2c0 com 100kHz
    gpio_set_function(4, GPIO_FUNC_I2C);  // GPIO 4 como SDA
    gpio_set_function(5, GPIO_FUNC_I2C);  // GPIO 5 como SCL
    gpio_pull_up(4);  // Ativa o pull-up no SDA
    gpio_pull_up(5);  // Ativa o pull-up no SCL

    stdio_init_all();  // Inicializa a comunicação serial
    sleep_ms(1000);    // Pequeno atraso para a inicialização

    // Inicializa o LCD
    lcd_init();

    // Limpa a tela e posiciona o cursor
    lcd_limpa_tela();
    lcd_posiciona_cursor(0, 0);  // Linha 0, Coluna 0

    printf("Inicio!\n");

    // Estrutura para o timer
    struct repeating_timer timer;

    //adicionando o timer com repetição periódica
    add_repeating_timer_ms(TIMER_MS, per_timer_callback, NULL, &timer);

    // Loop infinito para o funcionamento do pisca-pisca
    while (true) {

    // Mantém o processador ativo para o funcionamento do timer
    tight_loop_contents(); 
        
    }

    // Finaliza a biblioteca (não vai chegar aqui por causa do loop infinito)
    cyw43_arch_deinit();
}

// função de callback chamada na interrupção do timer
bool per_timer_callback(struct repeating_timer *t) {

    float temperatura = ler_temperatura();
    printf("Temperatura: %.2f°C\n", temperatura);

    // Limpa o LCD antes de atualizar
    lcd_limpa_tela();
    lcd_posiciona_cursor(0, 0);  // Linha 0, coluna 0 do LCD

    // Cria uma string formatada com a temperatura
    char temp_str[16];  // Buffer para 16 caracteres (tamanho máximo da linha do LCD)
    snprintf(temp_str, sizeof(temp_str), "Temp: %.2fC", temperatura);

    // Envia a string formatada ao LCD
    lcd_envia_string(temp_str);
    return true;

}

float ler_temperatura() {
    // Seleciona o sensor de temperatura (canal 4)
    adc_select_input(CANAL_ADC_SENSOR_TEMP);

    // Lê o valor bruto do ADC
    uint16_t adc_raw = adc_read();

    // Converte o valor bruto do ADC em tensão
    float tensao_adc = (adc_raw / VALOR_MAX_ADC) * REFERENCIA_TENSAO;

    // Converte a tensão em temperatura em Celsius usando a fórmula
    float temperatura = 27 - (tensao_adc - 0.706) / 0.001721;

    return temperatura;
}

// Função para enviar um comando ao LCD
void lcd_envia_comando(uint8_t val) {
    lcd_envia_byte(val, LCD_COMANDO);
}

// Função para enviar um caractere ao LCD
void lcd_envia_caracter(char caractere) {
    lcd_envia_byte(caractere, LCD_CARACTER);
}

// Função para enviar um byte (comando ou caractere) ao LCD
void lcd_envia_byte(uint8_t val, int modo) {
    uint8_t high_nibble = val & 0xF0;  // Parte alta do byte
    uint8_t low_nibble = (val << 4) & 0xF0;  // Parte baixa do byte
    uint8_t control = LCD_BACKLIGHT | (modo ? 0x01 : 0x00);

    // Envia a parte alta
    i2c_write_blocking(i2c0, BUS_ADDR, &high_nibble, 1, false);
    lcd_pulsa_enable(high_nibble | control);

    // Envia a parte baixa
    i2c_write_blocking(i2c0, BUS_ADDR, &low_nibble, 1, false);
    lcd_pulsa_enable(low_nibble | control);
}

// Função para enviar o pulso de habilitação
void lcd_pulsa_enable(uint8_t val) {
    i2c_write_blocking(i2c0, BUS_ADDR, &val, 1, false);
    sleep_us(DELAY_US);  // Aguarda o pulso
    val &= ~LCD_ENABLE_BIT;  // Limpa o bit de enable
    i2c_write_blocking(i2c0, BUS_ADDR, &val, 1, false);
    sleep_us(DELAY_US);  // Pequeno atraso
}

// Função para inicializar o LCD
void lcd_init() {
    sleep_ms(50);  // Aguarda o LCD estar pronto
    lcd_envia_comando(LCD_FUNCTIONSET | LCD_16x2);  // Configura o LCD para 16x2
    lcd_envia_comando(LCD_DISPLAYCONTROL | LCD_LIGA_DISPLAY);  // Liga o display
    lcd_envia_comando(LCD_ENTRYMODESET | LCD_INICIO_ESQUERDA);  // Configura o cursor para iniciar à esquerda
}

// Função para limpar a tela do LCD
void lcd_limpa_tela() {
    lcd_envia_comando(LCD_LIMPA_TELA);
    sleep_ms(2);  // Aguarda o comando de limpar a tela ser processado
}

// Função para posicionar o cursor
void lcd_posiciona_cursor(int linha, int coluna) {
    int linha_offset[] = {0x00, 0x40};  // Endereços das linhas do LCD 16x2
    lcd_envia_comando(0x80 | (coluna + linha_offset[linha]));  // Move o cursor
}

// Função para enviar uma string ao LCD
void lcd_envia_string(const char *s) {
    while (*s) {
        lcd_envia_caracter(*s++);
    }
}