# OCA Sample Project

## Description
This project is a sample OpenCV-based application that processes image files. The source code is written in C++ and uses CMake as the build system. The application expects image files from the `resources/` folder to be placed in the same directory as the executable.

## Build Instructions

### Building with Yocto SDK
If you are using a Yocto-based toolchain:

1. **Source the Yocto SDK environment**
   ```bash
   source /path/to/yocto/environment-setup-aarch64-poky-linux
   ```
   Adjust the `environment-setup-*` file based on your target architecture.

2. **Build with CMake**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=${OE_CMAKE_TOOLCHAIN_FILE}
   make
   ```

3. **Deploy and Run**
   Copy the executable and required image files to the target device:
   ```bash
   scp oca_sample user@target_device:/home/root/
   scp ../resources/*.png user@target_device:/home/root/
   ```
   SSH into the target device and run:
   ```bash
   ./oca_sample
   ```

## Notes
- Ensure the images from the `resources/` folder are placed in the same directory as the executable before running it.
- If you encounter missing OpenCV libraries, check the installation or update the `CMakeLists.txt` to set `OpenCV_DIR` manually.

## License
Refer to the `LICENSE` file for licensing details.

