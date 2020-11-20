TARGET_NAME = fbw

CXX = g++

# You may need to change -std=c++11 to -std=c++0x if your compiler is a bit older
CXXFLAGS = -std=c++11 

OBJS := main.o ImageWin.o ProcessEvents.o imagelib.o inifile/inifile.o

SDL_PATH = -I/usr/local/include
INCLUDE_PATH = -I../include $(SDL_PATH)
CFLAGS += $(INCLUDE_PATH) -g -Wno-switch -Wall -c

LIB_PATH = -L/usr/local/lib
LIB_LINK = -Bstatic -lSDL2 
LFLAGS += $(LIB_PATH) $(LIB_LINK)

all:	$(TARGET_NAME)

$(TARGET_NAME):	$(OBJS) 
	$(CXX) $^ -o $@ $(LFLAGS)	

$(OBJS):%o:%cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<


clean:
	rm -fr $(TARGET_NAME) $(OBJS) 
