# Nitrokey Update Tool

Nitrokey Update Tool is an application aiming to ease Nitrokey Storage firmware update process under Windows and macOS.

## Installation and downloads
Ready to use packages and install instructions are available releases' page: https://github.com/Nitrokey/nitrokey-update-tool/releases/

## Usage

To flash the Nitrokey Storage device please prepare:

- a .hex file with target firmware
- Windows or macOS PC
- Nitrokey Storage device with enabled update mode

#### Steps
After running the tool please:
1. Select the firmware file location with `Select firmware file` button
2. Check whether device has been detected by observing `Device in update mode connected`
3. If both conditions are fulfilled, `Update firmware` button should be active
4. After pressing `Update firmware` the procedure would start.

#### Important
Updating process should not be interrupted, otherwise the device might not work correctly or not power up at all. Please make sure your PC is connected to stable power supply.

## Compilation
Requirements:
- libusb 1.0
- C++11 compatible compiler
- CMake 3 (for building underlying `dfu-programmer` lib)
- Qt 5

### Getting the sources
Please make sure you have downloaded the code with the submodules, eg.:
```bash
git clone https://github.com/Nitrokey/nitrokey-update-tool.git --recursive
```

### Compiling on Ubuntu Linux 18.04
#### Packages list

Prerequisites for building on Ubuntu 17.10:
- `build-essential` - for building applications
- `cmake` - for compiling libnitrokey
- `qt5-default` - QT5 library
- `libusb-1.0-0-dev` - library to communicate with USB devices

#### dfu-programmer library compilation
Before the actual Tool building, a `dfu-programmer` library needs to be built. To do so, please execute (while being in the main directory):
```bash
mkdir -p 3rdparty/dfu-programmer/build
pushd 3rdparty/dfu-programmer/build
cmake ..
make
popd
```
This should result in a static object.

#### Tool compilation
To compile the tool, please execute (while being in the main directory):
```bash
mkdir -p build && cd build
qmake .. && make 
```

### Compiling on macOS 10.12
Similarly to Ubuntu instructions. `libusb` library could be provided with `brew.sh`.

### Cross-compiling
Cross-compiling works similarly as with Nitrokey App (via MXE). See its README file for details.