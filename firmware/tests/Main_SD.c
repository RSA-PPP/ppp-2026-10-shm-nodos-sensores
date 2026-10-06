#include <spiSD.h>
#include <sdcard.h>
#include <stdbool.h>
#include <stdlib.h> // Funciones auxiliares de la biblioteca estándar.

// RP41 = RB9: entrada SDI1 de SPI1.
#define RPINR20 (*((volatile unsigned int*)0x06C8))
// Parámetros de la aplicación.
#define IDNODO 1                                                                // Dirección del nodo.
#define SIZESD 16                                                               // Capacidad de la SD en GB.
#define DELTASECTOR 97952                                                       // Desplazamiento desde el primer sector físico.

// Constantes de configuración.
#define SD_DETECCION_HARDWARE 0                                                 // El contacto 9 no está soldado.
#define FP 80000000                                                             // Frecuencia del reloj.

// Índices de trabajo.
unsigned int i, j, x, y;

// Definición de pines y estado de la tarjeta.
struct sdflags sdflags;
sbit TEST at LATA2_bit;                                                         // LED de prueba.
sbit TEST_Direction at TRISA2_bit;
sbit CsADXL at LATA3_bit;                                                       // Chip select del acelerómetro.
sbit CsADXL_Direction at TRISA3_bit;
sbit sd_CS_lat at LATB0_bit;                                                    // Chip select de la SD.
sbit sd_CS_tris at TRISB0_bit;
sbit sd_detect_port at LATA4_bit;                                               // Entrada de detección; actualmente no disponible.
sbit sd_detect_tris at TRISA4_bit;
sbit MSRS485 at LATB12_bit;                                                     // Selección de modo del transceptor RS485.
sbit MSRS485_Direction at TRISB12_bit;

// Estado general del sistema.
unsigned short inicioSistema;
unsigned long horaSistema, fechaSistema;

// Variables para el manejo de sectores de la SD.
unsigned long PSF;                                                              // Primer sector físico.
unsigned long PSE;                                                              // Primer sector reservado para escritura.
unsigned long USF;                                                              // Último sector físico.
unsigned long PSEC;                                                             // Primer sector escrito en el ciclo actual.
unsigned long sectorSD;                                                         // Sector que se escribirá.
unsigned long sectorLec;                                                        // Sector que se leerá.
const unsigned int clusterSizeSD = 512;                                         // Tamaño del sector en bytes.
unsigned long infoPrimerSector;                                                 // Sector que almacena el primer sector escrito.
unsigned long infoUltimoSector;                                                 // Sector que almacena el último sector escrito.
unsigned char cabeceraSD[6] = {255, 253, 251, 10, 0, 250};                      // Cabecera: constantes, tamaño de muestra y frecuencia.
unsigned char bufferSD [clusterSizeSD];                                         // Buffer de un sector.
unsigned char checkEscSD;                                                       // Resultado de la última escritura.
unsigned char checkLecSD;
unsigned short banInsSec;                                                       // Habilita la inspección de un sector.

// Prototipos de la aplicación.
void ConfiguracionPrincipal();
void ConfigurarPPS_SPI1();
void GuardarTramaSD();
void GuardarBufferSD(unsigned char* bufferLleno, unsigned long sector);
void GuardarInfoSector(unsigned long datoSector, unsigned long localizacionSector);

void Ejemplo_uso_SD();
void LED(int veces, unsigned char tiempo_seg);
void LED_Error(unsigned char codigo);

void main() {
     ConfiguracionPrincipal();
     TEST = 0;
    TEST = 1;
    Delay_ms(1000);
    TEST = 0;
    // Inicializa los índices de trabajo.
     i = 0;
     j = 0;
     x = 0;
     y = 0;

    // Inicializa el estado del sistema.
     inicioSistema = 0;

    // Inicializa las variables de tiempo.
     horaSistema = 0;
     fechaSistema = 0;

    // Inicializa el estado de la SD.
     PSEC = 0;
     sectorSD = 0;
     sectorLec = 0;
     checkEscSD = 0;
     checkLecSD = 0;
    MSRS485 = 0;                                                               // Establece el transceptor RS485 en modo lectura.
     banInsSec = 0;

    // Determina el último sector físico según la capacidad configurada.
     switch (SIZESD){
            case 2:
                    PSF = 2048;
                    USF = 3911679;
                    break;
            case 4:
                    PSF = 2048;
                    USF = 7772160;
                    break;
            case 8:
                    PSF = 2048;
                    //USF = 15265792;
                    USF = 16779263;
                    break;
            case 16:
                    PSF = 2048;
                    USF = 31115263;
                    break;
     }
    infoPrimerSector = PSF+DELTASECTOR-2;                                      // Sector para el primer sector escrito.
    infoUltimoSector = PSF+DELTASECTOR-1;                                      // Sector para el último sector escrito.
     PSE = PSF+DELTASECTOR;

    // Datos de tiempo para la prueba.
    horaSistema = 86100;        // 23:55:00.
    fechaSistema = 200228;      // 20/02/28.

      #if SD_DETECCION_HARDWARE
    // Comprueba la presencia de la SD mediante el contacto de detección.
      while (1) {
              if (SD_Detect() == DETECTED) {
                  sdflags.detected = true;
                  break;
              } else {
                  sdflags.detected = false;
                  sdflags.init_ok = false;
              }
              Delay_ms(100);
      }
      #else
    // El zócalo no tiene soldado el contacto de detección; se da por válida
    // la presencia de la tarjeta para probar la comunicación SPI.
      sdflags.detected = true;
      #endif

    // Inicializa la SD.
    if (sdflags.detected && !sdflags.init_ok) {
        checkEscSD = SD_Init_Try(10);
        if (checkEscSD == SUCCESSFUL_INIT) {
              sdflags.init_ok = true;
              inicioSistema = 1;                                                // Permite iniciar el sistema.
              TEST = 1;
           } else {
              sdflags.init_ok = false;
              INT1IE_bit = 0;                                                   // Deshabilita la interrupción externa INT1.
              U1MODE.UARTEN = 0;                                                // Deshabilita el UART.
              inicioSistema = 0;                                                // Impide iniciar el sistema.
                    LED_Error(checkEscSD);
           }
     }
     //Delay_ms(2000);

     Ejemplo_uso_SD();

     //GuardarTramaSD();
}


// Rutinas de prueba y almacenamiento.
void Ejemplo_uso_SD(){
    /* Escribe un sector, lo lee y copia el resultado en el sector siguiente. */
    unsigned char data_to_write[512]; // Datos que se escribirán.
    unsigned long sector;             // Sector de lectura y escritura.
    unsigned char valor;              // Valor que se escribirá.
    unsigned char buffer[512];        // Datos leídos.

    // Llena el buffer con la secuencia 1..255 repetida.
    valor = 1;
     for (i = 0; i < 512; i++) {
         data_to_write[i] = valor;
         valor++;
         // Reinicia la secuencia después del valor 255.
         if (valor == 256) {
             valor = 1;
         }
     }

    // Escribe el buffer en un sector de prueba.
    sector = 2500;
     checkEscSD = SD_Write_Block(data_to_write,sector);
     if (checkEscSD == DATA_ACCEPTED){
         LED(1,1);
     } else {
         LED_Error(10);
     }

    // Lee el sector escrito.
     checkLecSD = SD_Read_Block(buffer, sector);
     if (checkLecSD==0) {
        LED(1,1);
      } else {
            LED_Error(11);
     }

    // Copia los datos leídos en el sector siguiente para comprobar la lectura.
     //sector=40001;
     checkEscSD = SD_Write_Block(buffer,sector+1);
     if (checkEscSD == DATA_ACCEPTED){
        LED(1,1);
      } else {
            LED_Error(12);
     }

}

// Genera parpadeos con intervalos de uno o dos segundos.
void LED(int veces, unsigned char tiempo_seg){
    /* tiempo_seg admite los valores 1 y 2. */
     unsigned int i;
         TEST_Direction = 0;
     if (tiempo_seg==1){
       for (i=0; i<veces; i++){
                     TEST = 1;
           Delay_ms(1000);
                     TEST = 0;
           Delay_ms(1000);
       }
     }else if (tiempo_seg==2){
       for (i=0; i<veces; i++){
                     TEST = 1;
           Delay_ms(2000);
                     TEST = 0;
           Delay_ms(2000);
       }
     }
}

// Indica un código de error mediante parpadeos cortos y una pausa larga.
void LED_Error(unsigned char codigo){
    unsigned char parpadeos;
    parpadeos = codigo;
    if (parpadeos > 9) parpadeos = parpadeos - 9;
    while (1){
        for (i = 0; i < parpadeos; i++){
            TEST = 1;
            Delay_ms(500);
            TEST = 0;
            Delay_ms(500);
        }
        Delay_ms(3000);
    }
}



void ConfiguracionPrincipal(){
    Delay_ms(4000);   // Tiempo de estabilización inicial.

    // Configura los puertos y el bus SPI.
    ANSELA = 0;
    ANSELB = 0;
    ConfigurarPPS_SPI1();
    TEST_Direction = 0;                                                        // LED como salida.
    TEST = 0;
    sd_CS_tris = 0;                                                            // Chip select de la SD como salida.
    sd_detect_tris = 1;                                                        // Entrada de detección.

    // Limpia las banderas de estado de la SD.
     sdflags.detected = false;
     sdflags.init_ok = false;
     sdflags.saving = false;

    Delay_ms(200);                                                             // Espera de estabilización.
}

// Asigna SCK1OUT a RB7, SDO1/MOSI a RB8 y SDI1/MISO a RB9.
void ConfigurarPPS_SPI1(){
    asm {
        MOV #0x46, W0
        MOV #0x57, W1
        MOV #0x0742, W2
        MOV.b W0, [W2]
        MOV.b W1, [W2]
        BCLR OSCCON, #6
    }

    RPOR2 = (RPOR2 & 0x00FF) | 0x0600;
    RPOR3 = (RPOR3 & 0xFF00) | 0x0005;
    RPINR20 = (RPINR20 & 0xFF00) | 41;

    TRISBbits.TRISB7 = 0;
    TRISBbits.TRISB8 = 0;
    TRISBbits.TRISB9 = 1;
}

// Escribe un buffer de 512 bytes, con hasta cinco intentos.
void GuardarBufferSD(unsigned char* bufferLleno, unsigned long sector){
    // Reintenta la escritura si la tarjeta la rechaza.
     for (x=0;x<5;x++){
         checkEscSD = SD_Write_Block(bufferLleno,sector);
         if (checkEscSD == DATA_ACCEPTED){
             break;
         }
         Delay_us(10);
     }
}

// Guarda una trama de aceleración distribuida en cinco sectores.
void GuardarTramaSD(){
    // La trama ocupa 2512 bytes: cuatro sectores completos y 464 bytes
    // del quinto sector, cuyo resto se rellena con ceros.
        unsigned char bufferSD[512];

        sectorSD=40000;
        for (sectorSD=40000; sectorSD<40020;sectorSD++){
            // Reserva espacio para la cabecera.
            for (x=0;x<6;x++){
                bufferSD[x] = 1;
            }
            // Reserva espacio para el tiempo.
            for (x=0;x<6;x++){
                bufferSD[6+x] = 2;
            }
            // Reserva los primeros 500 bytes de aceleración.
            for (x=0;x<500;x++){
                bufferSD[12+x] = 3;
            }

            // Guarda el primer sector.
            GuardarBufferSD(bufferSD, sectorSD);
            // Avanza al sector siguiente.
            sectorSD++;


            // Prepara el segundo sector de datos.
            for (x=0;x<512;x++){
                bufferSD[x] = 4;
            }
            GuardarBufferSD(bufferSD, sectorSD);
            sectorSD++;

            // Prepara el tercer sector de datos.
            for (x=0;x<512;x++){
                bufferSD[x] = 5;
            }
            GuardarBufferSD(bufferSD, sectorSD);
            sectorSD++;

            // Prepara el cuarto sector de datos.
            for (x=0;x<512;x++){
                bufferSD[x] = 6;
            }
            GuardarBufferSD(bufferSD, sectorSD);
            sectorSD++;

            // Prepara el quinto sector y rellena el espacio restante.
            for (x=0;x<512;x++){
                if (x<464){
                   bufferSD[x] = 7;
                } else {
                   bufferSD[x] = 0;
                }
            }
            // La escritura del quinto sector permanece deshabilitada durante la prueba.
            sectorSD++;

            // Indica que la trama se procesó correctamente.
            LED(3,1);
        }

        // Guarda la posición del último sector escrito cada cinco minutos.
        //if (horaSistema%300==0){
        //   GuardarInfoSector(sectorSD, infoUltimoSector);
        //}

        //TEST = 0;                                                               //Apaga el TEST cuando termina de gurdar la trama

}

// Guarda un número de sector en formato big-endian dentro de un sector.
void GuardarInfoSector(unsigned long datoSector, unsigned long localizacionSector){

    // La biblioteca escribe sectores completos, por lo que se rellena un buffer.
     unsigned char bufferSectores[512];
    bufferSectores[0] = (datoSector>>24)&0xFF;                                     // Byte más significativo.
     bufferSectores[1] = (datoSector>>16)&0xFF;
     bufferSectores[2] = (datoSector>>8)&0xFF;
    bufferSectores[3] = (datoSector)&0xFF;                                         // Byte menos significativo.
     for (x=4;x<512;x++){
         bufferSectores[x] = 0;                                                 // Rellena el resto con ceros.
     }

    // Reintenta la escritura hasta cinco veces.
     for (x=0;x<5;x++){
         checkEscSD = SD_Write_Block(bufferSectores,localizacionSector);
         if (checkEscSD == DATA_ACCEPTED){
             //TEST = ~TEST;
             break;
         }
         Delay_us(10);
     }

}