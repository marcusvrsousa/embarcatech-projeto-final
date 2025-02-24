#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Definições de pinos
#define PINO_DHT 28
#define PIN_LED_RED 2
#define PIN_LED_GREEN 5
#define PIN_LED_BLUE 9
#define PIN_BUZZER 13
#define TIMEOUT_US 1000 

// Função para medir a duração de um pulso
static uint32_t medir_pulso(bool nivel) {
    uint32_t inicio = time_us_32();
    while (gpio_get(PINO_DHT) == nivel) {
        if (time_us_32() - inicio > TIMEOUT_US) {
            return TIMEOUT_US; // Timeout
        }
    }
    return time_us_32() - inicio;
}

// Envia o pulso inicial para o DHT22
void enviar_pulso_inicial() {
    gpio_set_dir(PINO_DHT, GPIO_OUT);
    gpio_put(PINO_DHT, 0);  // Pulso baixo por 18ms
    sleep_ms(18);
    gpio_put(PINO_DHT, 1);  // Pulso alto por 20-40µs
    sleep_us(30);
    gpio_set_dir(PINO_DHT, GPIO_IN); // Configura como entrada
}

// Lê os dados do DHT22
bool ler_dados_dht22(uint8_t *dados) {
    memset(dados, 0, 5); // Zera o buffer

    // Aguarda a resposta do sensor
    if (medir_pulso(0) == TIMEOUT_US || medir_pulso(1) == TIMEOUT_US) {
        printf("Erro: Sem resposta do sensor\n");
        return false;
    }

    // Lê os 40 bits de dados (5 bytes)
    for (int i = 0; i < 40; i++) {
        if (medir_pulso(0) == TIMEOUT_US) {
            printf("Erro: Pulso baixo longo demais\n");
            return false;
        }

        uint32_t duracao = medir_pulso(1);
        if (duracao == TIMEOUT_US) {
            printf("Erro: Pulso alto longo demais\n");
            return false;
        }

        // Determina o valor do bit com base na duração do pulso
        if (duracao > 50) {
            dados[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    // Verifica o checksum
    uint8_t checksum = dados[0] + dados[1] + dados[2] + dados[3];
    if (checksum != dados[4]) {
        printf("Erro: Checksum inválido\n");
        return false;
    }

    return true;
}

// Inicializa GPIOs para dispositivos de controle
void inicializar_gpios() {
    gpio_init(PIN_LED_RED);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);
    gpio_put(PIN_LED_RED, 0);

    gpio_init(PIN_LED_GREEN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);
    gpio_put(PIN_LED_GREEN, 0);

    gpio_init(PIN_LED_BLUE);
    gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);
    gpio_put(PIN_LED_BLUE, 0);

    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_put(PIN_BUZZER, 0);
}


// Função para piscar LED AZUL lentamente
void piscar_led_azul_devagar() {
    for (int i = 0; i < 5; i++) {
        gpio_put(PIN_LED_BLUE, 1);   // Liga LED AZUL
        sleep_ms(1000);             // Espera 1 segundo
        gpio_put(PIN_LED_BLUE, 0);   // Desliga LED AZUL
        sleep_ms(1000);             // Espera 1 segundo
    }
}
// Função para piscar LED AZUL rapidamente
void piscar_led_azul_rapidamente() {
    for (int i = 0; i < 5; i++) {
        gpio_put(PIN_LED_BLUE, 1);   // Liga LED AZUL
        sleep_ms(200);              // Espera 200ms
        gpio_put(PIN_LED_BLUE, 0);   // Desliga LED AZUL
        sleep_ms(200);              // Espera 200ms
    }
}

// Função para piscar LED vermelho lentamente
void piscar_led_vermelho_devagar() {
    for (int i = 0; i < 5; i++) {
        gpio_put(PIN_LED_RED, 1);   // Liga LED vermelho
        sleep_ms(1000);             // Espera 1 segundo
        gpio_put(PIN_LED_RED, 0);   // Desliga LED vermelho
        sleep_ms(1000);             // Espera 1 segundo
    }
}
// Função para piscar LED vermelho rapidamente
void piscar_led_vermelho_rapidamente() {
    for (int i = 0; i < 5; i++) {
        gpio_put(PIN_LED_RED, 1);   // Liga LED vermelho
        sleep_ms(200);              // Espera 200ms
        gpio_put(PIN_LED_RED, 0);   // Desliga LED vermelho
        sleep_ms(200);              // Espera 200ms
    }
}



// Função para gerar som no buzzer
void tocar_buzzer(uint32_t frequencia_hz, uint32_t duracao_ms) {
    uint32_t tempo_on = 1000000 / (frequencia_hz * 2);  // Meio ciclo da frequência
    uint32_t tempo_off = tempo_on;

    uint32_t fim = time_us_32() + (duracao_ms * 1000);  // Converte duração de ms para us

    while (time_us_32() < fim) {
        gpio_put(PIN_BUZZER, 1);  // Ligando o buzzer
        sleep_us(tempo_on);       // Espera meio ciclo
        gpio_put(PIN_BUZZER, 0);  // Desligando o buzzer
        sleep_us(tempo_off);      // Espera meio ciclo
    }
}

void verificarTemperaturaUmidade(float temperatura, float umidade) {
    if (umidade < 40.0 || umidade >= 64.0) {
        gpio_put(PIN_LED_GREEN, 0); // Desliga LED verde
        gpio_put(PIN_LED_BLUE, 1);
        gpio_put(PIN_BUZZER, 1);
        printf("Umidade fora do recomendado (40° - 60°) Realize o ajuste imediatamente \n");
        tocar_buzzer(1500, 3000); // Som para umidade fora da faixa
        piscar_led_azul_rapidamente(); // Piscar LED vermelho rapidamente
    } else if (umidade >= 61.0 && umidade <= 63.0){
        gpio_put(PIN_LED_GREEN, 0); // Desliga LED verde (caso esteja ligado)
        gpio_put(PIN_LED_BLUE, 1);   // LED vermelho ligado
        tocar_buzzer(300, 1500);    // Som com frequência média (800Hz, duração 1.5 segundos)
        printf("Umidade elevada!. \n");
        piscar_led_azul_devagar(); // Piscar LED vermelho lentamente
  
    } else if (temperatura >= 24.0 && temperatura <= 25.0) {
        // Temperatura entre 24 e 25 graus: som diferente e LED piscando lentamente
        gpio_put(PIN_LED_GREEN, 0); // Desliga LED verde (caso esteja ligado)
        gpio_put(PIN_LED_RED, 1);   // LED vermelho ligado
        tocar_buzzer(800, 1500);    // Som com frequência média (800Hz, duração 1.5 segundos)
        printf("Temperatura entre 24°C e 25°C. Ajuste necessário!\n");
        piscar_led_vermelho_devagar(); // Piscar LED vermelho lentamente
    } else if (temperatura > 25.0 || temperatura < 15.0) {
        // Temperatura superior a 25 graus: som mais forte e LED piscando rapidamente
        gpio_put(PIN_LED_GREEN, 0); // Desliga LED verde
        gpio_put(PIN_LED_RED, 1);   // LED vermelho ligado
        tocar_buzzer(1200, 3000);   // Som mais alto (1200Hz) e duração maior (3 segundos)
        printf("Temperatura superior a 25°C! Realize o ajuste imediatamente!\n");
        piscar_led_vermelho_rapidamente(); // Piscar LED vermelho rapidamente
    } else {
        gpio_put(PIN_LED_GREEN, 1); // Liga o LED verde
        gpio_put(PIN_BUZZER, 0);    // Desliga o buzzer
        gpio_put(PIN_LED_RED, 0);   // Desliga o LED vermelho
        gpio_put(PIN_LED_BLUE, 0);   // Desliga o LED azul
        printf("Temperatura e Umidade em condições ideais! \n");
    }
}



int main() {
    stdio_init_all();       
    gpio_init(PINO_DHT);    
    inicializar_gpios();    

    uint8_t dados[5];       // Buffer para dados do sensor
    float temperatura, umidade;

    while (true) {
        printf("Iniciando leitura do sensor DHT22...\n");
        enviar_pulso_inicial();

        if (ler_dados_dht22(dados)) {
            // Converte os dados para valores reais
            umidade = ((dados[0] << 8) + dados[1]) / 10.0;
            temperatura = ((dados[2] << 8) + dados[3]) / 10.0;

            if (dados[2] & 0x80) { // Verifica se a temperatura é negativa
                temperatura = -temperatura;
            }

            // Validação adicional para valores lidos
            if (umidade >= 0.0 && umidade <= 100.0) {
                printf("Temperatura: %.1f °C, Umidade: %.1f %%\n", temperatura, umidade);
                verificarTemperaturaUmidade(temperatura, umidade);
            } else {
                printf("Erro: Umidade fora do intervalo válido (0-100%%): %.1f %%\n", umidade);
            }
        } else {
            printf("Erro ao ler o DHT22\n");
        }

        sleep_ms(3000); // Aguarda 10 segundos entre leituras
    }

    return 0;
}