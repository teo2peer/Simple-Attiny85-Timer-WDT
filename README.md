Aquí tienes un archivo `README.md` listo para copiar y pegar en GitHub. Cumple con el requisito de no usar iconos/emojis y presenta la información de forma técnica y limpia.

***

# ATtiny85 External Watchdog Timer (Low Power)

Este repositorio contiene el código fuente para convertir un microcontrolador ATtiny85 en un Watchdog Timer (WDT) externo de ultra bajo consumo.

El objetivo de este proyecto es reiniciar físicamente una placa principal (Arduino, ESP32, nodo LoRa, etc.) mediante un optoacoplador en intervalos de tiempo regulares (por ejemplo, cada 24 horas). Esto asegura que los dispositivos remotos o alimentados por energía solar se recuperen de bloqueos de software o condiciones de "Brown-out".

## Caracteristicas

*   **Ultra Bajo Consumo:** El ATtiny85 permanece en modo `SLEEP_MODE_PWR_DOWN` el 99% del tiempo, consumiendo aproximadamente 4uA - 5uA (dependiendo del voltaje y fusibles).
*   **Sin Librerias Externas:** Utiliza manipulación directa de registros AVR para maximizar la eficiencia.
*   **Aislamiento:** Utiliza un optoacoplador para aislar eléctricamente el circuito de reset de la placa objetivo.
*   **Temporizacion:** Utiliza el Watchdog Timer interno del ATtiny para despertar el chip periódicamente y contar el tiempo sin bloquear el procesador.

## Requisitos de Hardware

*   Microcontrolador ATtiny85 (DIP-8 o SMD).
*   Optoacoplador (ej. PC817).
*   Resistencia de 100 a 220 Ohmios.
*   Fuente de alimentación (2.7V - 5V).

## Diagrama de Conexion

El sistema utiliza el pin PB2 (Pata física 7) para activar el optoacoplador.

```text
       ATtiny85
      +---v---+
  RST |1     8| VCC (+) ---> 3V - 5V
  PB3 |2     7| PB2 -------> Resistencia (220 Ohm) ---> Anodo Optoacoplador (+)
  PB4 |3     6| PB1
  GND |4     5| PB0
      +-------+
         |
        GND (-)
         |
    Catodo Optoacoplador (-)


    Conexion del Lado del Optoacoplador (Salida):
    ---------------------------------------------
    Colector (C) ---> Pin RESET del dispositivo objetivo
    Emisor   (E) ---> GND del dispositivo objetivo
```

**Nota:** Es crucial usar la resistencia entre el PB2 y el optoacoplador para limitar la corriente y proteger el pin del microcontrolador.

## Configuracion del Entorno (Arduino IDE)

Para compilar y subir este código, se recomienda usar el gestor de tarjetas **ATTinyCore**.

1.  **Board:** ATtiny25/45/85
2.  **Chip:** ATtiny85
3.  **Clock:** 1 MHz (Internal)
4.  **BOD Level:** B.O.D. Disabled (Recomendado para máximo ahorro de energía)

**Importante:** Si es la primera vez que programas el chip con esta configuración, debes seleccionar la opción "Burn Bootloader" para configurar los fusibles correctamente (1MHz), incluso si no usas un bootloader real.

## Personalizacion del Tiempo

El Watchdog interno del ATtiny no es un reloj de precisión; se basa en un oscilador RC que varía ligeramente con la temperatura y el voltaje. El código despierta el chip cada ~8 segundos aproximadamente.

Para cambiar el intervalo de reset, modifica la siguiente linea en el codigo:

```cpp
// Calculo aproximado:
// Segundos Totales / 8 segundos = Numero de Ciclos
// Ejemplo 24 Horas: (24 * 3600) / 8 = 10800
#define TARGET_COUNTS 10800
```

Se recomienda calibrar este valor midiendo el tiempo real con un cronómetro para periodos largos.

## Funcionamiento Tecnico

1.  **Setup:** Se configuran los pines no utilizados como `INPUT_PULLUP` para evitar pines flotantes que consuman corriente. Se deshabilita el ADC (Convertidor Analógico-Digital).
2.  **Ciclo Principal:**
    *   El sistema entra en sueño profundo (`SLEEP_MODE_PWR_DOWN`).
    *   El Watchdog Timer interno despierta la CPU cada 8 segundos mediante una interrupción.
    *   La interrupción incrementa una variable contador.
    *   La CPU vuelve a dormir inmediatamente.
3.  **Disparo:** Cuando el contador alcanza `TARGET_COUNTS`, el pin PB2 se pone en ALTO durante 200ms, activando el optoacoplador y forzando el reinicio de la placa conectada.

## Licencia

Este proyecto es de código abierto. Siéntete libre de usarlo y modificarlo para tus necesidades.
