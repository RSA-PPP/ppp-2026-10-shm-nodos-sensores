# Red de Monitorización de Salud Estructural (SHM)
## Validación Integral de Nodos Sensores (Serie Acelerógrafo V1.4)

Repositorio oficial del proyecto de prácticas preprofesionales para la adecuación de hardware, desarrollo de firmware en tiempo real, integración sensórica y validación experimental de los **Nodos Sensores** de la red distribuida de Monitorización de Salud Estructural (SHM) de la **Red Sísmica del Austro (RSA)**.

---

## 📋 Información Institucional del Proyecto

| Parámetro | Detalle |
| :--- | :--- |
| **Código del Proyecto** | `RSA-PPP-2026-10` |
| **Proyecto Institucional** | Validación Integral de Nodos Sensores para Red SHM (Serie Acelerógrafo V1.4) |
| **Área Temática** | Sistemas Embebidos, Instrumentación Sísmica, Firmware en Tiempo Real y Análisis en Python |
| **Estudiante / Pasante** | Geovanny Cullquicondo (`geovanny.cullquicondo@ucuenca.edu.ec`) |
| **Carrera / Institución** | Ingeniería en Telecomunicaciones — Universidad de Cuenca |
| **Tutor Institucional** | Ing. Milton Muñoz (`milton.munozc@ucuenca.edu.ec`) — Red Sísmica del Austro (RSA) |
| **Dedicación y Cronograma** | 144 horas totales (~10.5 semanas a 14 h/semana) \| Octubre 2026 – Diciembre 2026 |
| **Estado Actual** | **En Curso** |

---

## 🎯 Contexto y Antecedentes Técnicos

En las etapas preliminares de la serie *Acelerógrafo V1.4*, se ensambló y caracterizó experimentalmente una red distribuida compuesta por 1 Concentrador Principal y 2 Nodos Sensores (A y B) en topología en cascada (*Daisy Chain*) mediante cableado UTP T-568B:

* **Sincronismo Físico Operativo:** Se comprobó la transmisión estable del pulso de sincronización con retardos medidos en osciloscopio de $1.2\,\mu\text{s}$ (Concentrador–Nodo) y apenas $14.0\text{ ns}$ entre Nodos A y B, con enlace de datos RS485 a 2 Mbps tras fijar el oscilador a 80 MHz (*FRCPLL*).
* **Causa Raíz del Bloqueo Previo:** Los zócalos MicroSD soldados en las placas carecían del pin mecánico de detección de presencia de tarjeta (*card-detect*). Pese a mitigar el problema por software, la escritura de sectores crudos no logró determinismo continuo en campo, detectándose tiempos de espera dispares entre fabricantes (Kingston vs. SanDisk).
* **Alcance de este Proyecto:** Con el fin de asegurar unidades sensoras completamente autónomas y fiables antes de abordar la interfaz con Raspberry Pi a través del Concentrador, este proyecto se enfoca al 100% en:
  1. **Retrabajo de Hardware:** Sustitución física de los zócalos por componentes con contacto de *card-detect*.
  2. **Almacenamiento Determinista:** Controlador SPI en MikroC con conmutación de velocidad (*Slow/Fast*) para escritura de bloques de 512 bytes verificados con HxD.
  3. **Arquitectura de Doble Búfer (Ping-Pong):** Desacoplamiento temporal en memoria RAM (Buffer A y Buffer B de 512 B) para garantizar que las interrupciones de sincronismo (`INT1`) no se pierdan durante la latencia de escritura en flash.
  4. **Integración Sensórica:** Integración del acelerómetro triaxial de precisión **ADXL355** sustituyendo tramas sintéticas por datos físicos reales.
  5. **Suite en Python y Ensayo de Co-localización:** Scripts para volcado directo de sectores USB y graficación temporal por intervalos tras someter a ambos nodos a perturbaciones periódicas sobre una misma base rígida.

> 📖 **Guía de Referencia y Transición:** Para consultar el mapeo de archivos, lecciones aprendidas y código del proyecto anterior de David Timbi (`RSA-PPP-2026-08`), revisa el documento [docs/referencia_proyecto_anterior.md](docs/referencia_proyecto_anterior.md).

---

## 🏗️ Arquitectura de la Red y Flujo de Datos

```text
               +-------------------------------------------+
               |         NODO CONCENTRADOR (dsPIC)         |
               +-------------------------------------------+
                     | Pulso Sincronismo (MAX483)  | RS485 Datos 2 Mbps (MAX485)
                     |                             |
     ================= CABLE UTP (Norma T-568B) =================
     |                                                          |
     v                                                          v
+--------------------------+               +--------------------------+
|   NODO SENSOR A (ID:1)   |               |   NODO SENSOR B (ID:2)   |
|   dsPIC33EP256MC202      |               |   dsPIC33EP256MC202      |
|--------------------------|               |--------------------------|
| • INT1: Pulso Sincronismo|               | • INT1: Pulso Sincronismo|
| • UART1: RS485 Timestamp |======14 ns===>| • UART1: RS485 Timestamp |
| • SPI: ADXL355 Triaxial  |  (Daisy Chain)| • SPI: ADXL355 Triaxial  |
| • Doble Búfer Ping-Pong  |               | • Doble Búfer Ping-Pong  |
| • MicroSD (Card-Detect)  |               | • MicroSD (Card-Detect)  |
+--------------------------+               +--------------------------+
             |                                          |
             +--------------------+---------------------+
                                  |
                                  v
                     [Extracción de Tarjetas SD]
                                  |
                                  v
                     [Suite de Análisis en PC]
                     • python/scripts/dump_sd.py
                     • python/scripts/plot_colocalizacion.py
```

### Arquitectura de Doble Búfer en RAM (*Ping-Pong Buffer*)

```text
[Muestreo / INT1 Sincronismo]
             |
             v
      +--------------+
      | Búfer Activo | ----> Se llena muestra a muestra en RAM (512 bytes)
      +--------------+
             | (Al completar 512 bytes: Conmuta puntero y activa bandera)
             v
      +------------------+
      | Búfer a Grabar   | ----> SD_Write_Block() escribe a MicroSD en el main()
      +------------------+       sin bloquear nuevas interrupciones de muestreo
```

### Formato de la Trama Binaria de Sector (512 Bytes)

| Campo | Tamaño | Descripción |
| :--- | :---: | :--- |
| **Cabecera fija** | 4 bytes | Identificador de inicio de bloque (`0xAA 0x55 0xAA 0x55`) |
| **ID del Nodo** | 2 bytes | Dirección física del sensor (`0x0001` / `0x0002`) |
| **Contador / Timestamp** | 4 bytes | Estampa de tiempo `uint32` sincronizada vía RS485 |
| **Carga Útil (Payload)** | 498 bytes | Muestras acelerométricas del ADXL355 (o patrón sintético $0 \dots 255$ en Fase 4) |
| **Checksum / CRC16** | 2 bytes | Suma de comprobación de integridad del bloque |
| **Marcador de Fin** | 2 bytes | Delimitador de cierre (`0x55 0xAA`) |

---

## 📂 Estructura del Repositorio

```text
ppp-2026-10-shm-nodos-sensores/
├── data/
│   ├── hxd_evidence/          # Capturas de inspección hexadecimal forense con HxD
│   └── raw_samples/           # Volcados binarios crudos (.bin, .raw) de prueba
│
├── docs/
│   ├── hardware/              # Reportes de retrabajo, pinout, diagramas y consumo eléctrico
│   ├── troubleshooting/       # Bitácora de incidencias técnicas encontradas y soluciones
│   ├── referencia_proyecto_anterior.md # Guía puente del proyecto previo (David Timbi)
│   └── planificacion.md       # Documento oficial del plan de trabajo de 144 horas
│
├── firmware/
│   ├── common/                # Definiciones de registros, tramas y estructuras globales
│   ├── drivers/               # Controladores de periféricos (adxl355.c/.h, sdcard.c/.h, spiSD.c/.h)
│   ├── nodo_sensor/           # Proyecto principal MikroC para los Nodos Sensores A y B
│   └── tests/                 # Firmwares de prueba unitaria (test LED card-detect, test SD cruda)
│
├── python/
│   ├── scripts/               # Scripts de volcado (dump_sd.py) y graficación (plot_colocalizacion.py)
│   ├── utils/                 # Módulos de desempaquetado binario (struct) y verificación de CRC
│   └── requirements.txt       # Librerías necesarias (numpy, scipy, matplotlib)
│
├── .gitignore                 # Filtro de artefactos temporales de MikroC y Python
└── README.md                  # Este documento
```

---

## 📅 Cronograma de Trabajo y Checkpoints Cuantificables (144 Horas)

El trabajo comprende **144 horas** distribuidas en bloques semanales de **14 horas** (~10.5 semanas):

| Fase | Semanas | Horas | Enfoque Principal | Checkpoints de Revisión |
| :---: | :---: | :---: | :--- | :--- |
| **Fase 1** | Sem 1 | **15 h** | Adecuación de hardware, retrabajo de zócalos con *card-detect* y consumo. | **CP 1.1:** Zócalos soldados y probados con multímetro.<br>**CP 1.2:** Matriz de consumo eléctrico a 12 V.<br>**CP 1.3:** Test LED respondiendo a presencia de SD. |
| **Fase 2** | Sem 2–3 | **30 h** | Inicialización robusta, conmutación SPI Slow/Fast y escritura/lectura en HxD. | **CP 2.1:** Driver SPI condicionado a *card-detect*.<br>**CP 2.2:** Test unitario en RAM sin errores.<br>**CP 2.3:** Sector 2500 verificado en HxD en Kingston y SanDisk. |
| **Fase 3** | Sem 4 | **15 h** | Transmisión de tiempo por RS485 a 1 pps y persistencia en MicroSD. | **CP 3.1:** Recepción de timestamp por UART1 confirmada.<br>**CP 3.2:** Registro de 100 sectores consecutivos.<br>**CP 3.3:** Verificación en HxD de incremento temporal monótono. |
| **Fase 4** | Sem 5–6 | **30 h** | Doble búfer (*Ping-Pong*), trama estructurada de 512 B con datos sintéticos. | **CP 4.1:** Controlador Ping-Pong Buffer con banderas y protección.<br>**CP 4.2:** Prueba de estrés de 500 sectores sin desbordamiento.<br>**CP 4.3:** Continuidad secuencial $0\dots 255$ verificada en HxD. |
| **Fase 5** | Sem 7–8 | **24 h** | Integración del ADXL355 por SPI, doble búfer con aceleraciones reales. | **CP 5.1:** Controlador ADXL355 operando a $\pm 2g$ sin conflictos.<br>**CP 5.2:** Volcado e inspección en HxD de datos estáticos ($1g$ en Z) y dinámicos. |
| **Fase 6** | Sem 9–10 | **18 h** | Suite en Python (volcado/graficación) y ensayo experimental de co-localización. | **CP 6.1:** Scripts en Python operativos (extracción y visor temporal).<br>**CP 6.2:** Gráficas de co-localización demostrando alineación estricta de fase ante impactos periódicos. |
| **Fase 7** | Sem 10–11 | **12 h** | Guía de *troubleshooting*, consolidación de anexos y redacción del Informe Final. | **CP 7.1:** Borrador consolidado de informe y anexos para revisión.<br>**CP 7.2:** Informe aprobado por tutor y actas suscritas. |
| **Total** | **~10.5 Sem** | **144 h** | **Planificación Global de Prácticas Preprofesionales** | |

---

## 🛠️ Tecnologías y Entorno de Desarrollo

* **Microcontrolador:** Microchip dsPIC33EP256MC202 (16 bits, 80 MHz con oscilador interno FRCPLL, 32 KB SRAM, encapsulado SPDIP-28 / SOIC-28).
* **Compilador e IDE:** MikroC PRO for dsPIC v6.2.0+ y MPLAB IPE v6.20 con programador hardware PICkit 3.
* **Transductor:** Acelerómetro triaxial MEMS de ultra bajo ruido Analog Devices **ADXL355** ($\pm 2g$, 20 bits de resolución).
* **Buses y Transceptores:** RS485 diferencial a 2 Mbps (MAX485), bus SPI de alta velocidad para memoria y sensor.
* **Memorias Evaluadas:** Tarjetas MicroSDHC Kingston y SanDisk (16 GB / 32 GB, Clase 10).
* **Entorno de Análisis en PC:** Python 3.10+ (`numpy`, `scipy`, `matplotlib`) y Editor Hexadecimal HxD.

---

## 🚀 Flujo de Trabajo en Git

1. **Trabajo en Rama Principal:** Todo el avance se integra directamente en la rama `main` de este repositorio.
2. **Formato de Commits:** Utilizar mensajes atómicos y descriptivos en minúsculas siguiendo la convención:
   * `feat: ...` (nuevos controladores, funciones o scripts)
   * `fix: ...` (corrección de errores de firmware o cableado)
   * `docs: ...` (actualización de diagramas, bitácoras o informes)
   * `test: ...` (pruebas de banco, capturas HxD o scripts de ensayo)
3. **Control de Hitos por Git Tags:** Al culminar cada una de las 7 fases del cronograma, el estudiante creará una etiqueta fija:
   ```bash
   git tag -a v0.1.0-fase1 -m "Fase 1 completada: Retrabajo de zócalos y consumo caracterizado"
   git push origin v0.1.0-fase1
   ```

---
*Red Sísmica del Austro (RSA) — Laboratorio de Instrumentación Sísmica y Salud Estructural (2026)*
