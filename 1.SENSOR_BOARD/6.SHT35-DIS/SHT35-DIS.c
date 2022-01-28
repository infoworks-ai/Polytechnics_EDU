#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "SHT35-DIS.h"



uint8_t reg, cmd;
uint8_t Temp_Hum_Raw_Data[5];
uint16_t ST, SRH;
double temp, hum;

//-----------------------------------------------------------


void SHT35_I2C_Slave_Check(void)
{	
    if(ioctl(SHT35_DIS_fd, I2C_SLAVE, SHT_SLAVE_ADDRESS) < 0)
	{
        printf("Failed to acquire bus access and/or talk to slave\n");
        exit(1);
    }
	
	if(write(SHT35_DIS_fd, NULL, 0) < 0)
	{
		printf("[SHT35-DIS(0x45)] I2C Sensor Is Missing\n");
		exit(1);
	}
	else
	{
		printf("Check OK!! [SHT35-DIS(0x45)] I2C Sensor\n");
	}
}

//---------------------------------------------------------------

//Sensor Register Data Write
/* [in] :   reg_addr - 사용하는 센서의 register 주소, data - reg_addr의 주소에서 읽어온 데이터를 써줄 목적지 주소
            size - reg_addr에서 읽어올 데이터의 크기
 * [out] :  WriteData 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : 센서의 register 주소에서 size 만큼 byte를 읽어와 data 매개변수의 주소에 찾아가 써준다.
 */
int SHT35_WriteData(uint8_t reg_addr, uint8_t *data, int size)
{
	uint8_t *buf;
	
	buf = malloc(size + 1);
	buf[0] = reg_addr;
	memcpy(buf + 1, data, size);
	write(SHT35_DIS_fd, buf, size + 1);
	free(buf);

	return SUCCESS;
}

//---------------------------------------------------------------

//SHT3X-DIS Sensor SHT35_Reset
void SHT35_Reset(void)
{	
	reg = SHT_SOFTRESET >> 8 & 0xFF; //0x30
	cmd = SHT_SOFTRESET & 0xFF; //0xA2
	SHT35_WriteData(reg, &cmd, 1);
}

//---------------------------------------------------------------

//Temperature, Humidity Data Extraction
/*
 * description : temperature,Humidity 센서 데이터를 Datasheet에 해당하는 방식에 맞게 바꿔주고 temp와 hum 변수에 저장하는 함수
 */
void Temp_Hum_Extraction(void)
{
	reg = SHT_MEAS_HIGHREP >> 8 & 0xFF; //0x24
	cmd = SHT_MEAS_HIGHREP & 0xFF; //0x00
	SHT35_WriteData(reg, &cmd, 1);
	usleep(1000*100);

	read(SHT35_DIS_fd, Temp_Hum_Raw_Data, 5); //Data receiving format 8bit |Temp MSB|TEMPLSB|CRC|HUM MSB|HUM LSB|CRC|

	//Obtaining Temperature Data
	ST = Temp_Hum_Raw_Data[0]; //Front 8bit Temperature
	ST <<= 8;
	ST |= Temp_Hum_Raw_Data[1];

	//Obtaining Humidity Data
	SRH = Temp_Hum_Raw_Data[3];
	SRH <<= 8;
	SRH |= Temp_Hum_Raw_Data[4];

	//temperature Data Save
	temp = ST;
	temp *= 175;
	temp = temp / (0xFFFF - 0x0001);
	temp += -45;

	//Humidity Data Save
	hum = SRH;
	hum *= 100;
	hum = hum / (0xFFFF - 0x0001);  
}

//---------------------------------------------------------------

//SHT3X-DIS Temperature, Humidity Data Print
void SHT35_Data_Print(void)
{	
	printf("│ Temperature: %.3lf °C │ Humidity: %.3lf %RH │\r\n", temp, hum);
}

//---------------------------------------------------------------

int SHT35_main(void)
{	
	if((SHT35_DIS_fd = open("/dev/i2c-3", O_RDWR)) < 0) 
	{
		perror("Failed to open i2c-3");
		exit(1);
	}

	SHT35_I2C_Slave_Check();
	SHT35_Reset();
	usleep(1000*100);	
	
	while(1)
	{		
		Temp_Hum_Extraction();
		SHT35_Data_Print();
	}
	
	return 0;
}
