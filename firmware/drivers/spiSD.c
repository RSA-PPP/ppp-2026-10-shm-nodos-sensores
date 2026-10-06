
// *****************************************************************************
// Controlador SPI utilizado por la tarjeta SD.
// *****************************************************************************

// Interfaz del controlador.
#include "spiSD.h"

// *****************************************************************************
// Configura SPI1 en modo maestro con la velocidad indicada.
// *****************************************************************************
void SPISD_Init(unsigned char speed) {
    SPI1STAT.SPIEN = 0;              // Deshabilita SPI1 durante la configuración.

    // Configuración para un reloj de 80 MHz (40 MIPS).
    if(speed == FAST) {
             // 2.5 MHz.
        SPI1_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, _SPI_PRESCALE_SEC_4, _SPI_PRESCALE_PRI_64, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);
            // 3.125 MHz.
        //SPI1_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, _SPI_PRESCALE_SEC_5, _SPI_PRESCALE_PRI_64, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);
            // 5 MHz.
        // SPI1_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, _SPI_PRESCALE_SEC_2, _SPI_PRESCALE_PRI_16, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);
            // 10 MHz.
        //SPI1_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, _SPI_PRESCALE_SEC_1, _SPI_PRESCALE_PRI_4, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);
        } else {
        // 625 kHz durante la inicialización de la tarjeta.
        SPI1_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, _SPI_PRESCALE_SEC_1, _SPI_PRESCALE_PRI_64, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);

    }
    SPI1STAT.SPIEN = 1;    // Habilita SPI1.
}
// *****************************************************************************
// **************************** Fin SPI_Init ***********************************
// *****************************************************************************


// *****************************************************************************
// Transmite un byte y devuelve el byte recibido simultáneamente.
// *****************************************************************************
unsigned char SPISD_Write(unsigned char datos) {
    SPI1BUF = datos;
    while(SPI1STATbits.SPITBF);
    while(SPI1STATbits.SPIRBF == 0);
    return SPI1BUF;
}
// *****************************************************************************
// **************************** Fin SPI_Write **********************************
// *****************************************************************************