# 🍷 Vinheria Agnello · ESP32 + MQTT (CP5 · Edge Computing)

Projeto do Check Point 05 de Edge Computing: um **ESP32 simulado no Wokwi** monitora as condições de armazenamento de vinhos da Vinheria Agnello, lendo **temperatura e umidade (DHT22)** e **luminosidade (LDR)**, e envia os dados para o **servidor de IoT (FIWARE)** via **MQTT**. O aplicativo **MyMQTT** é usado para ler as mensagens do ESP32 e para enviar comandos de volta a ele (ligar/desligar um LED de alerta).

🔗 **Simulação no Wokwi:** https://wokwi.com/projects/479325982275937281
🎥 **Vídeo de demonstração:** https://youtu.be/rouGTOE3Us4

## 👥 Integrantes

| Nome | RM |
|---|---|
| Enzo Dos Santos Galbiatti Biagiotti | RM568894 |
| Leonardo Farias Novaes Aguiar| RM570991|
| Maria Eduarda Santos Gomes Da Silva | RM573097 |
| Paulo Ricardo Siqueira de Carlos  | RM569992|
| Heitor Ortiz Nogueira | RM569762|
## 🏗️ Arquitetura

```
┌─────────────────────────┐   publica: /TEF/vinheriaENZ09/attrs    ┌──────────────────────────┐
│ ESP32 (Wokwi)           │ ─────────────────────────────────────▶ │ Servidor IoT (FIWARE)    │
│ DHT22 · LDR · LED       │                                        │ Broker Mosquitto :1883   │
│                         │ ◀───────────────────────────────────── │ 3.135.234.101            │
└─────────────────────────┘   assina:  /TEF/vinheriaENZ09/cmd      └────────────┬─────────────┘
                                                                                │
                                                                   ┌────────────┴─────────────┐
                                                                   │ MyMQTT (celular)         │
                                                                   │ lê /attrs · publica /cmd │
                                                                   └──────────────────────────┘
```

| Tópico | Direção | Conteúdo |
|---|---|---|
| `/TEF/vinheriaENZ09/attrs` | ESP32 → servidor | `t|<temperatura>|h|<umidade>|l|<luminosidade>` a cada 5 s |
| `/TEF/vinheriaENZ09/cmd` | servidor → ESP32 | `vinheriaENZ09@on|` ou `vinheriaENZ09@off|` |
| `/TEF/vinheriaENZ09/attrs/s` | ESP32 → servidor | `on` ou `off` (confirma o estado do LED) |

O formato das mensagens segue o padrão **UltraLight 2.0** usado pelo IoT Agent MQTT do FIWARE, preparando a integração com o Orion Context Broker nos próximos checkpoints.

## 🔌 Circuito

| Componente | Pino do componente | ESP32 |
|---|---|---|
| DHT22 | VCC / SDA / GND | 3V3 / **GPIO 15** / GND |
| LDR (photoresistor sensor) | VCC / AO / GND | 3V3 / **GPIO 34** / GND |
| LED vermelho | Ânodo / Cátodo | **GPIO 2** / resistor 220 Ω → GND |

## 📁 Arquivos

| Arquivo | Descrição |
|---|---|
| `sketch.ino` | Código do ESP32: Wi-Fi, leitura dos sensores, publicação MQTT e recebimento de comandos |
| `diagram.json` | Circuito do Wokwi (ESP32, DHT22, LDR, LED e resistor) |
| `libraries.txt` | Bibliotecas usadas: PubSubClient, DHT sensor library e Adafruit Unified Sensor |

## 🚀 Como executar

### 1. Simulação no Wokwi

1. Abra o link do projeto acima, **ou** crie um projeto novo em [wokwi.com/projects/new/esp32](https://wokwi.com/projects/new/esp32) e substitua `sketch.ino` e `diagram.json` pelos arquivos deste repositório.
2. Em **Library Manager**, adicione **PubSubClient** e **DHT sensor library** (Adafruit).
3. Confira as configurações no topo do `sketch.ino`:

```cpp
const char* BROKER_IP   = "3.135.234.101";   // servidor de IoT
const int   BROKER_PORT = 1883;
const char* DEVICE_ID   = "vinheriaENZ09";   // ID único do grupo
```

4. Clique em **▶ Start**. O Serial Monitor deve mostrar:

```
Wi-Fi OK
Conectando ao broker MQTT... OK
Inscrito em /TEF/vinheriaENZ09/cmd
Publicado em /TEF/vinheriaENZ09/attrs -> t|13.0|h|62.0|l|75
```

5. Durante a simulação, clique no **DHT22** e no **LDR** para alterar temperatura, umidade e luminosidade e ver os valores mudarem nas mensagens.

### 2. Leitura e escrita com o MyMQTT

1. Instale o **MyMQTT** (Android/iOS) e configure a conexão:

| Campo | Valor |
|---|---|
| Host | `3.135.234.101` |
| Port | `1883` |
| SSL | desligado |
| Versão | MQTT V3 |
| Usuário / senha | em branco |

2. **Leitura:** em **Subscribe**, assine `/TEF/vinheriaENZ09/attrs` e `/TEF/vinheriaENZ09/attrs/s`. As leituras aparecem na aba **Dashboard** a cada 5 segundos.
3. **Escrita:** em **Publish**, use o tópico `/TEF/vinheriaENZ09/cmd`:
   - `vinheriaENZ09@on|` → acende o LED do ESP32
   - `vinheriaENZ09@off|` → apaga o LED

   O ESP32 confirma publicando `on`/`off` em `/TEF/vinheriaENZ09/attrs/s`.

### 3. Conferir pelo terminal (opcional)

Com o cliente do Mosquitto instalado, é possível acompanhar as mensagens de qualquer computador:

```bash
mosquitto_sub -h 3.135.234.101 -t '/TEF/vinheriaENZ09/#' -v
mosquitto_pub -h 3.135.234.101 -t '/TEF/vinheriaENZ09/cmd' -m 'vinheriaENZ09@on|'
```

## 🛠️ Solução de problemas

| Sintoma | O que verificar |
|---|---|
| `PubSubClient.h: No such file` | Adicione as bibliotecas no Library Manager do Wokwi |
| `falhou (rc=-2)` ou `rc=-4` | `BROKER_IP` e `BROKER_PORT` corretos e servidor no ar |
| MyMQTT conectado mas sem mensagens | Tópico exato, com `/` no início e respeitando maiúsculas; mensagens aparecem na aba Dashboard |
| LED não acende | Publicar no tópico terminado em **`/cmd`** (não em `/attrs`) |

## 🧰 Tecnologias

ESP32 · Wokwi · C++/Arduino · DHT22 · LDR · MQTT · Mosquitto · FIWARE · MyMQTT
