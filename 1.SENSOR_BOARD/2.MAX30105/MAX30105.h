//MAX30105 Sensor Slave Register
#define MAX30105_ADDRESS            0x57

//FIFO Register
#define MAX30105_FIFO_WR_PTR        0x04
#define MAX30105_OVF_COUNTER        0x05
#define MAX30105_FIFO_RD_PTR        0x06
#define MAX30105_FIFO_DATA          0x07

//Setting Register
#define MAX30105_FIFO_CONFIG        0x08
#define MAX30105_MODE_CONFIG        0x09
#define MAX30105_SPO2_CONFIG        0x0A

#define MAX30105_LED1_PA            0x0C
#define MAX30105_LED2_PA            0x0D
#define MAX30105_LED3_PA            0x0E
#define MAX30105_LED_PROX_AMP       0x10

//TCA9543A I2C MUX
#define TCA9543A_ADDRESS            0x73

#include <stdint.h>

/*"/dev/i2c-0"'s file descriptor*/
int MAX30105_fd;

void MAX30105_I2C_Slave_Check(void);
int MAX30105_WriteData(uint8_t reg_addr, uint8_t *data, int size);
int MAX30105_ReadData(uint8_t reg_addr, uint8_t *data, int size);
void MAX30105_I2C_MUX(void);
void MAX30105_Reset(void);
int MAX30105_Init(void);
unsigned long millis();
void MAX30105_Read_FIFO(void);
void MAX30105_Data_Print(uint8_t data_check);
void MAX30105_Data_Setup(void);
void MAX30105_Data_Analisis(void);
int MAX30105_main(void);

