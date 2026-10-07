# Guía de Referencia y Transición Técnica
## Proyecto Precedente: Ensamblaje y Validación de Red SHM V1.4

Este documento establece el puente de continuidad técnica entre el proyecto finalizado por **David Timbi** (`RSA-PPP-2026-09`) y la actual fase de validación integral y almacenamiento autónomo ejecutada por **Geovanny Cullquicondo** (`RSA-PPP-2026-11`).

---

## 📌 Datos del Proyecto Base (Precedente)

| Parámetro | Detalle |
| :--- | :--- |
| **Código Institucional** | `RSA-PPP-2026-09` |
| **Nombre del Proyecto** | Ensamblaje, Programación y Validación de Red Distribuida SHM (Serie Acelerógrafo V1.4) |
| **Pasante Autor** | David Timbi (`david.timbi@ucuenca.edu.ec`) |
| **Tutor Institucional** | Ing. Milton Muñoz (`milton.munozc@ucuenca.edu.ec`) — RSA |
| **Repositorio Oficial** | [RSA-PPP/ppp-2026-09-shm-ensamblaje-validacion](https://github.com/RSA-PPP/ppp-2026-09-shm-ensamblaje-validacion) |
| **Estado** | Culminado (Septiembre 2026) |

---

## 🔍 Resumen de Logros Heredados (Qué está 100% probado y validado)

1. **Hardware Ensamblado:**
   * 3 placas físicas operativas: 1 Nodo Concentrador y 2 Nodos Sensores (Nodo A con plano de tierra continuo y Nodo B sin plano de masa).
   * Reguladores de tensión verificados en $12\text{ V}$, $5\text{ V}$ y $3.3\text{ V}$.
2. **Configuración de Reloj del dsPIC33EP256MC202:**
   * Bits de configuración corregidos a **FRCPLL interno** (sin cristal externo), garantizando operación estable a **80 MHz (40 MIPS)**.
3. **Comunicaciones RS485:**
   * Transceptor MAX485 operando a **2 Mbps** en UART2/UART1.
   * Protocolo de tramas estructurado con cabecera `0x3A`, verificación de `IDNODO` y eco de enlace `0xF1 / 0xD2`.
4. **Sincronismo Físico en Daisy Chain (Cableado T-568B):**
   * Señal de sincronismo diferencial gobernada por MAX483.
   * Retardos de propagación caracterizados con osciloscopio Hantek:
     * Concentrador $\rightarrow$ Nodo Sensor: **$1.200\,\mu\text{s}$** (tiempo de respuesta del transceptor y latencia de interrupción `INT1`).
     * Entre Nodos Sensores A y B: apenas **$14.0\text{ ns}$** (demuestra que la topología Daisy Chain no acumula desfasajes críticos).

---

## ⚠️ Causa Raíz del Cuello de Botella Heredado (¡Leer con Atención!)

En la iteración previa de David Timbi, la prueba de concepto (PoC) de almacenamiento en tarjeta MicroSD quedó incompleta debido a una **limitación puramente física de hardware**:

> **El Problema:**  
> Los zócalos para tarjetas MicroSD soldados originalmente en las placas V1.4 **carecían del pin mecánico de detección de presencia de tarjeta (*card-detect*)**.  
> Para sortear este problema, en el firmware anterior se forzó la detección por software (`sdflags.detected = 1` y `SD_DETECCION_HARDWARE = 0`). Sin embargo, sin la señal física de inserción, la inicialización del bus SPI en baja velocidad (`CMD0`, `CMD8`, `ACMD41`) no operó con repetibilidad, generando fallos continuos ante tarjetas SanDisk y respuestas anómalas en el comando `CMD17`.

### ¿Cómo se resuelve en el proyecto actual (`RSA-PPP-2026-11`)?
1. **En la Fase 1:** Se desueldan físicamente los zócalos anteriores y se montan nuevos zócalos MicroSD que sí disponen de contacto mecánico de *card-detect*.
2. **En la Fase 2:** El firmware en MikroC **no debe forzar banderas ficticias en memoria**. La inicialización y la máquina de estados deben quedar estrictamente condicionadas a la lectura del pin de hardware del nuevo zócalo.

---

## 🗺️ Mapa de Navegación del Repositorio Anterior

Si tienes dudas o necesitas consultar el código fuente de David Timbi en [ppp-2026-09-shm-ensamblaje-validacion](https://github.com/RSA-PPP/ppp-2026-09-shm-ensamblaje-validacion), guíate por este mapa:

| Carpeta en Repo Anterior | ¿Qué contiene? | ¿Cómo utilizarlo en tu proyecto? |
| :--- | :--- | :--- |
| `firmware/` | `NodoAcelerometro.c` y `concentrador.c`. | **Referencia de estructura:** Revisa cómo se configuran los registros PPS para los pines UART y SPI, y cómo se estructuran las interrupciones. |
| `Sincronizacion Nodos/` | Proyectos de MikroC para Concentrador, Nodo 1 y Nodo 2, junto a `rs485.c`. | **Referencia de comunicación:** Consulta `rs485.c` para reutilizar la función de envío de tramas y la atención de la interrupción `INT1` para sincronismo. |
| `Main_SD/` | Proyecto `test_sd_sector2500.c` y librerías `sdcard.c/.h` y `spiSD.c/.h`. | **Librería base:** Es la versión que escribe sectores crudos de 512 bytes en el sector 2500. *(Nota: Estas librerías ya fueron copiadas a tu carpeta `firmware/drivers/`)*. |
| `Pruebas de funcionamiento sd/` | Ensayos aislados (`test_blink.c`, `test_pines.c`, `test_sin_sd.c`). | **Solo lectura:** Experimentos históricos previos. No intentes compilarlos ni migrarlos. |
| `docs/` | Esquemas electrónicos, BOM y reporte técnico final de David Timbi. | **Consulta obligatoria:** Revisa los esquemáticos para comprobar números de pines, transceptores y pistas de alimentación antes de soldar en la Fase 1. |

---

## 📦 Elementos Ya Migrados al Repositorio Actual

Para facilitarte el inicio sin contaminar tu espacio de trabajo:

* Los archivos de control de bajo nivel para la tarjeta de memoria:
  * `sdcard.c` / `sdcard.h`
  * `spiSD.c` / `spiSD.h`  
  fueron extraídos de `Main_SD/` y ya se encuentran alojados en tu directorio:
  👉 `firmware/drivers/`

Tu tarea en la **Fase 2** consistirá en limpiar y actualizar estos controladores para que respondan al nuevo pin físico de detección de tarjeta y soporten conmutación de reloj SPI (*Slow/Fast*) para tarjetas Kingston y SanDisk.
