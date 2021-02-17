CC 				= aarch64-linux-gnu-gcc
SRCS            = $(wildcard *.c)
OBJS            = $(SRCS:.c=.o)
TARGET          = ../output/GestureDetection_APDS9960
LIBS            = -lAPDS9960
LIB_DIRS        = -L.
INC             = -I.
LIB_SRCS = APDS9960_Common.c APDS9960_Gesture.c APDS9960_Main.c APDS9960_Proximity.c
LIB_OBJS = APDS9960_Common.o APDS9960_Gesture.o APDS9960_Main.o APDS9960_Proximity.o

LIB_NAME = APDS9960
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
