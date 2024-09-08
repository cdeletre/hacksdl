#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_gamecontroller.h>

#define HACKSDL_HINT_MAP_INDEX_    "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_HINT_DEBUG         "HACKSDL_DEBUG"


void HACKSDL_debug(char* format, ...){
    va_list args;

    if(SDL_GetHintBoolean(HACKSDL_HINT_DEBUG,SDL_FALSE))
    {
        fprintf(stdout,"[HACKSDL] ");
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        fprintf(stdout,"\n");
    }

}

int HACKSDL_map_index(int index)
{

    char buffer[32];
    int new_index;
    
    sprintf(buffer,HACKSDL_HINT_MAP_INDEX_,index);

    const char *hint = SDL_GetHint(buffer);
    if (hint != NULL){
        errno = 0;
        new_index = (int)strtol(hint,NULL,10);

        if (errno != 0){
            HACKSDL_debug("incorrect mapping: %s", hint);
            new_index = index;
        }else{
            HACKSDL_debug("map index %d to %d", index, new_index);
        }
    }else{
        HACKSDL_debug("no mapping set for index %d", index);
        new_index = index;
    }
    
    return new_index;
}

int SDL_Init(Uint32 flags)
{
    int (*new_SDL_Init)(Uint32 flags);
    new_SDL_Init = dlsym(RTLD_NEXT, "SDL_Init");

    HACKSDL_debug("hook on SDL_Init");

    return new_SDL_Init(flags);
}

int SDL_NumJoysticks(void)
{
    int (*new_SDL_NumJoysticks)(void);
    new_SDL_NumJoysticks = dlsym(RTLD_NEXT, "SDL_NumJoysticks");

    HACKSDL_debug("hook on SDL_NumJoysticks");

    return new_SDL_NumJoysticks();
}

int SDL_JoystickGetDevicePlayerIndex(int device_index)
{
    int (*new_SDL_JoystickGetDevicePlayerIndex)(int device_index);
    new_SDL_JoystickGetDevicePlayerIndex = dlsym(RTLD_NEXT, "SDL_JoystickGetDevicePlayerIndex");

    HACKSDL_debug("hook on SDL_JoystickGetDevicePlayerIndex(%d)", device_index);

    return new_SDL_JoystickGetDevicePlayerIndex(HACKSDL_map_index(device_index));
}

SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID (*new_SDL_JoystickGetDeviceGUID)(int device_index);
    new_SDL_JoystickGetDeviceGUID = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceGUID");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceGUID(%d)", device_index);

    return new_SDL_JoystickGetDeviceGUID(HACKSDL_map_index(device_index));

}

SDL_Joystick* SDL_JoystickOpen(int device_index)
{
    SDL_Joystick* (*new_SDL_JoystickOpen)(int device_index);
    new_SDL_JoystickOpen = dlsym(RTLD_NEXT, "SDL_JoystickOpen");

    HACKSDL_debug("hook on SDL_JoystickOpen(%d)", device_index);
    
    return new_SDL_JoystickOpen(HACKSDL_map_index(device_index));
}

const char* SDL_JoystickNameForIndex(int device_index)
{
    const char* (*new_SDL_JoystickNameForIndex)(int device_index);
    new_SDL_JoystickNameForIndex = dlsym(RTLD_NEXT, "SDL_JoystickNameForIndex");

    HACKSDL_debug("hook on SDL_JoystickNameForIndex(%d)", device_index);
    
    return new_SDL_JoystickNameForIndex(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    Uint16 (*new_SDL_JoystickGetDeviceVendor)(int device_index);
    new_SDL_JoystickGetDeviceVendor = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceVendor");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceVendor(%d)", device_index);
    
    return new_SDL_JoystickGetDeviceVendor(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    Uint16 (*new_SDL_JoystickGetDeviceProduct)(int device_index);
    new_SDL_JoystickGetDeviceProduct = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceProduct");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceProduct(%d)", device_index);
    
    return new_SDL_JoystickGetDeviceProduct(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    Uint16 (*new_SDL_JoystickGetDeviceProductVersion)(int device_index);
    new_SDL_JoystickGetDeviceProductVersion = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceProductVersion");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceProductVersion(%d)", device_index);
    
    return new_SDL_JoystickGetDeviceProductVersion(HACKSDL_map_index(device_index));
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    SDL_JoystickType (*new_SDL_JoystickGetDeviceType)(int device_index);
    new_SDL_JoystickGetDeviceType = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceType");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceType(%d)", device_index);
    
    return new_SDL_JoystickGetDeviceType(HACKSDL_map_index(device_index));
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    SDL_JoystickID (*new_SDL_JoystickGetDeviceInstanceID)(int device_index);
    new_SDL_JoystickGetDeviceInstanceID = dlsym(RTLD_NEXT, "SDL_JoystickGetDeviceInstanceID");

    HACKSDL_debug("hook on SDL_JoystickGetDeviceInstanceID(%d)", device_index);
    
    return new_SDL_JoystickGetDeviceInstanceID(HACKSDL_map_index(device_index));
}

SDL_GameController* SDL_GameControllerOpen(int joystick_index)
{
    SDL_GameController* (*new_SDL_GameControllerOpen)(int joystick_index);
    new_SDL_GameControllerOpen = dlsym(RTLD_NEXT, "SDL_GameControllerOpen");

    HACKSDL_debug("hook on SDL_GameControllerOpen(%d)", joystick_index);
    
    return new_SDL_GameControllerOpen(HACKSDL_map_index(joystick_index));
}

char* SDL_GameControllerMappingForIndex(int mapping_index)
{
    char* (*new_SDL_GameControllerMappingForIndex)(int mapping_index);
    new_SDL_GameControllerMappingForIndex = dlsym(RTLD_NEXT, "SDL_GameControllerMappingForIndex");

    HACKSDL_debug("hook on SDL_GameControllerMappingForIndex(%d)\n", mapping_index);
    
    return new_SDL_GameControllerMappingForIndex(HACKSDL_map_index(mapping_index));
}

SDL_bool SDL_IsGameController(int joystick_index)
{
    SDL_bool (*new_SDL_IsGameController)(int joystick_index);
    new_SDL_IsGameController = dlsym(RTLD_NEXT, "SDL_IsGameController");

    HACKSDL_debug("hook on SDL_IsGameController(%d)", joystick_index);
    
    return new_SDL_IsGameController(HACKSDL_map_index(joystick_index));
}

const char* SDL_GameControllerNameForIndex(int joystick_index)
{
    const char* (*new_SDL_GameControllerNameForIndex)(int joystick_index);
    new_SDL_GameControllerNameForIndex = dlsym(RTLD_NEXT, "SDL_GameControllerNameForIndex");

    HACKSDL_debug("hook on SDL_GameControllerNameForIndex(%d)", joystick_index);
    
    return new_SDL_GameControllerNameForIndex(HACKSDL_map_index(joystick_index));
}

char* SDL_GameControllerMappingForDeviceIndex(int joystick_index)
{
    char* (*new_SDL_GameControllerMappingForDeviceIndex)(int joystick_index);
    new_SDL_GameControllerMappingForDeviceIndex = dlsym(RTLD_NEXT, "SDL_GameControllerMappingForDeviceIndex");

    HACKSDL_debug("hook on SDL_GameControllerMappingForDeviceIndex(%d)", joystick_index);
    
    return new_SDL_GameControllerMappingForDeviceIndex(HACKSDL_map_index(joystick_index));
}