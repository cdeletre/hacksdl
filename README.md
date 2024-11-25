# hacksdl

An **experimental** SDL2 library hook hack. No safety controls are made, segfault can happen if you set crazy values. You have been warned ;)

This hack can swap the indexes of the controllers using `HACKSDL_MAP_INDEX_<n>` env variable where `<n>` is the index you want to change. It allows you for example to use another controller in a game when the game only uses the main controller (index 0). Maximum index is 15 (included).

Also this hack can give light debug info (with `HACKSDL_VERBOSE` env variable) such as if one of the following function has been called (non exhaustive, see source code for accurate informations):

- int SDL_Init(Uint32 flags)
- int SDL_NumJoysticks(void)
- int SDL_JoystickGetDevicePlayerIndex(int device_index)
- SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
- SDL_Joystick* SDL_JoystickOpen(int device_index)
- const char* SDL_JoystickNameForIndex(int device_index)
- Uint16 SDL_JoystickGetDeviceVendor(int device_index)
- Uint16 SDL_JoystickGetDeviceProduct(int device_index)
- Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
- SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
- SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
- SDL_GameController* SDL_GameControllerOpen(int joystick_index)
- char* SDL_GameControllerMappingForIndex(int mapping_index)
- SDL_bool SDL_IsGameController(int joystick_index)
- const char* SDL_GameControllerNameForIndex(int joystick_index)
- char* SDL_GameControllerMappingForDeviceIndex(int joystick_index)

Additionnal hack are added such as:
- disabling of controller (by index or all)
- axis digital mode
- axis deadzone
- axis value modifier
- virtual axis
- display and windows size get/set

## Build requirements

```shell
apt install build-essential
apt install libsdl2-dev libconfig-dev
```

## Build

```shell
gcc hacksdl.c configuration.c debug.c /usr/lib/aarch64-linux-gnu/libconfig.a -o hacksdl.aarch64.so -fPIC -shared -lSDL2 -D_GNU_SOURCE
```

Note: build it for the target arch (aarch64, armhf, ...)

## Examples of usage

**Swap controller 0 and 1 with debug output**

```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_VERBOSE=2
export HACKSDL_DEVICE_MAP_INDEX_0=1
export HACKSDL_DEVICE_MAP_INDEX_1=0
./some_sdl2_prog
```

**Enable button X as modifier, dividing by 4 (2^2) the value read for a stick axis LEFTX**

```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_AXIS_MODIFIER_BUTTON=X
export HACKSDL_AXIS_MODIFIER_SHIFT_LEFTX=2
./some_sdl2_prog
```

**Disable all gamepad controller**
```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_NO_GAMECONTROLLER=1
./some_sdl2_prog
```

**Swap controller 0 and 2, one line command**

```shell
LD_PRELOAD="$PWD/hacksdl.aarch64.so" HACKSDL_DEVICE_MAP_INDEX_0=2 HACKSDL_DEVICE_MAP_INDEX_2=0 ./some_sdl2_prog
```

**Disable (mode 1) controller with index 0, one line command**

```shell
LD_PRELOAD="$PWD/hacksdl.aarch64.so" HACKSDL_DEVICE_DISABLE_0=1 ./some_sdl2_prog
```

*mode 1 forces SDL_IsGameController to return False for a disable device*

**Disable (mode 2) controller with index 0, one line command**

```shell
LD_PRELOAD="$PWD/hacksdl.aarch64.so" HACKSDL_DEVICE_DISABLE_0=2 ./some_sdl2_prog
```

*mode 2 does the same as mode 1 but also forces SDL_GameControllerOpen to return NULL for a disable device*

**Use configuration file**

```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_VERBOSE=1
export HACKSDL_CONFIG_FILE="$PWD/hacksdl.conf"
./some_sdl2_prog
```

- hacksdl.conf (example)

Left axis deadzone set to 2048 and modifier on button X
```conf
HACKSDL_VERBOSE="1";
HACKSDL_LIBSDL_NAME="libSDL2-2.0.so.0";
HACKSDL_AXIS_DEADZONE_LEFTX="2048";
HACKSDL_AXIS_DEADZONE_LEFTX="2048";
HACKSDL_AXIS_DIGITAL_LEFTX="1";
HACKSDL_AXIS_DIGITAL_LEFTY="1";
HACKSDL_AXIS_MODIFIER_BUTTON="X";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTX="2";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTY="2";
```

Virtual left axis using DPAD input with modifier on button X and merge disabled (if a real left axis exists it won't work)
```conf
HACKSDL_VERBOSE="1";
HACKSDL_AXIS_MODIFIER_BUTTON="X";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTX="2";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTY="2";
HACKSDL_AXIS_VIRTUAL_MINUS_MAP_LEFTX="DPLEFT";
HACKSDL_AXIS_VIRTUAL_PLUS_MAP_LEFTX="DPRIGHT";
HACKSDL_AXIS_VIRTUAL_MINUS_MAP_LEFTY="DPUP";
HACKSDL_AXIS_VIRTUAL_PLUS_MAP_LEFTY="DPDOWN";
HACKSDL_AXIS_VIRTUAL_MERGE_LEFTX="0";
HACKSDL_AXIS_VIRTUAL_MERGE_LEFTY="0";
```

Virtual left axis using DPAD input with modifier on button X, merge is enabled by default (if a real left axis exists it can be used as well). Virtual DPAD range is reduced to 0-8192.
```conf
HACKSDL_VERBOSE="1";
HACKSDL_AXIS_MODIFIER_BUTTON="X";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTX="1";
HACKSDL_AXIS_MODIFIER_SHIFT_LEFTY="1";
HACKSDL_AXIS_VIRTUAL_MINUS_MAP_LEFTX="DPLEFT";
HACKSDL_AXIS_VIRTUAL_PLUS_MAP_LEFTX="DPRIGHT";
HACKSDL_AXIS_VIRTUAL_MINUS_MAP_LEFTY="DPUP";
HACKSDL_AXIS_VIRTUAL_PLUS_MAP_LEFTY="DPDOWN";
HACKSDL_AXIS_VIRTUAL_MAX_LEFTX="8192";
HACKSDL_AXIS_VIRTUAL_MAX_LEFTY="8192";
```


Change display resolution detected and force windows size
```conf
HACKSDL_VERBOSE="1";
HACKSDL_LIBSDL_NAME="libSDL2-2.0.so.0";
HACKSDL_GET_DISPLAY_MODE_W="641";
HACKSDL_GET_DISPLAY_MODE_H="480";
HACKSDL_SET_WINDOW_SIZE_W="640";
HACKSDL_SET_WINDOW_SIZE_H="480";
```