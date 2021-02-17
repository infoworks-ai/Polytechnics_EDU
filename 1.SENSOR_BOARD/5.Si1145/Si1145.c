#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "Si1145.h"

uint16_t uv, visible;

//-----------------------------------------------------------

//SI1145 Sensor Check
void Si1145_I2C_Slave_Check(void)
{	
    if(ioctl(Si1145_fd, I2C_SLAVE, SI1145_I2C_ADDRESS) < 0)
	{
        printf("Failed to acquire bus access and/or talk to slave\n");
        exit(1);
    }
	
	if(write(Si1145_fd, NULL, 0) < 0)
	{
		printf("[SI1145(0x70)] I2C Sensor Is Missing\n");
		exit(1);
	}
	else
	{
		printf("Check OK!! [SI1145(0x60)] I2C Sensor\n");
	}
}

//-----------------------------------------------------------

//Sensor Register Data Write
/* [in] :   reg_addr - 사용하는 센서의 register 주소, data - reg_addr의 주소에서 읽어온 데이터를 써줄 목적지 주소
            size - reg_addr에서 읽어올 데이터의 크기
 * [out] :  WriteData 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : 센서의 register 주소에서 size 만큼 byte를 읽어와 data 매개변수의 주소에 찾아가 써준다.
 */
int Si1145_WriteData(uint8_t reg_addr, uint8_t *data, int size) 
{
    uint8_t *buf;
    buf = malloc(size + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, size);
    write(Si1145_fd, buf, size + 1);
    free(buf);

    return 1;
}

//-----------------------------------------------------------

//Register Data Read
/* [in] :   reg_addr - 사용하는 센서의 register 주소, data - reg_addr의 주소에서 읽어온 데이터를 써줄 목적지 주소,
            size - reg_addr에서 읽어올 데이터의 크기
 * [out] :  ReatData 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : 센서의 register 주소에서 size 만큼의 data 를 읽는 함수.
 */
int Si1145_ReadData(uint8_t reg_addr, uint8_t *data, int size) 
{
    write(Si1145_fd, &reg_addr, 1);
    read(Si1145_fd, data, size);

    return 1;
}

//-----------------------------------------------------------

/* [in] : p는 쓰려는 인자 RAM, v는 쓰려는 인자
 * description : 센서에 필요한 parmeter를 해당하는 register에 작성하는 함수
 */
void Si1145_WriteParam(uint8_t p, uint8_t v)
{
    uint8_t data = v;
    Si1145_WriteData(SI1145_REG_PARAMWR, &v, 1);
    
	data = p | 0xA0;
    Si1145_WriteData(SI1145_REG_COMMAND, &data, 1);
}

//-----------------------------------------------------------

void Si1145_Reset(void)
{
	uint8_t data;
    
	data = 0x00;
    Si1145_WriteData(SI1145_REG_MEASRATE0, &data, 1);
    Si1145_WriteData(SI1145_REG_MEASRATE1, &data, 1);
    Si1145_WriteData(SI1145_REG_INTCFG, &data, 1);  
	Si1145_WriteData(SI1145_REG_IRQEN, &data, 1);
    
	data = 0xFF;
    Si1145_WriteData(SI1145_REG_IRQSTAT, &data, 1);
    
	data = 0x01;
    Si1145_WriteData(SI1145_REG_COMMAND, &data, 1);
    usleep(1000 * 10);
    
	data = 0x17;
    Si1145_WriteData(SI1145_REG_HWKEY, &data, 1);

    usleep(1000 * 10);
}

//-----------------------------------------------------------

void Si1145_Init(void)
{    
	uint8_t data;

	data = 0x29;
    Si1145_WriteData(SI1145_REG_UCOEFF0, &data, 1);
    
	data = 0x89;
    Si1145_WriteData(SI1145_REG_UCOEFF1, &data ,1);
    
	data = 0x02;
    Si1145_WriteData(SI1145_REG_UCOEFF2, &data, 1);
    
	data = 0x00;
    Si1145_WriteData(SI1145_REG_UCOEFF3, &data, 1);

    // UV 센서 사용을 허용해주는 파라미터 설정
    Si1145_WriteParam(SI1145_PARAM_CHLIST, SI1145_PARAM_CHLIST_ENUV |
    SI1145_PARAM_CHLIST_ENALSVIS | SI1145_PARAM_CHLIST_ENPS1);

    data = 0x03;
    Si1145_WriteData(SI1145_REG_PSLED21, &data, 1); // LED 1에만 20mA
    Si1145_WriteParam(SI1145_PARAM_PS1ADCMUX, SI1145_PARAM_ADCMUX_LARGEIR);
	
    // 근접 감지 1센서와 LED 1번 사용하는 파라미터 설정
    Si1145_WriteParam(SI1145_PARAM_PSLED12SEL, SI1145_PARAM_PSLED12SEL_PS1LED1);
    Si1145_WriteParam(SI1145_PARAM_PSADCGAIN, 0);
    Si1145_WriteParam(SI1145_PARAM_PSADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
    Si1145_WriteParam(SI1145_PARAM_PSADCMISC, SI1145_PARAM_PSADCMISC_RANGE|
        SI1145_PARAM_PSADCMISC_PSMODE);

    Si1145_WriteParam(SI1145_PARAM_ALSVISADCGAIN, 1);
    Si1145_WriteParam(SI1145_PARAM_ALSVISADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
    Si1145_WriteParam(SI1145_PARAM_ALSVISADCMISC, SI1145_PARAM_ALSVISADCMISC_VISRANGE);

    /************************/

    // 자동 측정
    data = 0xFF;
    Si1145_WriteData(SI1145_REG_MEASRATE0, &data, 1); // 255 * 31.25uS = 8ms
    
    // 자동 실행
    data = 0x0F;
    Si1145_WriteData(SI1145_REG_COMMAND, &data, 1);
}

//-----------------------------------------------------------

//UV_Visible_IR_Prox_Extraction
/*
 * description : UV, Visible 센서 데이터를 Datasheet에 해당하는 방식에 맞게 바꿔주고 uv와 visible 변수에 저장하는 함수
 */
void UV_Visible_IR_Prox_Extraction(void)
{
    uint8_t buf[2];
    
    uint16_t tmp;
	
	//UV Extraction
    Si1145_ReadData(SI1145_REG_UVINDEX0, buf, 2);
	tmp = buf[1] << 8;
    uv = (buf[0] + tmp) / 100;
	
	//Visible Extraction
    Si1145_ReadData(SI1145_REG_ALSVISDATA0, buf, 2);
    tmp = buf[1] << 8;
    visible = (buf[0] + tmp);
}

//-----------------------------------------------------------

//SI1145 UV Visible_Light IR Proximity Print
void Si1145_Data_print(void)
{	
	printf("UV: %hu, Visible Light: %hu lx\n", uv, visible);
}

//-----------------------------------------------------------

int Si1145_main(void)
{	
	if((Si1145_fd = open("/dev/i2c-2", O_RDWR)) < 0) 
	{
		perror("Failed to open i2c-2");
		exit(1);
	}

	Si1145_I2C_Slave_Check();
	Si1145_Reset();
	Si1145_Init();
    
	//100msec 대기
	usleep(1000*100);	
	
	while(1)
	{		
		UV_Visible_IR_Prox_Extraction();
		Si1145_Data_print();
		
		usleep(1000*100);
	}
	
	return 0;
}
