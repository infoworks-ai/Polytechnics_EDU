//SHT3x-DIS Sensor Slave Address
#define SHT_SLAVE_ADDRESS 0x45

//SHT3x-DIS Sensor Register Address
#define SHT_MEAS_HIGHREP 0x2400
#define SHT_SOFTRESET 0x30A2

#define SUCCESS 1
#define SHT35_ERROR -1

#include <stdint.h>

/*"/dev/i2c-0"'s file descriptor*/
int SHT35_DIS_fd;

void SHT35_I2C_Slave_Check(void);
int SHT35_WriteData(uint8_t reg_addr, uint8_t *data, int size);
void SHT35_Reset(void);
void Temp_Hum_Extraction(void);
void SHT35_Data_Print(void);
int SHT35_main(void);
