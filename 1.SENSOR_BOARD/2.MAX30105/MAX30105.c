#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include "MAX30105.h"

//Data Important Variable
long samplesTaken = 0; //Counter for calculating the Hz or read rate
long unblockedValue = 0; //Average IR at power up
unsigned long startTime; //Used to calculate measurement rate
uint32_t MAX30105_red_led, MAX30105_ir_led;
float Hz;
long currentDelta;

//-----------------------------------------------------------

// MAX30105 Sensor Slave Address
void MAX30105_I2C_Slave_Check(void)
{
	if (ioctl(MAX30105_fd, I2C_SLAVE, MAX30105_ADDRESS) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave\n");
		exit(1);
	}

	if (write(MAX30105_fd, NULL, 0) < 0)
	{
		printf("[MAX30105(0x57)] I2C Sensor Is Missing\n");
		exit(1);
	}
	else
	{
		printf("Check OK!! [MAX30105(0x57)] I2C Sensor\n");
	}
}

//-----------------------------------------------------------

//MAX30105 Sensor Register Data Write
int MAX30105_WriteData(uint8_t reg_addr, uint8_t *data, int size) 
{
	uint8_t *buf;
	buf = malloc(size + 1);
	buf[0] = reg_addr;
	memcpy(buf + 1, data, size);
	write(MAX30105_fd, buf, size + 1);
	free(buf);

	return 1;
}

//-----------------------------------------------------------

//MAX30105 Sensor Register Data Read
int MAX30105_ReadData(uint8_t reg_addr, uint8_t *data, int size) 
{
	write(MAX30105_fd, &reg_addr, 1);
	read(MAX30105_fd, data, size);

	return 1;
}

//-----------------------------------------------------------
//I2C_MUX
/*
 * description : I2C의 SLAVE 설정이 되었는지 확인하는 함수
 */
void MAX30105_I2C_MUX(void)
{
	uint8_t data = 0x02;

	if (ioctl(MAX30105_fd, I2C_SLAVE, TCA9543A_ADDRESS) < 0)
	{   
		printf("Failed to acquire bus access and/or talk to slave\n");
		exit(1);
	}   

	if (write(MAX30105_fd, NULL, 0) < 0)
	{   
		printf("[TCA9543A(0x70)] I2C Sensor Is Missing\n");
		exit(1);
	}   
	else
	{   
		printf("Check OK!! [TCA9543A(0x70)] I2C Sensor\n");
		MAX30105_WriteData(TCA9543A_ADDRESS, &data, 1);    
	}   
}


//-----------------------------------------------------------

//MAX30105 Sensor Setting MAX30105_Reset(MAX30105 Sensor Standby)
void MAX30105_Reset(void) 
{
	uint8_t data = 0x40;
	MAX30105_WriteData(MAX30105_MODE_CONFIG, &data, 1);
	usleep(1000 * 100); 
}

//-----------------------------------------------------------

//MAX30105 Sensor Register Setting
int MAX30105_Init(void)
{
	uint8_t data;

	data = 0x04;
	MAX30105_WriteData(MAX30105_FIFO_CONFIG, &data, 1); //Address : 0x08

	data = 0x03;
	MAX30105_WriteData(MAX30105_MODE_CONFIG, &data, 1); //Address : 0x09

	data = 0x0F;
	MAX30105_WriteData(MAX30105_SPO2_CONFIG, &data, 1); //Address : 0x0A

	data = 0xFF;
	MAX30105_WriteData(MAX30105_LED1_PA, &data, 1); //Address : 0x0C

	data = 0xFF;
	MAX30105_WriteData(MAX30105_LED2_PA, &data, 1); //Address : 0x0D

	data = 0xFF;
	MAX30105_WriteData(MAX30105_LED3_PA, &data, 1); //Address : 0x0E

	data = 0xFF;
	MAX30105_WriteData(MAX30105_LED_PROX_AMP, &data, 1); //Address : 0x10

	data = 0x00;
	MAX30105_WriteData(MAX30105_FIFO_WR_PTR, &data, 1); //Address : 0x04

	data = 0x00;
	MAX30105_WriteData(MAX30105_OVF_COUNTER, &data, 1); //Address : 0x05

	data = 0x00;
	MAX30105_WriteData(MAX30105_FIFO_RD_PTR, &data, 1); //Address : 0x06

	data = 0x00;
	MAX30105_WriteData(MAX30105_LED1_PA, &data, 1); //Address : 0x0C

	data = 0x00;
	MAX30105_WriteData(MAX30105_LED3_PA, &data, 1); //Address : 0x0E   

	usleep(1000 * 100); 

	return 1;
}

//-----------------------------------------------------------

// MAX30105 Working Time Calculte 
/* 
 * [out] : msec를 반환
 * description : 현재 시간을 msec로 변환해주는 함수
 */
unsigned long millis() 
{
	struct timeval te; 
	gettimeofday(&te, NULL);    //현재 시간 얻어오기
	unsigned long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // 밀리세컨드 계산
	return milliseconds;
}

//-----------------------------------------------------------

//Obtaining IR_LED Data
/*
 * description : LED 센서 데이터를 Datasheet에 해당하는 방식에 맞게 바꿔주고 ir_led에 저장하는 함수
 */
void MAX30105_Read_FIFO(void) 
{
	uint8_t buf[9];
	MAX30105_red_led = 0;
	MAX30105_ir_led = 0;

	if(!MAX30105_ReadData(MAX30105_FIFO_DATA, buf, 9))
	{
		printf("MAX30105 Data Read Error\n");
		exit(1);
	}

	//Obtaining MAX30105_ir_led Data
	MAX30105_ir_led += (uint32_t)buf[3] << 16;
	MAX30105_ir_led += (uint32_t)buf[4] << 8;
	MAX30105_ir_led += (uint32_t)buf[5];

	//MAX30105_ir_led Data Save
	MAX30105_ir_led &= 0x03FFFF;
}

//-----------------------------------------------------------

//MAX30105 Smoke(Particle) Data Print
/* [in] :   currentDelta가 일정 값 이상인 경우 1을 입력한다.
 * description : currentDelta의 값을 출력해주면서 currentDelta가 일정 값 이상인 경우에 알림을 해주는 함수
 */
void MAX30105_Data_Print(uint8_t data_check)
{	
	//printf("IR: %7d, Hz: %7.2f, delta: %7d  ", MAX30105_ir_led, Hz, currentDelta);
	printf("delta: %7d  ", currentDelta);
	if(data_check == 1)
	{
		printf("Somthing Is There!");
	}
	printf("\n");
}

//-----------------------------------------------------------

/* MAX30105 Data Setup
 * description : ir_led 값을 읽어와 unblockedValue 값을 세팅해준다.
 */
void MAX30105_Data_Setup(void)
{
	uint8_t x;

	printf("---------------------------------------------------------------------------------\n");
	printf("Start MAX30105 Data Setup\n");
	printf("Maintain A 17cm Gap From Sensor During Data Setup\n");
	printf("If Extraction Data Is Abnormal Or Data Setup Is Not Complete, Clean The Sensor.\n");
	printf("\n");

	while(1)
	{
		for (x = 0 ; x < 32 ; x++)
		{
			MAX30105_Read_FIFO(); 
			if(MAX30105_ir_led >= 7000)
			{
				unblockedValue = 0;
				break;
			}
			else
			{
				unblockedValue += MAX30105_ir_led;
			}
		}
		if(x >= 31)
		{
			break;
		}
	}

	unblockedValue /= 32;

	startTime = millis();

	printf("---------------------------------------------------------------------------------\n");
	printf("Data Setup Finish\n");
	printf("Smoke(Particle) Data Print Start\n");
	printf("\n");
}

//-----------------------------------------------------------

/* MAX30105 Smoke(Particle) Data Analisis
 * description : Data_Setup 함수를 통해 구한 unblockedValue 값을 이용해, ir_led값의 변화량을 구하고,
 * 공기 중 입자 데이터를 검출한다.
 */
void MAX30105_Data_Analisis(void)
{
	static uint16_t data_print_count = 0;
	samplesTaken++;

	MAX30105_Read_FIFO();
	Hz = (float)samplesTaken / ((millis() - startTime)/ 1000.0);

	currentDelta = MAX30105_ir_led - unblockedValue;

	if(data_print_count >= 100)
	{
		if(currentDelta > (long)2000)
		{
			MAX30105_Data_Print(1);
		}
		else
		{
			MAX30105_Data_Print(0);
		}
		data_print_count = 0;
	}
	else
	{
		data_print_count++;
	}
}

//-----------------------------------------------------------

int MAX30105_main(void)// /dev/i2c-0 
{
	if((MAX30105_fd = open("/dev/i2c-2", O_RDWR)) < 0) 
	{
		printf("Failed to open i2c-2");
		exit(1);
	}

	MAX30105_I2C_MUX();
	MAX30105_I2C_Slave_Check();
	MAX30105_Reset(); 
	MAX30105_Init();
	MAX30105_Data_Setup();

	while(1) 
	{
		MAX30105_Data_Analisis();
	}
	return 0;
}
