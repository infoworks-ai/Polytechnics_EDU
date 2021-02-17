#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include "MAX30101.h"

//Data Important Variable
uint32_t MAX30101_red_led, MAX30101_ir_led;
uint32_t irBuffer[101]; 
uint32_t redBuffer[101];

int32_t bufferLength = 100;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

uint8_t MAX30101_i;



//-----------------------------------------------------------

// MAX30101 Sensor Slave Address
void MAX30101_I2C_Slave_Check(void)
{
    if (ioctl(MAX30101_fd, I2C_SLAVE, MAX30101_ADDRESS) < 0)
    {
        printf("Failed to acquire bus access and/or talk to slave\n");
        exit(1);
    }

    if (write(MAX30101_fd, NULL, 0) < 0)
    {
        printf("[MAX30101(0x57)] I2C Sensor Is Missing\n");
        exit(1);
    }
    else
    {
        printf("Check OK!! [MAX30101(0x57)] I2C Sensor\n");
    }
}

//-----------------------------------------------------------

//MAX30101 Sensor Register Data Write
int WriteData(uint8_t reg_addr, uint8_t *data, int size) 
{
    uint8_t *buf;
    buf = malloc(size + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, size);
    write(MAX30101_fd, buf, size + 1);
    free(buf);

    return 1;
}

//-----------------------------------------------------------

//MAX30101 Sensor Register Data Read
int ReadData(uint8_t reg_addr, uint8_t *data, int size) 
{
    write(MAX30101_fd, &reg_addr, 1);
    read(MAX30101_fd, data, size);

    return 1;
}

//-----------------------------------------------------------

void MAX30101_I2C_MUX(void)
{
	uint8_t data = 0x01;

	if (ioctl(MAX30101_fd, I2C_SLAVE, TCA9543A_ADDRESS) < 0)
    {
        printf("Failed to acquire bus access and/or talk to slave\n");
        exit(1);
    }

    if (write(MAX30101_fd, NULL, 0) < 0)
    {
        printf("[TCA9543A(0x70)] I2C Sensor Is Missing\n");
		exit(1);
    }
    else
    {
        printf("Check OK!! [TCA9543A(0x70)] I2C Sensor\n");
		WriteData(TCA9543A_ADDRESS, &data, 1); 		
	}
}

//-----------------------------------------------------------

//MAX30101 Sensor Setting MAX30101_Reset(MAX30105 Sensor Standby)
void MAX30101_Reset(void) 
{
    uint8_t data = 0x40;
    WriteData(MAX30101_MODE_CONFIG, &data, 1);
    usleep(1000 * 100); 
}

//-----------------------------------------------------------

//MAX30101 Sensor Register Setting
int MAX30101_Init(void) 
{
    uint8_t data;

    data = 0x04;
    WriteData(MAX30101_FIFO_CONFIG, &data, 1); //Address : 0x08

    data = 0x03;
    WriteData(MAX30101_MODE_CONFIG, &data, 1); //Address : 0x09

    data = 0x27;
    WriteData(MAX30101_SPO2_CONFIG, &data, 1); //Address : 0x0A

    data = 0x2A;
    WriteData(MAX30101_LED1_PA, &data, 1); //Address : 0x0C

    data = 0x2A;
    WriteData(MAX30101_LED2_PA, &data, 1); //Address : 0x0D

    data = 0x2A;
    WriteData(MAX30101_LED3_PA, &data, 1); //Address : 0x0E

    data = 0x2A;
    WriteData(MAX30101_LED_PROX_AMP, &data, 1); //Address : 0x10

    data = 0x00;
    WriteData(MAX30101_FIFO_WR_PTR, &data, 1); //Address : 0x04

    data = 0x00;
    WriteData(MAX30101_OVF_COUNTER, &data, 1); //Address : 0x05

    data = 0x00;
    WriteData(MAX30101_FIFO_RD_PTR, &data, 1); //Address : 0x06

    usleep(1000 * 100); 

    return 1;
}

//-----------------------------------------------------------

//Obtaining MAX30101_red_led, MAX30101_ir_led Data, GREEN_LED 
void MAX30101_Read_FIFO(void) 
{
    uint8_t buf[9];
    MAX30101_red_led = 0;
    MAX30101_ir_led = 0;

	if(!ReadData(MAX30101_FIFO_DATA, buf, 9))
    {
        printf("MAX30105 Data Read Error\n");
        exit(1);
    }
		
	//Obtaining MAX30101_red_led Data
	MAX30101_red_led += (uint32_t)buf[0] << 16;
    MAX30101_red_led += (uint32_t)buf[1] << 8;
    MAX30101_red_led += (uint32_t)buf[2];

	//Obtaining MAX30101_ir_led Data
    MAX30101_ir_led += (uint32_t)buf[3] << 16;
    MAX30101_ir_led += (uint32_t)buf[4] << 8;
    MAX30101_ir_led += (uint32_t)buf[5];

	//MAX30101_red_led Data Save
    MAX30101_red_led &= 0x03FFFF;
    
	//MAX30101_ir_led Data Save
	MAX30101_ir_led &= 0x03FFFF;
}

//-----------------------------------------------------------

//MAX30101 HR, SPO2 Data Print
void MAX30101_Data_Print(uint8_t data_check)
{
    switch(data_check)
    {
        case 0 :
            printf("Red: %7d, IR: %7d\n", MAX30101_red_led, MAX30101_ir_led);    
            break;
        
        case 1 : 
            //printf("Red: %7d, IR: %7d, BPM: %7d, HRvalid: %7d, SPO2: %7d, SPO2Valid: %7d\n", redBuffer[MAX30101_i], irBuffer[MAX30101_i], heartRate, validHeartRate, spo2, validSPO2);
            printf("HeartRate: %7d BPM, SPO2: %7d %\n", heartRate, spo2);
			break;
    }
}

//-----------------------------------------------------------

/* MAX30101 Data Setup
 * description : MAX30101 센서의 LED 값을 읽어와서 심장박동 수와 산소포화도를 계산한다.
 */
void MAX30101_Data_Setup(void)
{
    uint8_t finger_message_flag = 0;

    printf("---------------------------------------------------------------------------------\n");
    printf("Start MAX30101 Data Setup\n");
    usleep(1000 * 1000); 
    printf("Put On Your Finger In Sensor\n\n");

    while(1)
    {
	
        for (MAX30101_i = 0; MAX30101_i < bufferLength; MAX30101_i++)
		{
            // MAX30101에 있는 Red_LED, IR_LED Data, GREEN_LED 의 센서 값을 읽어 온다.
            MAX30101_Read_FIFO();
            if(MAX30101_ir_led >= 145000)
            {
                redBuffer[MAX30101_i] = MAX30101_red_led;
                irBuffer[MAX30101_i] = MAX30101_ir_led;       
                //MAX30101_Data_Print(0);
            }
            else
            {
                break;
            }    
        }
        if(MAX30101_i >= bufferLength-1)
        {
            break;
        }
    }
    // ir_led와 red_led에 저장된 값을 매개변수로 넘겨주고, spo2, vaslidSPO2, heartRate, validHeartrate 변수에 계산해서 저장한다.
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    printf("---------------------------------------------------------------------------------\n");
    printf("Data Setup Finish\n");
    printf("HR, SPO2 Data Print Start\n");
    printf("\n");
}

//-----------------------------------------------------------

//MAX30101 HR, SPO2 Data Analisis
void MAX30101_Data_Analisis(void)
{
    static uint16_t printf_data_number = 0;
    static uint8_t finger_message_flag = 0;

    // red_led와 ir_led 데이터 buffer 배열에서 25~99 번째 값들을 0~74번째로 옮긴다.
    for (MAX30101_i = 25; MAX30101_i < bufferLength; MAX30101_i++)
    {
        redBuffer[MAX30101_i - 25] = redBuffer[MAX30101_i];
        irBuffer[MAX30101_i - 25] = irBuffer[MAX30101_i];
    }

    // red_led와 ir_led 데이터 buffer 배열의 75~99번째 값들을 다시 센서에서 읽어온 값들로 채운다.
    for (MAX30101_i = 75; MAX30101_i < bufferLength; MAX30101_i++)
    {      
        MAX30101_Read_FIFO();    
        redBuffer[MAX30101_i] = MAX30101_red_led;
        irBuffer[MAX30101_i] = MAX30101_ir_led;  

        if(printf_data_number == 1000)
        {
            if(heartRate != -999 && spo2 != -999)
            {
                if(MAX30101_ir_led >= 145000)
                {
                    MAX30101_Data_Print(1);
                    finger_message_flag = 0;
                    printf_data_number = 0;
                }
                else
                {
                    if(finger_message_flag == 0)
                    {
                        printf("---------------------------------------------------------------------------------\n");
                        printf("Put On Your Finger In Sensor\n\n");
                        finger_message_flag = 1;
                    }
                    printf_data_number = 0;
                }
            }
        }
        else
        {
            printf_data_number++;
        }
    }
    // ir_led와 red_led에 저장된 값을 매개변수로 넘겨주고, spo2, vaslidSPO2, heartRate, validHeartrate 변수에 계산해서 저장한다.
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}

//-----------------------------------------------------------

int MAX30101_main(void)// /dev/i2c-0 
{
    if((MAX30101_fd = open("/dev/i2c-2", O_RDWR)) < 0) 
	{
        printf("Failed to open i2c-2");
        exit(1);
    }

	MAX30101_I2C_MUX();
	MAX30101_I2C_Slave_Check();
	MAX30101_Reset(); 
	MAX30101_Init();
    MAX30101_Data_Setup();
        
	while(1) 
	{
        MAX30101_Data_Analisis();
    }
    return 0;
}
