---
titulo: "Plan de Trabajo de Prácticas Preprofesionales: Validación Integral de Nodos Sensores para Red de Monitorización de Salud Estructural (SHM)"
proyecto: "Validación Integral de Nodos Sensores para Red de Monitorización de Salud Estructural (Serie Acelerógrafo V1.4 - Nodos Sensores)"
codigo_proyecto: "RSA-PPP-2026-10"
area_tematica: "Sistemas Embebidos, Instrumentación Sísmica, Firmware en Tiempo Real (Doble Búfer), Almacenamiento SPI y Análisis en Python"
estado: "Por Iniciar"
version: "1.0"
fecha_creacion: "2026-09-25"
fecha_actualizacion: "2026-09-25"

pasante:
  nombre: "Geovanny Cullquicondo"
  cedula: "N/D"
  correo: "geovanny.cullquicondo@ucuenca.edu.ec"
  carrera: "Ingeniería en Telecomunicaciones"
  institucion: "Universidad de Cuenca"

tutoria:
  tutor_institucional: "Ing. Milton Muñoz. (Red Sísmica del Austro - RSA)"
  correo_tutor_institucional: "milton.munozc@ucuenca.edu.ec"

cronograma:
  duracion_total_horas: 144
  dedicacion_semanal_horas: 14
  duracion_semanas: 10.5
  fecha_inicio: "2026-10-05"
  fecha_fin_estimada: "2026-12-18"
  modalidad: "Presencial"

tecnologias:
  - "Microcontrolador y Arquitectura: Microchip dsPIC33EP256MC202 (16 bits, 80 MHz con oscilador FRCPLL interno, 32 KB SRAM)"
  - "Entorno de Desarrollo y Grabación: MikroC PRO for dsPIC, MPLAB IPE v6.20 y programador hardware PICkit 3"
  - "Sensores de Auscultación: Acelerómetro Triaxial de Precisión ADXL355 (Bus SPI, rango ±2g, ODR configurable)"
  - "Comunicaciones y Red Física: Bus RS485 a 2 Mbps (MAX485), Línea de sincronismo dedicada (MAX483), Topología Daisy Chain bajo norma T-568B"
  - "Almacenamiento Masivo en Flash: Tarjetas MicroSD Kingston y SanDisk (16/32 GB SDHC), zócalos con detección mecánica card-detect, bloques crudos de 512 bytes"
  - "Técnicas de Firmware en RAM: Arquitectura de Doble Búfer (Ping-Pong Buffer) para desacoplamiento temporal de interrupciones y escritura"
  - "Herramientas de Análisis Forense y Validación: Python 3 (NumPy, SciPy, Matplotlib para co-localización), Editor Hexadecimal HxD, Osciloscopio Hantek"

repositorio:
  url: "No disponible / Repositorio institucional en proceso de creación"
  rama_base: "main"
---

---

## 1. Antecedentes y Justificación Técnica

Durante la ejecución de las fases preliminares de desarrollo y validación de la serie *Acelerógrafo V1.4* (documentadas en los informes técnicos institucionales precedentes):

1. **Infraestructura de Red y Sincronismo Físico Operativos:** 
   Se ensamblaron y caracterizaron experimentalmente tres placas de circuito impreso (1 Nodo Concentrador y 2 Nodos Sensores: A y B) comunicadas en topología en cascada (*Daisy Chain*) mediante cableado UTP bajo la norma T-568B. Se comprobó la transmisión estable del pulso de sincronización con retardos medidos en osciloscopio de apenas $1.2\,\mu\text{s}$ (Concentrador–Nodo) y $14.0\text{ ns}$ (entre Nodos Sensores A y B), así como el enlace de datos por RS485 a 2 Mbps tras corregir los bits de configuración del oscilador interno a *FRCPLL* (80 MHz).
2. **Limitación Crítica en Almacenamiento MicroSD:** 
   En la prueba de concepto anterior, se determinó que los zócalos para tarjetas MicroSD soldados físicamente en las placas carecían del contacto mecánico de detección de presencia (*card-detect*). A pesar de implementarse mitigaciones por software (`sdflags.detected = true`), la inicialización y escritura de sectores crudos no logró operar con la repetibilidad y determinismo requeridos para operación continua en campo. Asimismo, se identificaron particularidades de respuesta y tiempos de espera dispares entre tarjetas de diferentes fabricantes (Kingston vs. SanDisk ante el comando `CMD17`).
3. **Desacoplamiento del Alcance para el Cierre de Nodos Sensores:** 
   Con el propósito de consolidar unidades sensoras autónomas, fiables y completamente verificadas antes de abordar la etapa de interfaz de alto nivel con la Raspberry Pi a través del Concentrador, esta planificación enfoca todos los esfuerzos en el perfeccionamiento integral de los **Nodos Sensores**: retrabajo de hardware con nuevos zócalos con pin *card-detect*, almacenamiento robusto de sectores crudos de 512 bytes, arquitectura de doble búfer (*Ping-Pong*) en memoria RAM, integración de la librería del acelerómetro triaxial ADXL355 y validación experimental del sincronismo mediante un ensayo de co-localización con herramientas de análisis en Python.

---

## 2. Objetivos del Plan de Trabajo

### 2.1. Objetivo General
Implementar, validar y caracterizar experimentalmente el funcionamiento autónomo de dos Nodos Sensores para la red distribuida de monitorización de salud estructural (SHM), integrando un zócalo MicroSD con detección física de tarjeta, una arquitectura de doble búfer en RAM para almacenamiento continuo de sectores crudos de 512 bytes sin pérdida de muestras, la adquisición de datos triaxiales del acelerómetro ADXL355 y una suite de software en Python para el volcado y verificación del sincronismo temporal mediante pruebas de co-localización mecánica.

### 2.2. Objetivos Específicos
1. **[Retrabajo de Hardware y Perfil Eléctrico - 15 h]:** Ejecutar el retrabajo físico en las dos placas de Nodos Sensores para desoldar los zócalos defectuosos e instalar zócalos MicroSD con pin de detección mecánica (*card-detect*), caracterizando el perfil de consumo de corriente individual y en cadena Daisy Chain a 12V.
2. **[Controlador Robusto de MicroSD y HxD - 30 h]:** Desarrollar y verificar las rutinas de bajo nivel en MikroC para inicialización (conmutación SPI lenta/rápida condicionada por hardware), escritura y lectura determinística de sectores crudos de 512 bytes en tarjetas Kingston y SanDisk, comprobando la integridad física con el editor hexadecimal HxD.
3. **[Transmisión de Tiempo y Persistencia - 15 h]:** Sincronizar el reloj relativo de los Nodos Sensores recibiendo marcas de tiempo enviadas desde el Concentrador por el bus RS485 a 1 pps y persistir secuencialmente estas estampas temporales en la tarjeta MicroSD.
4. **[Arquitectura de Doble Búfer (Ping-Pong) - 30 h]:** Diseñar e implementar un esquema de doble búfer (*Buffer A / Buffer B* de 512 bytes) en la memoria RAM del dsPIC33EP256MC202 para garantizar la escritura en flash sin bloqueos ni pérdidas de datos ante interrupciones de sincronismo, validándolo inicialmente con tramas sintéticas continuas.
5. **[Integración del Acelerómetro ADXL355 - 24 h]:** Integrar el controlador SPI del acelerómetro triaxial ADXL355 en el firmware del nodo sensor, sustituyendo las tramas sintéticas por aceleraciones físicas reales ($X, Y, Z$) adquiridas en tiempo real bajo reposo y solicitaciones dinámicas.
6. **[Suite en Python y Ensayo de Co-localización - 18 h]:** Desarrollar una herramienta en Python para volcar sectores crudos desde lectores USB en PC y ejecutar un ensayo de co-localización física sometiendo ambos nodos a perturbaciones periódicas para certificar coherencia de fase y ausencia de deriva temporal.
7. **[Documentación Técnica y Troubleshooting - 12 h]:** Consolidar la guía de resolución de problemas (*troubleshooting*), compilar evidencias metrológicas y redactar el Informe Técnico Final de prácticas preprofesionales.

---

## 3. Plan de Trabajo Detallado y Checkpoints Cuantificables

### Fase 1: Adecuación de Hardware, Retrabajo de Zócalos y Caracterización Eléctrica
* **Duración:** 15 horas (Semana 1)
* **Objetivo:** Eliminar la causa raíz del bloqueo de hardware instalando zócalos MicroSD con detección mecánica y determinar el perfil de consumo eléctrico del sistema.
* **Actividades técnicas:**
  1. Desoldar con estación de retrabajo de aire caliente los zócalos MicroSD anteriores en las placas de los Nodos Sensores A y B.
  2. Limpiar pistas de cobre, montar y soldar manualmente los nuevos zócalos MicroSD que incorporan el contacto mecánico de detección de presencia de tarjeta (*card-detect*).
  3. Comprobación eléctrica con multímetro:
     - Verificar la conmutación limpia del estado lógico del pin *card-detect* al insertar y extraer físicamente la tarjeta de memoria.
     - Verificar continuidad y descartar puentes en las líneas de comunicación SPI1 (RB7 SCK, RB8 MOSI, RB9 MISO, RB0 CS).
  4. Caracterización de consumo de corriente:
     - Medir corriente estática y dinámica de cada placa de forma individual (Concentrador, Nodo A, Nodo B) en la línea de 12 V y en las salidas reguladas de 5 V y 3.3 V.
     - Medir el consumo total de la red completa interconectada en topología Daisy Chain.
  5. Cargar un firmware elemental de test en el dsPIC para conmutar un LED de estado en respuesta directa a la presencia física de la MicroSD.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 1.1:** Dos placas sensoras con el nuevo zócalo soldado, limpiado e inspeccionado; confirmación con multímetro de conmutación limpia del contacto *card-detect*.
  - [ ] **CP 1.2:** Matriz de caracterización eléctrica documentada con voltajes reales y corrientes medidas (mA) para Concentrador, Nodo A, Nodo B y consumo total a 12 V.
  - [ ] **CP 1.3:** Firmware de prueba ejecutándose en ambos nodos demostrando encendido/apagado de LED en respuesta directa a la presencia física de la MicroSD.

---

### Fase 2: Validación de Inicialización, Escritura y Lectura Cruda en MicroSD (Kingston y SanDisk)
* **Duración:** 30 horas (Semanas 2 y 3)
* **Objetivo:** Asegurar que el dsPIC33EP256MC202 realice operaciones de inicialización, escritura y lectura determinística de bloques físicos de 512 bytes por SPI en tarjetas de diferentes fabricantes, comprobando la integridad en HxD.
* **Actividades técnicas:**
  1. Actualizar la librería de bajo nivel en MikroC (`sdcard.c/.h`, `spiSD.c/.h`) para condicionar el arranque del protocolo al pin físico de *card-detect*.
  2. Calibración del reloj SPI: configurar `SPISD_Init(SLOW)` a frecuencia estandarizada ($100\text{ kHz} \le f_{init} \le 400\text{ kHz}$) durante el envío de comandos preliminares (`CMD0`, `CMD8`, `CMD58`, `CMD55`, `ACMD41`, `CMD16`), conmutando posteriormente a alta velocidad `SPISD_Init(FAST)` para la transferencia de datos.
  3. Implementar la rutina de prueba unitaria `Ejemplo_uso_SD`:
     - Escritura de un bloque patrón de 512 bytes en un sector de prueba predefinido (ej. sector 2500).
     - Lectura directa del mismo sector a memoria RAM y comparación de integridad byte a byte.
     - Activación de LED de éxito (verde) o parpadeo codificado de error (`LED_Error()`).
  4. Realizar pruebas comparativas de compatibilidad entre tarjetas:
     - Memorias Kingston (16 GB / 32 GB SDHC).
     - Memorias SanDisk (16 GB SDHC) — analizando y resolviendo el comportamiento de respuestas R1 ante `CMD17` y ciclos de reloj extra.
  5. Extracción de tarjetas y validación forense en PC mediante el editor hexadecimal HxD.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 2.1:** Código fuente en MikroC con inicialización robusta condicionada por hardware y conmutación de velocidades de bus SPI1.
  - [ ] **CP 2.2:** Test unitario en placa aprobado: verificación de coincidencia al 100% de datos escritos y leídos en RAM sin códigos de falla en los LEDs.
  - [ ] **CP 2.3:** Reporte con capturas en HxD que demuestren que el sector 2500 contiene la estructura de prueba exacta tanto en memoria Kingston como en SanDisk.

---

### Fase 3: Transmisión de Tiempo Concentrador–Nodos vía RS485 y Registro en MicroSD
* **Duración:** 15 horas (Semana 4)
* **Objetivo:** Establecer la difusión de marcas de tiempo desde el Concentrador por el bus de datos RS485, actualizando el reloj local de los nodos y registrando estos timestamps secuencialmente en la MicroSD.
* **Actividades técnicas:**
  1. Definir la trama de tiempo en RS485: `[Cabecera 0x3A, Dirección (broadcast o específica), Función_Tiempo, Timestamp uint32 (4 bytes), Checksum]`.
  2. Programar el firmware del Concentrador para generar y emitir periódicamente a 1 pps (pulso por segundo) la marca de tiempo calculada por temporizador interno a través del transceptor MAX485 a 2 Mbps.
  3. Programar en los Nodos Sensores la captura de la trama por interrupción del UART1 (`urx_1`), decodificando el timestamp y actualizando el contador local con cada pulso pps.
  4. Programar en el bucle del nodo sensor una rutina de persistencia: empaquetar un sector de 512 bytes conteniendo ID de nodo, timestamp recibido y contador de paquetes, escribiéndolo en la MicroSD (`sector_actual++`).
  5. Realizar ensayos de registro sostenido durante 10, 30 y 60 minutos en la red Daisy Chain.
  6. Extraer las tarjetas y analizar la progresión temporal de los sectores grabados en HxD.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 3.1:** Protocolo de trama de tiempo implementado; indicador LED en nodos confirmando recepción periódica de tramas de tiempo válidas.
  - [ ] **CP 3.2:** Registro continuo en MicroSD de al menos 100 sectores consecutivos con información de tiempo en ambos nodos sensores.
  - [ ] **CP 3.3:** Verificación en HxD del incremento temporal lineal y monótono en los sectores grabados, demostrando ausencia de saltos o pérdidas de tiempo.

---

### Fase 4: Arquitectura de Doble Búfer (Ping-Pong) y Trama Estructurada con Datos Sintéticos
* **Duración:** 30 horas (Semanas 5 y 6)
* **Objetivo:** Implementar en RAM el desacoplamiento entre la captura por interrupción de sincronismo (INT1) y la latencia de escritura de sectores en la MicroSD, estructurando paquetes estandarizados de 512 bytes con datos artificiales de aceleración.
* **Actividades técnicas:**
  1. Definir formalmente la trama binaria de 512 bytes por sector:
     - Cabecera fija (4 bytes: `0xAA, 0x55, 0xAA, 0x55`).
     - ID del Nodo Sensor (2 bytes: `0x0001` / `0x0002`).
     - Contador de sincronismo / Timestamp (4 bytes `uint32`).
     - Carga útil de aceleración sintética (498 bytes: patrón continuo $0 \dots 255$ repetido).
     - Checksum / CRC16 (2 bytes) y Marcador de fin de bloque (2 bytes: `0x55, 0xAA`).
  2. Desarrollar la arquitectura de **Doble Búfer (Ping-Pong Buffer)** en el dsPIC:
     - Reservar en RAM dos áreas independientes: `Buffer_A[512]` y `Buffer_B[512]`.
     - La interrupción externa de sincronismo (`INT1`) alimenta de forma determinística el búfer activo.
     - Al completarse el sector, se conmuta el puntero al búfer complementario y se levanta una bandera de escritura.
     - El bucle principal (`main`) detecta la bandera y ejecuta `SD_Write_Block()` en el búfer lleno, garantizando que el tiempo de escritura en la memoria no interfiera con la atención de nuevos pulsos de sincronismo.
  3. Medir con osciloscopio y temporizador el tiempo de ejecución de `SD_Write_Block()` frente a la tasa de llegada de muestras para verificar el margen de seguridad temporal.
  4. Realizar pruebas de estrés sometiendo a los nodos a trenes continuos de 500 a 1000 pulsos en red Daisy Chain.
  5. Inspeccionar en HxD la correlación y continuidad perfecta de los sectores grabados.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 4.1:** Controlador de Ping-Pong Buffer implementado en C con banderas de sincronización y protección contra desbordamiento (*buffer overflow*).
  - [ ] **CP 4.2:** Prueba de estrés superada: 500 sectores grabados en cada tarjeta MicroSD sin pérdidas de pulsos ni bloqueos del microcontrolador.
  - [ ] **CP 4.3:** Verificación en HxD de que la totalidad de los sectores almacenados conservan cabeceras íntegras, numeración secuencial de pulsos continua y el patrón incremental $0 \dots 255$ sin alteraciones.

---

### Fase 5: Integración del ADXL355, Doble Búfer y Almacenamiento Continuo de Datos Reales
* **Duración:** 24 horas (Semanas 7 y 8)
* **Objetivo:** Incorporar la librería existente del acelerómetro triaxial ADXL355, reemplazar los datos sintéticos de la trama por lecturas reales de aceleración ($X, Y, Z$) y registrar series de tiempo continuas en la MicroSD con el doble búfer.
* **Actividades técnicas:**
  1. Integrar los archivos fuente del controlador del ADXL355 en el proyecto MikroC de los nodos sensores; validar la inicialización del sensor por SPI, el rango de medición ($\pm 2g$) y los filtros de salida (ODR).
  2. Adaptar la carga útil de la trama de 512 bytes para empaquetar muestras acelerométricas reales en 20 bits para los ejes $X, Y, Z$, conservando cabecera, ID de nodo, timestamp y CRC.
  3. Sincronizar la lectura del sensor con el doble búfer: ante cada evento de muestreo/sincronismo, se adquiere la terna de aceleración y se almacena en el búfer activo de RAM. Al completarse los 512 bytes, el búfer conmuta y se dispara la escritura de sector crudo a la tarjeta MicroSD.
  4. Ejecución de ensayos de registro sostenido (15 a 30 minutos) en dos condiciones:
     - Reposo absoluto sobre mesa nivelada: verificar vector gravitatorio estático ($\approx 1g$ en el eje vertical $Z$ y $\approx 0g$ en $X, Y$).
     - Dinámica manual: aplicar inclinaciones y movimientos para verificar respuesta activa en los tres ejes.
  5. Extraer las tarjetas MicroSD y examinar en HxD los sectores para confirmar que los datos corresponden a amplitudes y variaciones coherentes con aceleración física.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 5.1:** Módulo ADXL355 plenamente integrado en el firmware de los nodos sensores sin conflictos de periféricos ni demoras en la ISR.
  - [ ] **CP 5.2:** Volcado e inspección en HxD de secuencias continuas de sectores: comprobación de valores estáticos de aceleración ($\approx 1g$ en eje Z) y patrones dinámicos en presencia de movimiento, con timestamps monótonos y sin corrupción de datos.

---

### Fase 6: Software en Python (Volcado y Graficación) y Validación Experimental por Co-localización
* **Duración:** 18 horas (Semanas 9 y 10)
* **Objetivo:** Desarrollar en PC una suite de scripts en Python para volcar y procesar los sectores crudos de las tarjetas MicroSD, y validar experimentalmente el sincronismo temporal y la ausencia de pérdida de muestras entre los Nodos A y B mediante un ensayo de co-localización física con perturbaciones periódicas.
* **Actividades técnicas:**
  1. **Desarrollo del script de volcado y extracción en Python:**
     - Leer la imagen binaria o sectores físicos de la tarjeta MicroSD mediante lector USB en PC.
     - Parsear bloques de 512 bytes: buscar cabeceras fijas (`0xAA 0x55 0xAA 0x55`), decodificar ID de nodo y timestamp, desempaquetar las muestras de 20 bits del ADXL355 usando `struct.unpack`, verificar CRC y exportar las series temporales ($t, a_x, a_y, a_z$) a archivos estructurados (.csv / .npy).
  2. **Desarrollo del script de graficación interactiva por intervalos:**
     - Herramienta en Python (`matplotlib` / `scipy`) que cargue en memoria los registros de ambos nodos (Nodo A y Nodo B).
     - Permitir al usuario seleccionar una ventana o intervalo temporal de visualización (ej. ventana de 30 segundos en cualquier punto del ensayo).
     - Graficar en subplots alineados temporalmente los canales de aceleración de ambos nodos para contrastar la coincidencia de fase y amplitud.
     - Opcional: cálculo numérico de la función de correlación cruzada (*cross-correlation*) para determinar cuantitativamente el retardo $\Delta t$ entre nodos.
  3. **Montaje y ejecución del ensayo de co-localización:**
     - Montar físicamente el Nodo Sensor A y el Nodo Sensor B sobre una misma placa rígida o superficie de prueba compartida, interconectados en Daisy Chain con el Concentrador.
     - Iniciar la adquisición continua en ambos nodos simultáneamente.
     - Aplicar perturbaciones mecánicas periódicas controladas (ej. impactos leves o toques cada 5 segundos) durante una sesión continua de 5 a 10 minutos.
  4. Extraer las tarjetas MicroSD, procesarlas con la suite en Python y evaluar la correlación temporal de las señales.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 6.1:** Scripts en Python concluidos y operativos: script de volcado de sectores a series temporales limpias y script gráfico que permita explorar intervalos temporales seleccionables.
  - [ ] **CP 6.2:** Gráficas de aceleración temporal generadas a partir del ensayo de co-localización que demuestren que los picos mecánicos periódicos coinciden de forma idéntica en el tiempo entre el Nodo A y el Nodo B, confirmando que la sincronización se mantiene estable sin desfasajes acumulativos ni pérdida de muestras.

---

### Fase 7: Documentación Técnica, Troubleshooting e Informe Final de Pasantías
* **Duración:** 12 horas (Semanas 10 y 11)
* **Objetivo:** Consolidar toda la información de ingeniería, estructurar la guía de solución de incidencias y redactar el Informe Técnico Final formal de la pasantía.
* **Actividades técnicas:**
  1. **Guía de Solución de Problemas (Troubleshooting):** Documentar problemas técnicos encontrados y soluciones aplicadas durante todo el proyecto (detección física vs. software en zócalos SD, temporización SPI SLOW/FAST, compatibilidad Kingston vs. SanDisk, control de desbordamiento de doble búfer y decodificación de tramas en Python).
  2. **Consolidación de evidencias experimentales:** Compilar tablas de consumo de potencia, registros fotográficos del retrabajo de hardware, oscilogramas de red, capturas de HxD y figuras de correlación temporal generadas en Python.
  3. **Redacción del Informe Técnico Final:** Redactar el documento final siguiendo la estructura formal de informe institucional (Resumen, Introducción, Arquitectura de Hardware/Firmware, Metodología, Resultados Experimentales, Discusión, Conclusiones y Recomendaciones).
  4. Revisión técnica con el tutor, incorporación de observaciones y preparación del repositorio en GitHub con código fuente comentado y limpio.
* **Checkpoints medibles para revisión:**
  - [ ] **CP 7.1:** Borrador consolidado del Informe Técnico Final con todos los anexos, figuras y guía de troubleshooting listo para revisión del tutor.
  - [ ] **CP 7.2:** Versión final del informe aprobada por el tutor, repositorio de código documentado y actas de culminación de prácticas debidamente suscritas.

---

## 4. Resumen de Fases y Cronograma Semanal (144 Horas)

El trabajo se divide en bloques semanales de **14 horas** (salvo las semanas quincenales o de cierre) para facilitar revisiones periódicas con checkpoints cuantificables:

| Fase | Semanas | Horas | Enfoque Principal |
| :---: | :---: | :---: | :--- |
| **Fase 1** | Semana 1 | **15 h** | Adecuación de hardware, zócalos con *card-detect* y consumo eléctrico. |
| **Fase 2** | Semanas 2–3 | **30 h** | Validación de inicialización, escritura/lectura cruda en SD y HxD. |
| **Fase 3** | Semana 4 | **15 h** | Transmisión de tiempo por RS485, registro en SD y HxD. |
| **Fase 4** | Semanas 5–6 | **30 h** | Doble búfer (Ping-Pong), trama de 512 B con datos sintéticos y HxD. |
| **Fase 5** | Semanas 7–8 | **24 h** | Integración del ADXL355, doble búfer con datos reales y guardado en SD. |
| **Fase 6** | Semanas 9–10 | **18 h** | Suite en Python (volcado/graficación) y ensayo de co-localización. |
| **Fase 7** | Semanas 10–11 | **12 h** | Guía de *troubleshooting*, anexos y redacción del Informe Técnico Final. |
| **Total** | **~10.5 Semanas** | **144 h** | **Planificación Global de Prácticas Preprofesionales** |

---

## 5. Entregables y Criterios de Éxito de la Pasantía

1. **Hardware de Nodos Sensores Optimizado:**
   * 2 placas de Nodos Sensores (A y B) con nuevos zócalos MicroSD con pin de detección física de tarjeta (*card-detect*) operativos.
   * Tabla de caracterización de consumo de corriente en régimen estático y dinámico.
2. **Firmware de Nodos Sensores en MikroC:**
   * Código fuente modular y debidamente comentado para el microcontrolador dsPIC33EP256MC202 conteniendo:
     * Controlador robusto de MicroSD por SPI (compatible con Kingston y SanDisk).
     * Receptor de marcas de tiempo y tramas RS485.
     * Módulo de Doble Búfer (Ping-Pong) en RAM.
     * Integración y lectura de aceleraciones triaxiales del ADXL355.
3. **Suite de Procesamiento en Python:**
   * Script de volcado directo de sectores crudos desde MicroSD a archivos binarios y CSV.
   * Script de graficación interactiva por intervalos de tiempo para inspección visual y análisis de sincronía entre nodos.
4. **Evidencia Experimental y Validación:**
   * Capturas del editor hexadecimal HxD certificando la integridad de las tramas de 512 bytes por sector.
   * Gráficas del ensayo de co-localización demostrando alineación temporal estricta de las ondas de aceleración ante impactos periódicos.
5. **Informe Técnico Final y Guía de Troubleshooting:**
   * Documento técnico formal de culminación de prácticas laborales, compilando la teoría, diseño, ejecución, resultados y guía detallada de resolución de fallas.
