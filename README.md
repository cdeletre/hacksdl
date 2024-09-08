# hacksdl

An **experimental** SDL2 library hook hack. No safety controls are made, segfault can happen if you set crazy values. You have been warned ;)

This hack can swap the indexes of the controllers using `HACKSDL_MAP_INDEX_<n>` env variable where `<n>` is the index you want to change. It allows you for example to use another controller in a game when the game only uses the main controller (index 0).

Also this hack can give light debug info (with `HACKSDL_HINT_DEBUG` env variable) such as if one of the followin function has been called:

int SDL_Init(Uint32 flags)
int SDL_NumJoysticks(void)
int SDL_JoystickGetDevicePlayerIndex(int device_index)
SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
SDL_Joystick* SDL_JoystickOpen(int device_index)
const char* SDL_JoystickNameForIndex(int device_index)
Uint16 SDL_JoystickGetDeviceVendor(int device_index)
Uint16 SDL_JoystickGetDeviceProduct(int device_index)
Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
SDL_GameController* SDL_GameControllerOpen(int joystick_index)
char* SDL_GameControllerMappingForIndex(int mapping_index)
SDL_bool SDL_IsGameController(int joystick_index)
const char* SDL_GameControllerNameForIndex(int joystick_index)
char* SDL_GameControllerMappingForDeviceIndex(int joystick_index)

## Build

```
gcc hacksdl.c -o hacksdl.so -fPIC -shared -lSDL2 -D_GNU_SOURCE
```

Note: build it for the target arch (aarch64, armhf, ...)

## Examples of usage

Swap controller 0 and 1 with debug output
```
export LD_PRELOAD="hacksdl.so"
export HACKSDL_DEBUG=1
export HACKSDL_MAP_INDEX_0=1
export HACKSDL_MAP_INDEX_1=0
some_sdl2_prog
```

Swap controller 0 and 2, one line command

```
LD_PRELOAD="hacksdl.so" HACKSDL_MAP_INDEX_0=2 HACKSDL_MAP_INDEX_2=0 some_sdl2_prog
```