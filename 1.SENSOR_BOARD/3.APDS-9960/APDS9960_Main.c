#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>

#include "APDS9960.h"



int APDS_9960_main(void)
{
	// i2c 통신을 하기 위해 버스를 열어준다.
	if((APDS_9960_fd = open("/dev/i2c-3", O_RDWR)) < 0) 
	{
        perror("Failed to open i2c-3");
        exit(1);
    }
	
	// 사용할 센서의 고유 주소로 i2c 버스에 연결 시도. 
    if(ioctl(APDS_9960_fd, I2C_SLAVE, APDS9960_SLAVE_ADDRESS) < 0) 
	{
        perror("Failed to acquire bus access and/or talk to slave\n");
        exit(1);
    }
	
	// 제스처 감지 센서(APDS9960) 사용을 위한 레지스터 값들 초기화.
    if(!APDS9960_Init()) 
	{
        printf("[APDS-9960(0x39)] I2C Sensor Is Missing\n\n");
        exit(1);
    }	
	
	printf("Check OK!! [APDS-9960(0x39)] I2C Sensor\n\n");
	
	if(!APDS9960_enableProximity(0))
	{
		printf("Enable Proximity Sensor Failed\n");
		exit(1);
	}
	
	// 제스처 감지 센서(APDS9960) 사용모드를 enable로 바꿔주고 관련 parameter 초기화.
	if(!APDS9960_enableGesture())
	{
		printf("Enable Gesture Sensor Failed\n");
		exit(1);
	}

	usleep(7000);

	while(1)
	{
		// 제스처를 읽어와서 계속 출력 해줌.
		APDS9960_printGesture();
		usleep(130000);

		// Proximity 센서에서 읽어온 근접도 값을 계속 출력 해줌 
		// APDS9960_printProximity(); 근접도 값을 보고 싶은 경우 사용
		// microsecond 단위. 1000*1000 microsecond -> 1 second.
		// 1.3초간 sleep(delay). 1.3초 단위로 제스처를 출력해줌.
	}


	return 0;
}
