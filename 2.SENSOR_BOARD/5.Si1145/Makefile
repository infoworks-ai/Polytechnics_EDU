CC 				= aarch64-linux-gnu-gcc
SRCS            = $(wildcard *.c)
OBJS            = $(SRCS:.c=.o)
TARGET          = ../output/ProximityUVAmbient_Si1145
LIBS            = -lSi1145
LIB_DIRS        = -L.
INC             = -I.
LIB_SRCS = Si1145.c
LIB_OBJS = Si1145.o

LIB_NAME = Si1145
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
