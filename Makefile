.POSIX:
.SUFFIXES:

# Simple POSIX Makefile wrapper for crt_renderer
CMAKE      = cmake
BUILD_DIR  = build
BUILD_TYPE = Release
# separate out each configuration into its own sub‐dir
BUILD_STANDALONE_DIR = $(BUILD_DIR)/standalone
BUILD_PYTHON_DIR     = $(BUILD_DIR)/python
BUILD_BLENDER_DIR    = $(BUILD_DIR)/blender

CMAKE_COMMON_FLAGS = -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# CMake flags for each mode
CMAKE_ARGS_STANDALONE = $(CMAKE_COMMON_FLAGS) \
						-DBUILD_STANDALONE=ON \
                        -DBUILD_PYTHON=OFF \
                        -DBUILD_BLENDER_EXTENSION=OFF
CMAKE_ARGS_PYTHON     = $(CMAKE_COMMON_FLAGS) \
						-DBUILD_STANDALONE=OFF \
                        -DBUILD_PYTHON=ON \
                        -DBUILD_BLENDER_EXTENSION=OFF
CMAKE_ARGS_BLENDER    = $(CMAKE_COMMON_FLAGS) \
					    -DBUILD_STANDALONE=OFF \
                        -DBUILD_PYTHON=ON \
                        -DBUILD_BLENDER_EXTENSION=ON

.PHONY: standalone python blender clean

standalone:
	mkdir -p $(BUILD_STANDALONE_DIR)
	$(CMAKE) -S . -B $(BUILD_STANDALONE_DIR) $(CMAKE_ARGS_STANDALONE)
	$(CMAKE) --build $(BUILD_STANDALONE_DIR)
	@echo
	@echo ">"
	@echo "> Build finished successfully"
	@echo "> Run with ./build/crt_renderer <scene_file> <output_file>"
	@echo ">"

python:
	mkdir -p $(BUILD_PYTHON_DIR)
	$(CMAKE) -S . -B $(BUILD_PYTHON_DIR) $(CMAKE_ARGS_PYTHON)
	$(CMAKE) --build $(BUILD_PYTHON_DIR) --target _crt
	@echo
	@echo ">"
	@echo "> Build finished successfully"
	@echo "> You can import the _crt module from ./build/python"
	@echo ">"

blender:
	mkdir -p $(BUILD_BLENDER_DIR)
	$(CMAKE) -S . -B $(BUILD_BLENDER_DIR) $(CMAKE_ARGS_BLENDER)
	$(CMAKE) --build $(BUILD_BLENDER_DIR) --target crt_blender_extension
	@echo
	@echo ">"
	@echo "> Build finished successfully"
	@echo "> You can install the ZIP archive from ./build/blender from Blender's user preferences"
	@echo ">"

clean:
	rm -rf $(BUILD_DIR)
