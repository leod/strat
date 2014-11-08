#CXX=g++ 

INC=-Ilib/enet-1.3.12/include -Ilib/glew-1.11.0/include -Ilib/glfw-3.0.4.bin.WIN32/include -Ilib/glm -Ilib/ -I. -Ilib/entityx-master -Ilib/DevIL/1.7.8/include/ -Ilib/boost_1_57_0
LIB=-Llib/entityx-master -Llib/enet-1.3.12 -Llib/glew-1.11.0/lib -Llib/glfw-3.0.4.bin.WIN32/lib-mingw  -Llib/DevIL/1.7.8/lib/MinGW/Release
CXXFLAGS=--std=c++0x -Wall -O3 $(INC) -DGLEW_STATIC -g

LIBS_GAME=-lglfw3 -lglew32s -lopengl32 -lglu32 -lgdi32 -lenet -lws2_32 -lwinmm -lentityx -lDevIL
LIBS_SERVER=-lglfw3 -lgdi32 -lenet -lws2_32 -lwinmm 

SRCS_COMMON=common/BitStream.cc common/Defs.cc common/GameSettings.cc common/Message.cc common/Order.cc
OBJS_COMMON=$(subst .cc,.o,$(SRCS_COMMON))

SRCS_OPENGL=opengl/Buffer.cc opengl/Error.cc opengl/Framebuffer.cc opengl/OBJ.cc opengl/Program.cc opengl/ProgramManager.cc opengl/Shader.cc opengl/Texture.cc opengl/TextureManager.cc

SRCS_UTIL=util/Log.cc util/Print.cc

SRCS_GAME=game/Client.cc game/Graphics.cc game/Main.cc game/Map.cc game/Math.cc game/Sim.cc game/SimState.cc game/SimSystems.cc game/InterpState.cc game/Input.cc game/Terrain.cc $(SRCS_OPENGL) $(SRCS_UTIL)
OBJS_GAME=$(subst .cc,.o,$(SRCS_GAME))

SRCS_SERVER=server/Server.cc
OBJS_SERVER=$(subst .cc,.o,$(SRCS_SERVER))

all: game server

clean: 
	rm -f $(OBJS_COMMON) $(OBJS_GAME) $(OBJS_SERVER) game.exe server.exe

game:  $(OBJS_COMMON) $(OBJS_GAME)
	$(CXX) $(OBJS_COMMON) $(OBJS_GAME) $(LIB) $(LIBS_GAME) -o game

server:  $(OBJS_COMMON) $(OBJS_SERVER)
	$(CXX) $(OBJS_COMMON) $(OBJS_SERVER) $(LIB) $(LIBS_SERVER) -o server

depend: .depend

.depend: $(SRCS_COMMON) $(SRCS_GAME) $(SRCS_SERVER)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend

include .depend
