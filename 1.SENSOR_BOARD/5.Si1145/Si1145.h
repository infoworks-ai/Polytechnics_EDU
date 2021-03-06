#ifndef __SI1145__
#define __SI1145__

//SI1145 Sensor Slave Address
#define SI1145_I2C_ADDRESS 0x60

//SI1145 Sensor Register Address
#define SI1145_REG_MEASRATE0 0x08
#define SI1145_REG_MEASRATE1  0x09

#define SI1145_REG_INTCFG  0x03
#define SI1145_REG_IRQEN  0x04
#define SI1145_REG_IRQSTAT  0x21
#define SI1145_REG_COMMAND  0x18
#define SI1145_REG_HWKEY  0x07

#define SI1145_REG_UCOEFF0  0x13
#define SI1145_REG_UCOEFF1  0x14
#define SI1145_REG_UCOEFF2  0x15
#define SI1145_REG_UCOEFF3  0x16

#define SI1145_REG_PSLED21  0x0F

#define SI1145_REG_COMMAND  0x18

#define SI1145_REG_PARAMWR  0x17

#define SI1145_REG_UVINDEX0 0x2C
#define SI1145_REG_ALSIRDATA0 0x24
#define SI1145_REG_ALSVISDATA0 0x22
#define SI1145_REG_PS1DATA0 0x26

//SI1145 Parameter Value
#define SI1145_PARAM_CHLIST   0x01
#define SI1145_PARAM_CHLIST_ENUV 0x80
#define SI1145_PARAM_CHLIST_ENALSIR 0x20
#define SI1145_PARAM_CHLIST_ENALSVIS 0x10
#define SI1145_PARAM_CHLIST_ENPS1 0x01

#define SI1145_PARAM_PS1ADCMUX   0x07
#define SI1145_PARAM_ADCMUX_LARGEIR  0x03
#define SI1145_PARAM_PSLED12SEL   0x02
#define SI1145_PARAM_PSLED12SEL_PS1LED1 0x01
#define SI1145_PARAM_PSADCGAIN 0x0B
#define SI1145_PARAM_PSADCOUNTER   0x0A
#define SI1145_PARAM_ADCCOUNTER_511CLK 0x70
#define SI1145_PARAM_PSADCMISC 0x0C
#define SI1145_PARAM_PSADCMISC_RANGE 0x20
#define SI1145_PARAM_PSADCMISC_PSMODE 0x04

#define SI1145_PARAM_ALSIRADCMUX   0x0E
#define SI1145_PARAM_ADCMUX_SMALLIR  0x00
#define SI1145_PARAM_ALSIRADCGAIN 0x1E
#define SI1145_PARAM_ALSIRADCOUNTER   0x1D
#define SI1145_PARAM_ADCCOUNTER_511CLK 0x70
#define SI1145_PARAM_ALSIRADCMISC 0x1F
#define SI1145_PARAM_ALSVISADCGAIN 0x11
#define SI1145_PARAM_ALSVISADCOUNTER   0x10
#define SI1145_PARAM_ALSVISADCMISC 0x12
#define SI1145_PARAM_ALSIRADCMISC_RANGE 0x20
#define SI1145_PARAM_ALSVISADCMISC_VISRANGE 0x20

#endif

#include <stdint.h>

/*"/dev/i2c-0"'s file descriptor*/
int Si1145_fd;

void Si1145_I2C_Slave_Check(void);
int Si1145_WriteData(uint8_t reg_addr, uint8_t *data, int size); 
int Si1145_ReadData(uint8_t reg_addr, uint8_t *data, int size);
void Si1145_WriteParam(uint8_t p, uint8_t v);
void Si1145_Reset(void);
void Si1145_Init(void);
void UV_Visible_IR_Prox_Extraction(void);
void Si1145_Data_print(void);
int Si1145_main(void);