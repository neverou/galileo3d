all: shader
	mkdir -p build/
	g++ -g -o build/galileo3d src/*.cpp -Iinclude/ -Llib/ -lSDL2 -lpthread -ldl -Wall -Wextra

shader: 
	mkdir -p data/shaders/
	glslc -fshader-stage=vert -o data/shaders/render.vert.spv raw_shaders/render.vert
	glslc -fshader-stage=frag -o data/shaders/render.pix.spv raw_shaders/render.frag

	glslc -fshader-stage=vert -o data/shaders/filter_temporal.vert.spv raw_shaders/filter_temporal.vert
	glslc -fshader-stage=frag -o data/shaders/filter_temporal.pix.spv raw_shaders/filter_temporal.frag	

	glslc -fshader-stage=vert -o data/shaders/filter_spatial.vert.spv raw_shaders/filter_spatial.vert
	glslc -fshader-stage=frag -o data/shaders/filter_spatial.pix.spv raw_shaders/filter_spatial.frag	

	glslc -fshader-stage=vert -o data/shaders/blit.vert.spv raw_shaders/blit.vert
	glslc -fshader-stage=frag -o data/shaders/blit.pix.spv raw_shaders/blit.frag	

	glslc -fshader-stage=vert -o data/shaders/composite.vert.spv raw_shaders/composite.vert
	glslc -fshader-stage=frag -o data/shaders/composite.pix.spv raw_shaders/composite.frag	


	# glslangValidator -G raw_shaders/vertex.vert -o data/shaders/vertex.spv 
	# glslangValidator -G raw_shaders/pixel.frag -o data/shaders/pixel.spv 
clean:
	rm -rf build/*