/*
Conforme apresentado no vídeo, da segunda aula remota, o Micrcontrolador RP2040, presente na Raspberry Pi Pico (w), 
possui ligado a um de seus canais no conversor analógico digital um sensor de temperatura, para monitoramento da temperatura no núcleo da CPU, 
com base nos exemplos do vídeo e o conteúdo do Datasheet, escreva um firmware que:
Através de Um Timer periódico, Leia a temperatura da CPU e a envie via serial para o computador;
Os códigos desenvolvidos devem ser colocados em uma conta do github e somente seu link deve ser enviado como resposta.
*/
// Inclui a biblioteca padrão do SDK e a biblioteca do Wi-Fi

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

// Constantes de conversão de temperatura
#define CANAL_ADC_SENSOR_TEMP 4    // Canal 4 para o sensor de temperatura
#define REFERENCIA_TENSAO 3.3      // Referência de tensão para o ADC (Pico usa 3.3V)
#define VALOR_MAX_ADC 4095.0       // Valor máximo do ADC de 12 bits
#define TIMER_MS 3000              // Intervalo do timer em milissegundos

//flag que controla se o timer foi disparado ou não
volatile bool timer_disparado = false;
bool led_status = false;

//Protótipos
bool per_timer_callback(struct repeating_timer *t);
float ler_temperatura();

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