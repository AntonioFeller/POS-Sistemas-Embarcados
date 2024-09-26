/*
Conforme os conteúdos vistos em aula o Micrcontrolador RP2040, presente na Raspberry Pi Pico (w),
possui a capacidade de lidar com interrupções externas nos seus periféricos, sendo um deles os GPIOS, sendo assim escreva um firmware que:
Através de Uma Interrupção externa num GPIO, à sua escolha, dispare um alarme;
Os códigos desenvolvidos devem ser colocados em uma conta do github e somente seu link deve ser enviado como resposta.
O Alarme pode ser construído através de um buzzer, ou simulado através de um led.
A entrada pode ser simulada por um botão, ou sensor de presença
*/

// Inclui a biblioteca padrão do SDK e a biblioteca do Wi-Fi
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include "hardware/gpio.h"

// Protótipos
void gpio_callback(uint gpio, uint32_t events);

volatile bool led_ligado = false;  // Variável que mantém o estado do LED

int main() {
    stdio_init_all();

    // Inicializa a biblioteca Wi-Fi (necessária para controlar o LED do Pico W)
    if (cyw43_arch_init()) {
        return -1; // Retorna erro se não conseguir inicializar
    }

    printf("Sistema iniciado!\n");

    // Configura o botão ou sensor no GPIO 2 como entrada com pull-up
    int sensor_pin = 2;
    gpio_init(sensor_pin);
    gpio_set_dir(sensor_pin, GPIO_IN);
    gpio_pull_up(sensor_pin);

    // Configura a interrupção no GPIO 2 para borda de subida e descida
    gpio_set_irq_enabled_with_callback(sensor_pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // Configura o LED embutido no Pico W como saída
    const uint LED_PIN = CYW43_WL_GPIO_LED_PIN;
    cyw43_arch_gpio_put(LED_PIN, 0);  // Começa com o LED desligado

    // Loop infinito para manter o programa rodando
    while (true) {
        // Verifica o estado do LED e o atualiza
        if (led_ligado) {
            cyw43_arch_gpio_put(LED_PIN, 1);  // Liga o LED se o botão estiver pressionado
        } else {
            cyw43_arch_gpio_put(LED_PIN, 0);  // Desliga o LED se o botão não estiver pressionado
        }

        tight_loop_contents();  // Mantém o processador ativo
    }

    // Finaliza a biblioteca Wi-Fi (não será alcançado devido ao loop infinito)
    cyw43_arch_deinit();
}

// Callback que será executado quando a interrupção do GPIO for disparada
void gpio_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        // Se houver uma borda de descida (botão pressionado)
        led_ligado = true;
        printf("Botão pressionado. LED ligado.\n");
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
        // Se houver uma borda de subida (botão solto)
        led_ligado = false;
        printf("Botão solto. LED desligado.\n");
    }
}
