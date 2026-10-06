// *****************************************************************************
// Interfaz del controlador SPI para la tarjeta SD.
// ******************************************************************************

#ifndef SPI_H
#define SPI_H

#define FAST    1
#define SLOW    0

// Funciones del controlador SPI.
void SPISD_Init(unsigned char speed);
unsigned char SPISD_Write(unsigned char datos);

#endif      // SPI_H datos);