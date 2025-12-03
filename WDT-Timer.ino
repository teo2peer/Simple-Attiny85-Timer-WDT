/**
 * ATtiny85 - Watchdog Timer Externo de Ultra Bajo Consumo
 * 
 * Funcionalidad:
 * - El ATtiny duerme el 99% del tiempo (Modo Power Down).
 * - Despierta cada ~8 segundos mediante el Watchdog Timer interno.
 * - Cuenta ciclos hasta llegar a 24 horas (aprox).
 * - Envía un pulso de reset por el Pin PB2 a un optoacoplador.
 * 
 * Hardware:
 * - Salida: PB2 (Pata fisica 7) -> Resistencia 220ohm -> Optoacoplador.
 * - Frecuencia de reloj recomendada: 1 MHz (Internal).
 */

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

// --- CONFIGURACION ---

// Pin PB2 es la pata fisica 7 del ATtiny85
#define RESET_PIN 2          

// Tiempo que dura el pulso de reset (en milisegundos)
#define RESET_DURATION_MS 200   

// CALCULO DE TIEMPO:
// El Watchdog se despierta cada 8 segundos aprox.
// 24 Horas = 86400 segundos.
// 86400 / 8 = 10800 ciclos.
// Nota: Ajusta este valor si tu ATtiny va muy rápido o muy lento.
#define TARGET_COUNTS 10800  

// Variable volatil porque se modifica dentro de la interrupcion
volatile uint16_t watchdogCounter = 0; 

// --- INTERRUPCION DEL WATCHDOG (ISR) ---
// Se ejecuta cada vez que el temporizador (8s) se desborda
ISR(WDT_vect) {
  watchdogCounter++;
}

void setup() {
  // 1. Deshabilitar el Watchdog al inicio por seguridad
  // Evita bucles de reinicio si la configuración anterior era incorrecta
  wdt_disable();

  // 2. Configurar Pin de Salida (PB2)
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW); // Asegurar que arranca apagado

  // 3. AHORRO DE ENERGIA: Pines no utilizados
  // Configurar pines sobrantes como INPUT_PULLUP evita que "floten" y gasten bateria.
  // Pines libres: PB0(0), PB1(1), PB3(3), PB4(4)
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  // 4. AHORRO DE ENERGIA: Deshabilitar ADC (Convertidor Analogico)
  // Esto ahorra aprox 200uA constantes. Vital para bajo consumo.
  ADCSRA &= ~(1 << ADEN);

  // 5. Configurar el Watchdog Timer
  noInterrupts(); // Desactivar interrupciones durante la config
  
  MCUSR &= ~(1 << WDRF); // Limpiar la bandera de reset del watchdog
  
  // Habilitar el cambio de configuración (Secuencia de seguridad)
  WDTCR = (1 << WDCE) | (1 << WDE);
  
  // Configurar:
  // WDP3 y WDP0 en 1 = Prescaler de ~8.0 segundos
  // WDIE en 1 = Modo Interrupción (No resetea el ATtiny, solo interrumpe)
  WDTCR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);
  
  interrupts(); // Reactivar interrupciones
}

void loop() {
  // Verificar si hemos llegado al tiempo objetivo (ej. 24h)
  if (watchdogCounter >= TARGET_COUNTS) {
    
    // --- SECUENCIA DE RESET ---
    digitalWrite(RESET_PIN, HIGH); // Activar Optoacoplador
    delay(RESET_DURATION_MS);      // Esperar 200ms
    digitalWrite(RESET_PIN, LOW);  // Desactivar Optoacoplador
    
    // Reiniciar contador para el siguiente ciclo
    watchdogCounter = 0;
  }

  // --- DORMIR ---
  // Configurar modo de sueño más profundo (Power Down)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  sleep_enable();
  
  // Apagar Brown Out Detection (BOD) por software si el chip lo permite
  // Ahorra unos 15uA extra.
  sleep_bod_disable(); 
  
  // El procesador se detiene aquí hasta que pasen 8 segundos
  sleep_cpu(); 
  
  // --- DESPERTAR ---
  // El código continua aquí después de la interrupción (ISR)
  sleep_disable();
}
