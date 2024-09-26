/*
Conforme os conteúdos vistos em aula o Micrcontrolador RP2040, presente na Raspberry Pi Pico (w), 
possui timers como um de seus periféricos, sendo assim escreva um firmware que:
Através de Uma Interrupção do timer funcione como um relógio, contanto horas, minutos e segundos;
O horário deverá ser enviado ao computador pela porta serial;
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
#define TIMER_MS 1000              // Intervalo do timer em milissegundos

//flag que controla se o timer foi disparado ou não
volatile bool timer_disparado = false;
bool led_status = false;
int cont = 0;
int minutos = 0;
int horas = 0;

//Protótipos
bool per_timer_callback(struct repeating_timer *t);

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

// Incrementa os segundos
cont++;

// Se os segundos chegam a 60, incrementa os minutos e zera os segundos
if (cont >= 60) {
    cont = 0;
    minutos++;
}

// Se os minutos chegam a 60, incrementa as horas e zera os minutos
if (minutos >= 60) {
    minutos = 0;
    horas++;
}

// Se as horas chegam a 24, reinicia o ciclo do relógio
if (horas >= 24) {
    horas = 0;
}

// Envia o horário formatado para o computador via Serial
printf("Horário: %02d:%02d:%02d\n", horas, minutos, cont);

return true;

}
