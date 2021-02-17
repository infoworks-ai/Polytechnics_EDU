//MAX30105 Sensor Slave Register
#define MAX30101_ADDRESS            0x57

//FIFO Register
#define MAX30101_FIFO_WR_PTR        0x04
#define MAX30101_OVF_COUNTER        0x05
#define MAX30101_FIFO_RD_PTR        0x06
#define MAX30101_FIFO_DATA          0x07

//Setting Register
#define MAX30101_FIFO_CONFIG        0x08
#define MAX30101_MODE_CONFIG        0x09
#define MAX30101_SPO2_CONFIG        0x0A

#define MAX30101_LED1_PA            0x0C
#define MAX30101_LED2_PA            0x0D
#define MAX30101_LED3_PA            0x0E
#define MAX30101_LED_PROX_AMP       0x10

//TCA9543A I2C MUX
#define TCA9543A_ADDRESS				0x73

#include <stdint.h>

/*"/dev/i2c-0"'s file descriptor*/
int MAX30101_fd;

void MAX30101_I2C_Slave_Check(void);
int WriteData(uint8_t reg_addr, uint8_t *data, int size);
int ReadData(uint8_t reg_addr, uint8_t *data, int size);
void MAX30101_I2C_MUX(void);
void MAX30101_Reset(void);
int MAX30101_Init(void);
void MAX30101_Read_FIFO(void);
void MAX30101_Data_Print(uint8_t data_check);
void MAX30101_Data_Setup(void);
void MAX30101_Data_Analisis(void);
int MAX30101_main(void);