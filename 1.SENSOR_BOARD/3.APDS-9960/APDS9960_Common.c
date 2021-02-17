#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>

#include "APDS9960.h"

//---------------------------------------------------------------


extern int APDS_9960_fd;

/* APDS-9960 Sensor Register Data Write
 * [in] : reg_addr - 사용하는 센서의 register 주소, data - reg_addr의 주소에 써줄 값의 시작 주소, 
 *          size - data의 주소에서 얼만큼 값을 읽어올 것인지 지정해주는 매개 변수.
 * [out] : WriteData 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : data 매개변수에 담긴 주소로 부터 size 만큼 크기의 값을 읽어와 센서의 register 주소에 찾아가 저장한다.
 */
int APDS_9960_WriteData(uint8_t reg_addr, uint8_t *data, int size) 
{
    uint8_t *buf;
    buf = malloc(size + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, size);
    write(APDS_9960_fd, buf, size + 1);
    free(buf);

    return 1;
}

//---------------------------------------------------------------

/* APDS-9960 Sensor Register Data Read
 * [in] : reg_addr - 사용하는 센서의 register 주소, data - reg_addr의 주소에서 읽어온 값을 써줄 목적지 주소, 
 *          size - reg_addr에서 읽어올 데이터의 크기
 * [out] : ReadData 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : 센서의 register 주소에서 size 만큼 byte를 읽어와 data 매개변수의 주소에 찾아가 저장한다.
 */
int APDS_9960_ReadData(uint8_t reg_addr, uint8_t *data, int size)
{
    write(APDS_9960_fd, &reg_addr, 1);
    read(APDS_9960_fd, data, size);

    return 1;
}

//Activating APDS-9960 GetMode
uint8_t APDS9960_getMode()
{
    uint8_t enable_value;
    
    if( !APDS_9960_ReadData(APDS9960_ENABLE, &enable_value, 1) ) 
	{
        return APDS9960_ERROR;
    }
    
    return enable_value;
}

//---------------------------------------------------------------

//Activating APDS-9960 Sensor
int APDS9960_setMode(uint8_t mode, uint8_t enable)
{
    uint8_t reg_val;

    reg_val = APDS9960_getMode();
    if( reg_val == APDS9960_ERROR ) 
	{
        printf("APDS9960_ERROR\n");
        return 0;
    }
    
    enable = enable & 0x01;
    if( mode >= 0 && mode <= 6 ) 
	{
        if (enable) 
		{
            reg_val |= (1 << mode);
        } 
		
		else 
		{
            reg_val &= ~(1 << mode);
        }
    } 
	
	else if( mode == ALL ) 
	{
        if (enable) 
		{
            reg_val = 0x7F;
        } 
		
		else 
		{
            reg_val = 0x00;
        }
    }
        
    if( !APDS_9960_WriteData(APDS9960_ENABLE, &reg_val, 1) ) 
	{
        return 0;
    }
        
    return 1;
}

//---------------------------------------------------------------

int APDS9960_setLEDDrive(uint8_t drive)
{
    uint8_t val;
    
    if( !APDS_9960_ReadData(APDS9960_CONTROL, &val, 1) ) {
        return 0;
    }
    
    drive &= 0b00000011;
    drive = drive << 6;
    val &= 0b00111111;
    val |= drive;
    
    if( !APDS_9960_WriteData(APDS9960_CONTROL, &val, 1) ) {
        return 0;
    }
    
    return 1;
}

//Activating APDS-9960 SensorPower
int APDS9960_enablePower()
{
    if( !APDS9960_setMode(POWER, 1) ) 
	{
        return 0;
    }
    
    return 1;
}

//APDS9960 Sensor Register Setting
int APDS9960_Init() 
{

    //ID 확인
    uint8_t id, data;
    APDS_9960_ReadData(APDS9960_ID, &id, 1);
    printf("APDS-9960 ID: %x\n",id);
    if(id != 0xAB) {
        return 0;
    }
    //ENABLE 레지스터 전부 끄기
    if(!APDS9960_setMode(ALL, 0))      //MSB는 Reserved 비트
        return 0;
    data = DEFAULT_ATIME;
    if(!APDS_9960_WriteData(APDS9960_ATIME, &data, 1))
        return 0;
    data = DEFAULT_WTIME;
    if(!APDS_9960_WriteData(APDS9960_WTIME, &data, 1)) 
        return 0;
    data = DEFAULT_PROX_PPULSE;
    if(!APDS_9960_WriteData(APDS9960_PPULSE, &data, 1)) 
        return 0;
    data = DEFAULT_POFFSET_UR;
    if(!APDS_9960_WriteData(APDS9960_POFFSET_UR, &data, 1)) 
        return 0;
    data = DEFAULT_POFFSET_DL;
    if(!APDS_9960_WriteData(APDS9960_POFFSET_UR, &data, 1)) 
        return 0;
    data = DEFAULT_CONFIG1;
    if(!APDS_9960_WriteData(APDS9960_CONFIG1, &data, 1))
        return 0;
    data = DEFAULT_CONFIG2;
    if(!APDS_9960_WriteData(APDS9960_CONFIG2, &data, 1))
        return 0;
    data = DEFAULT_CONFIG3;
    if(!APDS_9960_WriteData(APDS9960_CONFIG3, &data, 1))
        return 0;
    data = 0x00;
    if(!APDS_9960_WriteData(APDS9960_CONTROL, &data, 1))     //LED Drive 12.5, P, A Gain 1x, AGAIN_1X
        return 0;
    data = 0xFF;
    if(!APDS_9960_WriteData(APDS9960_AILTL, &data, 1))
        return 0;
    if(!APDS_9960_WriteData(APDS9960_AILTH, &data, 1))
        return 0;
    data = 0x00;
    if(!APDS_9960_WriteData(APDS9960_AIHTL, &data, 1))
        return 0;
    if(!APDS_9960_WriteData(APDS9960_AIHTH, &data, 1))
        return 0;
    data = 0;
    if(!APDS_9960_WriteData(APDS9960_PILT, &data, 1))
        return 0;
    data = 50;
    if(!APDS_9960_WriteData(APDS9960_PIHT, &data, 1))
        return 0;
    
    //제스쳐 설정
    data = DEFAULT_GPENTH;
    if(!APDS_9960_WriteData(APDS9960_GPENTH, &data, 1))
        return 0;
    data = DEFAULT_GEXTH;
    if(!APDS_9960_WriteData(APDS9960_GEXTH, &data, 1))
        return 0;
    data = DEFAULT_GCONF1;
    if(!APDS_9960_WriteData(APDS9960_GCONF1, &data, 1))
        return 0;
    if(!APDS9960_setGestureGain(DEFAULT_GGAIN))
        return 0;
    if(!APDS9960_setGestureLEDDrive(DEFAULT_GLDRIVE))
        return 0;
    if(!APDS9960_setGestureWaitTime(DEFAULT_GWTIME))
    	return 0;
    //기본 오프셋 설정
    data = 0;
    if(!APDS_9960_WriteData(APDS9960_GOFFSET_U, &data, 1))
        return 0;
    data = 0;
    if(!APDS_9960_WriteData(APDS9960_GOFFSET_D, &data, 1))
        return 0;
    data = 0;
    if(!APDS_9960_WriteData(APDS9960_GOFFSET_L, &data, 1))
        return 0;
    data = 0;
    if(!APDS_9960_WriteData(APDS9960_GOFFSET_R, &data, 1))
        return 0;
    data = DEFAULT_GPULSE;
    if(!APDS_9960_WriteData(APDS9960_GPULSE, &data, 1))
        return 0;

    return 1;
}
