#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include "mcp3021_ra12p.h"

/* [in] :   fd에 저장된 데이터를 얻기 위해 fd를 불러온다.
 * [out] :  센서값 result를 반환한다.
 * description : 압력 센서 데이터를 Datasheet에 해당하는 방식에 맞게 바꿔주고 result에 저장하는 함수
 */

uint16_t getRA12Pvalue(int mcp3021_fd)
{
	uint8_t data[2] = {0};
	uint16_t result = 0;

	read(mcp3021_fd, data, 2);	

	result = (result | data[0]) << 6;
	data[1] = data[1] >> 2;
	result = result | data[1];

	return result;
}

void mcp3021_Data_Print(int fd){
	printf("RA12P : %hu\n", getRA12Pvalue(fd));

}

int mcp3021_main(void)
{


	if((mcp3021_fd = open("/dev/i2c-2", O_RDWR)) < 0)
	{
		perror("Failed to open i2c-2");
		exit(1);
	}	
	
    if(ioctl(mcp3021_fd, I2C_SLAVE, MCP3021_SLAVE_ADDRESS) < 0) 
	{
        	perror("Failed to acquire bus access and/or talk to slave\n");
       		exit(1);
    	}
	
	printf("Searching For I2C Sensor [MCP3021-RA12P(0x4D)]\n");

	//센서 데이터를 1초마다 출력한다.
	while(1)
	{
		mcp3021_Data_Print(mcp3021_fd);
		usleep(1000000);
	}

	close(mcp3021_fd);

	return 0;
}
