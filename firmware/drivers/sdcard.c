
// *****************************************************************************
// Controlador de una tarjeta SD mediante SPI.
// Las operaciones de lectura y escritura trabajan con sectores completos de
// 512 bytes y acceden directamente a la dirección física de cada sector.

// *****************************************************************************

// Dependencias del controlador.
#include "spiSD.h"
#include "sdcard.h"
#include <stdbool.h>

// *****************************************************************************
// Señales y estado compartidos con la aplicación.
// *****************************************************************************
// Señales de chip select definidas por la aplicación.
extern sfr sbit sd_CS_lat; // Chip select, activo en bajo.

extern sfr sbit sd_CS_tris; // Dirección del chip select: 0 salida, 1 entrada.

// Pin de detección de la tarjeta.
extern sfr sbit sd_detect_port;
extern sfr sbit sd_detect_tris;

// Estado de la tarjeta, definido e inicializado por la aplicación.
extern struct sdflags sdflags;

// Card Capacity Status: 0x02 para SDSC y 0x03 para SDHC/SDXC.
unsigned char ccs;

// Genera parpadeos de diagnóstico en el LED de la tarjeta.
void LEDcard(int veces, unsigned char tiempo_seg){
     unsigned int i;
    TRISAbits.TRISA1 = 0;
     if (tiempo_seg==1){
       for (i=0; i<veces; i++){
           LATAbits.LATA1 = 1;
           Delay_ms(1000);
           LATAbits.LATA1 = 0;
           Delay_ms(1000);
       }
     }else if (tiempo_seg==2){
       for (i=0; i<veces; i++){
           LATAbits.LATA1 = 1;
           Delay_ms(350);
           LATAbits.LATA1 = 0;
           Delay_ms(350);
       }
     }
}

// *****************************************************************************
// Lee nbytes después de recibir el token de inicio de datos.
// *****************************************************************************
unsigned char SD_Read(unsigned char *Buffer, unsigned int nbytes){
    unsigned int i;
    unsigned char temp;
    for(i = 0; i < SD_TIME_OUT; i++){
        temp = SPISD_Write(0xFF);
        if(temp == 0xFE) break;
        if(i == SD_TIME_OUT-1) return TOKEN_NOT_RECEIVED;
    }
    for(i = 0; i < nbytes; i++){
        Buffer[i] = SPISD_Write(0xFF);
    }
    temp = SPISD_Write(0xFF);     // Descarta el primer byte de CRC.
    temp = SPISD_Write(0xFF);     // Descarta el segundo byte de CRC.
    return 0x00;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Lee un sector completo de 512 bytes.
// *****************************************************************************
unsigned char SD_Read_Block(unsigned char *Buffer, unsigned long Address){
    unsigned char temp;
    Select_SD();

    if(ccs == 0x02) Address<<=9;                      // Convierte el sector a bytes para SDSC.
    if(SD_Ready() == 0) return SD_NOT_READY;
    SD_Send_Command(READ_SINGLE_BLOCK,Address,0xFF);
    temp = R1_Response();
    if(temp == 0xFF){LEDcard(10,2);}
    if(temp != 0x00) return temp;
    temp = SD_Read(Buffer,512);

    Release_SD();
    return temp;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Escribe un sector completo de 512 bytes.
// *****************************************************************************
unsigned char SD_Write_Block(unsigned char *Buffer, unsigned long Address){
    unsigned char temp;
    unsigned int i;

    Select_SD();

    if(ccs == 0x02) Address<<=9;        // Convierte el sector a bytes para SDSC.
    if(SD_Ready() == 0) return SD_NOT_READY;
    SD_Send_Command(WRITE_BLOCK,Address,0xFF);
    temp = R1_Response();
    if(temp != 0x00) return temp;
    temp = SPISD_Write(0xFE);    // Token de inicio del bloque.
    for(i = 0; i < 512; i++){
        temp = SPISD_Write(Buffer[i]);
    }
    temp = SPISD_Write(0xFF);        // CRC ficticio, byte alto.
    temp = SPISD_Write(0xFF);
    temp = SPISD_Write(0xFF); // Respuesta de datos.
    temp = (temp&0x0E)>>1;
    if(SD_Ready() == 0) return SD_NOT_READY;

    Release_SD();
    if(temp == 0x02) return DATA_ACCEPTED;
    else if(temp == 0x05) return DATA_REJECTED_CRC_ERROR;
    else if(temp == 0x06) return DATA_REJECTED_WR_ERROR;
    else return ERROR;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Intenta inicializar la tarjeta hasta try_value veces.
// *****************************************************************************
unsigned char SD_Init_Try(unsigned char try_value){
    unsigned char i,init_status;
    if(try_value == 0) try_value = 1;
    for(i = 0; i < try_value; i++){
        init_status = SD_Init();
        if(init_status == SUCCESSFUL_INIT) break;
        Release_SD();
        Delay_ms(10);
    }
    return init_status;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Inicializa la tarjeta y devuelve un código de estado.
// *****************************************************************************
unsigned char SD_Init(void){
    // Variables locales del protocolo.
    unsigned int i;
    unsigned int j;
    unsigned char temp;
    unsigned long temp_long;

    // Configura el chip select como salida y lo libera.
    sd_CS_tris = 0;

    Release_SD();

    // La inicialización requiere una velocidad baja.
    SPISD_Init(SLOW);

    // Envía al menos 74 ciclos con MOSI en alto antes de CMD0.
    for(i = 0; i < 80; i++) SPISD_Write(0xFF);

    Select_SD();

    // Mantiene la línea de reloj activa antes del primer comando.
    for(i = 0; i < 16; i++) SPISD_Write(0xFF);


    // Envía CMD0 hasta que la tarjeta responda en estado inactivo.
    for(i = 0; i < SD_TIME_OUT; i++){
        SD_Send_Command(GO_IDLE_STATE,0x00000000,0x4A);     // CMD0
        temp = R1_Response();
        if(temp == (1<<IDLE_STATE)) {
           break;
        }
        if(i==(SD_TIME_OUT-1)) return CARD_NOT_INSERTED;
    }
    

    // CMD8 comprueba el rango de 2.7 a 3.6 V y el patrón 0xAA.
    if(SD_Ready() == 0){
       return SD_NOT_READY;
    }
    
    SD_Send_Command(SEND_IF_COND,0x000001AA,0x43);          // CMD8
    temp = R1_Response();
    
    if(temp != (1<<IDLE_STATE)){
        // Una respuesta distinta indica una tarjeta antigua: usa CMD1.
        for(i = 0; i < SD_TIME_OUT; i++){
            if(SD_Ready() == 0) return SD_NOT_READY;
            SD_Send_Command(SEND_OP_COND,0x00000000,0x7C);  // CMD1
            temp = R1_Response();
            if(temp == 0x00) break;
            if(i==(SD_TIME_OUT-1)) return UNUSABLE_CARD;
        }
        
    } else if (temp == (1<<IDLE_STATE)){

        temp_long = Response_32b();
        temp = (temp_long & ECHO_BACK_MASK);
        if(temp != 0xAA) return ECHO_BACK_ERROR;
        temp = ((temp_long & VOLTAGE_ACCEPTED_MASK)>>8);
        if(temp != 0x01) return INCOMPATIBLE_VOLTAGE;

        // Lee el registro OCR.
        if(SD_Ready() == 0) return SD_NOT_READY;
        SD_Send_Command(READ_OCR,0x00000000,0x7E);          // CMD58
        temp = R1_Response();                               // Parte de la respuesta R3
        if(temp != (1<<IDLE_STATE)) return temp;
        temp_long = Response_32b();                         // Parte de la repuesta R3
        if((temp_long & VOLTAGE_RANGE_MASK) != VOLTAGE_RANGE_MASK)
            return INCOMPATIBLE_VOLTAGE;

        // Activa temporalmente la verificación CRC antes de ACMD41.
        if(SD_Ready() == 0) return SD_NOT_READY;
        SD_Send_Command(CRC_ON_OFF,0x00000001,0x48);        // CMD59, CRC final 0x91
        temp = R1_Response();
        if(temp != (1<<IDLE_STATE)) return temp;

        // Espera a que termine la inicialización de la tarjeta.
        for(i = 0; i < SD_TIME_OUT; i++){
            if(SD_Ready() == 0) return SD_NOT_READY;
            SD_Send_Command(APP_CMD,0x00000000,0x32);           // CMD55
            temp = R1_Response();
            if(SD_Ready() == 0) return SD_NOT_READY;
            
            // Mantiene la separación requerida entre comandos.
            for(j = 0; j < 16; j++) SPISD_Write(0xFF);
            
            SD_Send_Command(SD_SEND_OP_COND,0x40000000,0x3B);   // ACMD41
            temp = R1_Response();
            if(temp == 0x00) break;
            if(i==(SD_TIME_OUT-1)) return UNUSABLE_CARD;
        }
    }
    else return temp;

    // Desactiva la verificación CRC para los comandos siguientes.
    if(SD_Ready() == 0) return SD_NOT_READY;
    SD_Send_Command(CRC_ON_OFF,0x00000000,0x48);        // CMD59
    temp = R1_Response();
    if(temp != 0x00) return temp;

    // Fija el tamaño de bloque en 512 bytes.
    if(SD_Ready() == 0) return SD_NOT_READY;
    SD_Send_Command(SET_BLOCKLEN,0x00000200,0x0A);      // CMD16
    temp = R1_Response();
    if(temp != 0x00) return temp;

    // Lee el estado de capacidad de la tarjeta.
    if(SD_Ready() == 0) return SD_NOT_READY;
    SD_Send_Command(READ_OCR,0x00000000,0x7E);          // CMD58
    temp = R1_Response();
    if(temp != 0x00) return temp;
    temp_long = Response_32b();
    ccs = (long)(temp_long >> 30);

    Release_SD();
    
    // Envía ciclos adicionales antes de cambiar la velocidad.
    for(i = 0; i < 16; i++) SPISD_Write(0xFF);
    
    // Cambia SPI a la velocidad de operación.
    SPISD_Init(FAST);

    return SUCCESSFUL_INIT;
}
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// Recibe una respuesta R1 de 8 bits.
// *****************************************************************************
unsigned char R1_Response(void){
    unsigned char temp;
    temp = SPISD_Write(0xFF);
    temp = SPISD_Write(0xFF);
    return temp;
}
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// Recibe una respuesta R2 de 16 bits.
// *****************************************************************************
unsigned int R2_Response(void){
    unsigned char temp;
    unsigned int response;
    temp = SPISD_Write(0xFF);
    response = SPISD_Write(0xFF);
    temp = SPISD_Write(0xFF);
    response = (response<<8)|temp;
    return response;
}
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// Recibe los 32 bits de una respuesta extendida.
// *****************************************************************************
unsigned long Response_32b(void){
    unsigned char temp;
    unsigned long response;
    response = SPISD_Write(0xFF);
    temp = SPISD_Write(0xFF);
    response = (response<<8)|temp;
    temp = SPISD_Write(0xFF);
    response = (response<<8)|temp;
    temp = SPISD_Write(0xFF);
    response = (response<<8)|temp;
    return response;
}
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// Envía un comando SD con argumento y CRC7.
// *****************************************************************************
void SD_Send_Command(unsigned char command,unsigned long argument, unsigned char crc){

    SPISD_Write(command |= 0x40);

    // Envía el argumento de 32 bits.
    SPISD_Write((unsigned char)(argument>>24));
    SPISD_Write((unsigned char)(argument>>16));
    SPISD_Write((unsigned char)(argument>>8));
    SPISD_Write((unsigned char)(argument));

    // Envía el CRC y el bit de parada.
    SPISD_Write((crc<<1)|0x01);
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Espera hasta que la tarjeta quede libre.
// *****************************************************************************
unsigned char SD_Ready(void){
    unsigned int i;
    unsigned char temp;
    for(i = 0; i < SD_TIME_OUT; i++){
        temp = SPISD_Write(0xFF);
        if(temp == 0xFF) break;
        if(i == (SD_TIME_OUT-1)) return 0x00;
    }
    return temp;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Libera la tarjeta colocando CS en nivel alto.
// *****************************************************************************
void Release_SD(void){
    sd_CS_lat = 1;
    asm nop;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Selecciona la tarjeta colocando CS en nivel bajo.
// *****************************************************************************
void Select_SD(void){
    sd_CS_lat = 0;
    asm nop;
}
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Devuelve DETECTED si el pin indica que la tarjeta está insertada.
// *****************************************************************************
unsigned char SD_Detect(void) {
    if (sd_detect_port == 0) {
         return DETECTED;
    } else {
         return 0;
    }
}
// *****************************************************************************
// *****************************************************************************