#define MCP3021_SLAVE_ADDRESS 0x4D

#include <stdint.h>

int mcp3021_fd;

uint16_t getRA12Pvalue(int fd);
int mcp3021_main(void);
void mcp3021_Data_Print(int fd);