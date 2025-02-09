# Comunicação Serial e Controle de LEDs na Raspberry Pi Pico W

## Descrição do Projeto
Este projeto utiliza uma **Raspberry Pi Pico W** para explorar as interfaces de comunicação serial **UART e I2C**, além do controle de **LEDs WS2812** e um **display SSD1306**. O objetivo principal é demonstrar o uso dessas interfaces combinadas com interrupções e debounce em botões.

## Componentes Utilizados
- **Raspberry Pi Pico W**
- **Matriz de LEDs WS2812 (5x5)**
- **LED RGB** (Pinos: 11, 12 e 13)
- **Botão A (GPIO 5)** para alternar LED Verde
- **Botão B (GPIO 6)** para alternar LED Azul
- **Display SSD1306** via I2C (GPIO 14 e 15)

## Estrutura do Código
O projeto é organizado nos seguintes arquivos:

### 1. **`main.c`**
Arquivo principal que configura os periféricos, gerencia os eventos dos botões e controla a comunicação serial.

### 2. **`ssd1306.h`**
Biblioteca para manipulação do display OLED SSD1306 via I2C.

### 3. **`framesNumeros.h`**
Contém a representação de números para exibição na matriz WS2812.

### 4. **`ws2812.pio.h`**
Controla os LEDs endereçáveis WS2812 através do **PIO (Programmable I/O)** da Raspberry Pi Pico.

## Funcionalidades Implementadas
- **Leitura de caracteres via UART**: caracteres digitados no **Serial Monitor** do VS Code são exibidos no **display OLED**.
- **Exibição de números na matriz WS2812**: ao digitar um número (0-9) no Serial Monitor, ele é exibido na matriz.
- **Botão A**: Alterna o estado do LED Verde e exibe a informação no **display OLED** e no **Serial Monitor**.
- **Botão B**: Alterna o estado do LED Azul e exibe a informação no **display OLED** e no **Serial Monitor**.
- **Debouncing via software** para os botões, garantindo leituras confiáveis.

## Configuração do Hardware
| Componente       | Pino na Pico W |
|-----------------|--------------|
| Matriz WS2812   | GPIO 7        |
| LED Verde       | GPIO 11       |
| LED Azul        | GPIO 12       |
| Botão A         | GPIO 5        |
| Botão B         | GPIO 6        |
| Display SSD1306 | GPIO 14, 15   |

## Como Compilar e Rodar

### 1. Instale o Raspberry Pi Pico SDK
Siga as instruções oficiais do SDK: [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)

### 2. Clone este repositório
```sh
    git clone https://github.com/GabrielSampa1o/Controle-de-Interrup-es-e-LEDs-WS2812-na-Raspberry-Pi-Pico-W.git
    cd Controle-de-Interrup-es-e-LEDs-WS2812-na-Raspberry
```

### 3. Abra o VS Code e importe o projeto:
- Vá até a **Extensão Raspberry Pi Pico**.
- Selecione **Import Project**.
- Escolha a pasta do repositório clonado.
- Clique em **Import**.

### 4. Compilar o código:
- Utilize a opção de **Build** da extensão.

### 5. Carregue o binário na Pico
1. Pressione e segure o **botão BOOTSEL** da Raspberry Pi Pico W.
2. Conecte-a ao PC via **USB**.
3. Copie o arquivo **.uf2** gerado para a unidade montada.

## Exemplo de Uso
1. Digite um **número (0-9)** no **Serial Monitor** → O número será exibido na **matriz WS2812**.
2. Digite uma **letra (A-Z, a-z)** → O caractere será mostrado no **display SSD1306**.
3. Pressione **Botão A** → Alterna o **LED Verde** e exibe a mensagem correspondente no **Serial Monitor** e **OLED**.
4. Pressione **Botão B** → Alterna o **LED Azul** e exibe a mensagem correspondente no **Serial Monitor** e **OLED**.

## Possíveis Melhorias
- Suporte para exibição de frases completas no **display SSD1306**.
- Integração com comunicação **SPI** para novos dispositivos.
- Implementação de padrões gráficos na **matriz WS2812**.

## Autor
- [Gabriel Silva Sampaio]


## Demonstração
[Assista ao vídeo de demonstração aqui](https://www.dropbox.com/scl/fi/jmjt36kcc51w9gk4dphgt/VID_20250209_181038.mp4?rlkey=jgd36o7u5kb3s7904wqd5x0i4&st=x4lxkk4u&dl=0)

