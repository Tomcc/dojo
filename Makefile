OBJECTS := src/AnimatedQuad.o src/Color.o src/dojomath.o src/Font.o src/FrameSet.o src/Game.o src/GameState.o src/Math.o src/Mesh.o src/Object.o src/ParticlePool.o src/Platform.o src/Renderable.o src/Render.o src/RenderState.o src/ResourceGroup.o src/SoundBuffer.o src/SoundManager.o src/SoundSource.o src/Sprite.o src/Table.o src/Texture.o src/TouchSource.o src/Vector.o src/Viewport.o
CFLAGS  := -I Linux -I include -D LINUX -I /usr/include/AL

dojo: $(OBJECTS)
	$(LD) -o src/$@ $^

%.o: %.cpp
	g++ $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf src/*.o doj
