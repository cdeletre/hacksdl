# hacksdl

An **experimental** SDL2 library hook hack. No safety controls are made, segfault can happen if you set crazy values. You have been warned ;)

This hack can swap the indexes of the controllers using `HACKSDL_MAP_INDEX_<n>` env variable where `<n>` is the index you want to change. It allows you for example to use another controller in a game when the game only uses the main controller (index 0). Maximum index is 15 (included).

Also this hack can give light debug info (with `HACKSDL_HINT_DEBUG` env variable) such as if one of the following function has been called (non exhaustive, see source code for accurate informations):

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
- disabling of controller
- axis digital mode
- stick value modifier

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
export HACKSDL_MAP_INDEX_0=1
export HACKSDL_MAP_INDEX_1=0
./some_sdl2_prog
```

**Enable button X as modifier, dividing by 4 (2^2) the value read for a stick axis 0**

```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_MODIFIER_BUTTON=X
export HACKSDL_MODIFIER_SHIFT_0=2
./some_sdl2_prog
```

**Disable gamepad controller**
```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_NO_GAMECONTROLLER=1
./some_sdl2_prog
```

**Swap controller 0 and 2, one line command**

```shell
LD_PRELOAD="$PWD/hacksdl.aarch64.so" HACKSDL_MAP_INDEX_0=2 HACKSDL_MAP_INDEX_2=0 ./some_sdl2_prog
```

**Use configuration file**

```shell
export LD_PRELOAD="$PWD/hacksdl.aarch64.so"
export HACKSDL_VERBOSE=1
export HACKSDL_CONFIG_FILE="$PWD/hacksdl.conf"
./some_sdl2_prog
```

- hacksdl.conf (example)
```conf
HACKSDL_VERBOSE="1";
HACKSDL_LIBSDL_NAME="libSDL2-2.0.so.0";
HACKSDL_AXIS_DIGITAL_4="1";
HACKSDL_AXIS_DIGITAL_5="1";
HACKSDL_AXIS_THRESHOLD_5="4096";
HACKSDL_AXIS_MIN_5="0";
HACKSDL_AXIS_MAX_5="20000";
HACKSDL_MODIFIER_BUTTON=X
HACKSDL_MODIFIER_SHIFT_0=2
```