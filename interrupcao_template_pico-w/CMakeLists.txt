cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

set(CMAKE_C STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)


project(Interrupcao C CXX ASM)

pico_sdk_init()

add_executable(Interrupcao
        main.c
        )

# Vincule as bibliotecas necessárias ao seu projeto
target_link_libraries(Interrupcao
    pico_stdlib              # Biblioteca padrão do Pico
    pico_cyw43_arch_none     # Biblioteca para usar Wi-Fi e o LED no Pico W
    pico_stdlib hardware_adc # Biblioteca para ADC
    )       

#configura a saída padrão como sendo uma 'Serial-Over USB'
if (SIMULADOR STREQUAL "y")
   pico_enable_stdio_usb(Interrupcao 0)
   pico_enable_stdio_uart(Interrupcao 1)
   message("Usando os pinos da UART como stdout")
else ()
   pico_enable_stdio_usb(Interrupcao 1)
   pico_enable_stdio_uart(Interrupcao 0)
endif ()

# create map/bin/hex file etc.
pico_add_extra_outputs(Interrupcao)