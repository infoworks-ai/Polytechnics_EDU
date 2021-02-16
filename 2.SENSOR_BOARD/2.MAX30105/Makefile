CC 				= aarch64-linux-gnu-gcc
SRCS            = $(wildcard *.c)
OBJS            = $(SRCS:.c=.o)
TARGET          = ../output/Smokedetection_MAX30105
LIBS            = -lMAX30105
LIB_DIRS        = -L.
INC             = -I.
LIB_SRCS = MAX30105.c
LIB_OBJS = MAX30105.o

LIB_NAME = MAX30105
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
