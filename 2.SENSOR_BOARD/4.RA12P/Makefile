CC 				= aarch64-linux-gnu-gcc
SRCS            = $(wildcard *.c)
OBJS            = $(SRCS:.c=.o)
TARGET          = ../output/Pressure_ra12p
LIBS            = -lmcp3021_ra12p
LIB_DIRS        = -L.
INC             = -I.
LIB_SRCS = mcp3021_ra12p.c
LIB_OBJS = mcp3021_ra12p.o

LIB_NAME = mcp3021_ra12p
LIB_FULL_NAME = lib$(LIB_NAME).a
 


all : $(LIB_FULL_NAME) $(TARGET)
	$(CC) -o $(TARGET) $(OBJS) $(INC) $(LIB_DIRS) $(LIBS)

$(LIB_FULL_NAME) : 
	$(CC) -c $(LIB_SRCS)
	$(AR) rcv $@ $(LIB_OBJS)
 
$(TARGET) :
	$(CC) -c $(SRCS) $(INC) $(LIB_DIRS) $(LIBS)
 
clean :
	rm -f $(TARGET)
	rm -f *.o
	rm -f *.a
