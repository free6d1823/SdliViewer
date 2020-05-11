TARGET_NAME = imv

CXX = g++

# You may need to change -std=c++11 to -std=c++0x if your compiler is a bit older
CXXFLAGS = -std=c++11 

OBJS := main.o ImageWin.o ProcessEvents.o common.o

SDL_PATH = -I/usr/local/include
INCLUDE_PATH = -I../include $(SDL_PATH)
CFLAGS += $(INCLUDE_PATH) -g -Wno-switch -Wall -c

LIB_PATH = -L/usr/local/lib
LIB_LINK = -lSDL2 
LFLAGS += $(LIB_PATH) $(LIB_LINK)

all:	$(TARGET_NAME)
	cp $(TARGET_NAME) ../bin/.

$(TARGET_NAME):	$(OBJS) 
	$(CXX) $^ -o $@ $(LFLAGS)	

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr $(TARGET_NAME) $(OBJS) 
