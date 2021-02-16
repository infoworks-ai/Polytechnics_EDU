CC 				= aarch64-linux-gnu-gcc
SRCS            = $(wildcard *.c)
OBJS            = $(SRCS:.c=.o)
TARGET          = ../output/HumidityTemperature_SHT35-DIS
LIBS            = -lSHT35-DIS
LIB_DIRS        = -L.
INC             = -I.
LIB_SRCS = SHT35-DIS.c
LIB_OBJS = SHT35-DIS.o

LIB_NAME = SHT35-DIS
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
