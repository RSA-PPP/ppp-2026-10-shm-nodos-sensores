
// Interfaz del controlador de la tarjeta SD.

// Tipos booleanos usados por la aplicación.
#include <stdbool.h>

#ifndef SD_H
#define SD_H

// *****************************************************************************
// ********************** Definicion de estructuras ****************************
// *****************************************************************************
struct sdflags {
    unsigned char init_ok:1;
    unsigned char detected:1;
    unsigned char saving:1;
};


//#define spi_lat_CS  LATF
//#define spi_tris_CS TRISF
//#define spi_port_CS PORTF
//#define spi_bit_CS  0x01

// *****************************************************************************
// ******************** Comandos para la tarjeta SD ****************************
// *****************************************************************************
#define GO_IDLE_STATE       0x00        // CMD0
#define SEND_OP_COND        0x01        // CMD1
#define SEND_IF_COND        0x08        // CMD8
#define SEND_CSD            0x09        // CMD9
#define SEND_CID            0x0A        // CMD10
#define SEND_STATUS         0x0D        // CMD13
#define SET_BLOCKLEN        0x10        // CMD16
#define READ_SINGLE_BLOCK   0x11        // CMD17
#define WRITE_BLOCK         0x18        // CMD24
#define APP_CMD             0x37        // CMD55
#define READ_OCR            0x3A        // CMD58
#define CRC_ON_OFF          0x3B        // CMD59
#define SD_STATUS           0x0D        // ACMD13
#define SD_SEND_OP_COND     0x29        // ACMD41
#define SET_CLR_CARD_DETECT 0x2A        // ACMD42
#define SEND_SCR            0x33        // ACMD51

// *****************************************************************************
// ************* Comandos de respuesta para la tarjeta SD **********************
// *****************************************************************************
// R1 response (8bits)
#define IDLE_STATE              0
#define ERASE_RESET             1
#define ILLEGAL_COMMAND         2
#define COM_CRC_ERROR           3
#define ERASE_SEQUENCE_ERROR    4
#define ADDRESS_ERROR           5
#define PARAMETER_ERROR         6
// First 8bits of R2 response
#define CARD_IS_LOCKED          8
#define WP_ERASE_SKIP           9
#define ERROR                   10
#define CC_ERROR                11
#define CARD_ECC_FAILED         12
#define WP_VIOLATION            13
#define ERASE_PARAM             14
#define OUT_OF_RANGE            15
/* Estados de inicialización y transferencia usados por este controlador. */
#define CARD_NOT_INSERTED       16
#define SD_NOT_READY            17
#define UNUSABLE_CARD           18
#define ECHO_BACK_ERROR         19
#define INCOMPATIBLE_VOLTAGE    20
#define TOKEN_NOT_RECEIVED      21
#define DATA_ACCEPTED           22
#define DATA_REJECTED_CRC_ERROR 23
#define DATA_REJECTED_WR_ERROR  24
#define SUCCESSFUL_INIT         0xAA

// OCR: primeros 32 bits de la respuesta R3.
#define VOLTAGE_RANGE_MASK      0x00FF8000
#define POWER_UP_STATUS         31
#define CAPACITY_STATUS         30

// R7: campos de eco y tensión aceptada.
#define ECHO_BACK_MASK          0x000000FF
#define VOLTAGE_ACCEPTED_MASK   0x00000F00

#define SD_TIME_OUT             2000

#define DETECTED                0xDE

// *****************************************************************************
// Funciones de inicialización, lectura y escritura.
// *****************************************************************************
unsigned char SD_Init(void);
unsigned char SD_Init_Try(unsigned char);
unsigned char SD_Write_Block(unsigned char*,unsigned long);
unsigned char SD_Read_Block(unsigned char*,unsigned long);
unsigned char SD_Read(unsigned char*,unsigned int);
void SD_Send_Command(unsigned char, unsigned long, unsigned char);
unsigned char R1_Response(void);
unsigned int R2_Response(void);
unsigned long Response_32b(void);
unsigned char SD_Ready(void);
void Select_SD(void);
void Release_SD(void);
bool Detect_SD (void);
unsigned char SD_Detect(void);
void SD_Check(void);

#endif      // SD_H