#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_gamecontroller.h>

#define HACKSDL_HINT_MAP_INDEX_                 "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_MAP_INDEX_MAX                   16
#define HACKSDL_HINT_DEBUG                      "HACKSDL_DEBUG"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_MODIFIER_BUTTON            "HACKSDL_MODIFIER_BUTTON"
#define HACKSDL_HINT_MODIFIER_SHIFT             "HACKSDL_MODIFIER_SHIFT"

int initialized = 0;
int debug = 0;
int no_gamecontroller = 0;
SDL_GameControllerButton modifier_button = SDL_CONTROLLER_BUTTON_INVALID;
int modifier_shift = 1;
int index_mapping[HACKSDL_MAP_INDEX_MAX];

void HACKSDL_debug(char* format, ...){
    va_list args;

    if(debug)
    {
        fprintf(stdout,"[HACKSDL] ");
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        fprintf(stdout,"\n");
        fflush(stdout);
    }

}

SDL_GameControllerButton get_gc_button(const char* modifier_button_hint)
{
    if(modifier_button_hint == NULL)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON undefined");
        return SDL_CONTROLLER_BUTTON_INVALID;
    }
    else if(strcmp(modifier_button_hint,"A") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = A");
        return SDL_CONTROLLER_BUTTON_A;

    }
    else if(strcmp(modifier_button_hint,"B") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = B");
        return SDL_CONTROLLER_BUTTON_B;
    }
    else if(strcmp(modifier_button_hint,"X") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = X");
        return SDL_CONTROLLER_BUTTON_X;
    }
    else if(strcmp(modifier_button_hint,"Y") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = Y");
        return SDL_CONTROLLER_BUTTON_Y;
    }
    else if(strcmp(modifier_button_hint,"BACK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = BACK");
        return SDL_CONTROLLER_BUTTON_BACK;
    }
    else if(strcmp(modifier_button_hint,"GUIDE") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = GUIDE");
        return SDL_CONTROLLER_BUTTON_GUIDE;
    }
    else if(strcmp(modifier_button_hint,"START") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = START");
        return SDL_CONTROLLER_BUTTON_START;
    }
    else if(strcmp(modifier_button_hint,"LEFTSTICK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = LEFTSTICK");
        return SDL_CONTROLLER_BUTTON_LEFTSTICK;
    }
    else if(strcmp(modifier_button_hint,"RIGHTSTICK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = RIGHTSTICK");
        return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    }
    else if(strcmp(modifier_button_hint,"LEFTSHOULDER") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = LEFTSHOULDER");
        return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    }
    else if(strcmp(modifier_button_hint,"RIGHTSHOULDER") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = RIGHTSHOULDER");
        return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    }
    else
    {
        HACKSDL_debug("incorrect HACKSDL_MODIFIER_BUTTON: %s", modifier_button_hint);
        return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

void initialize()
{
    if(initialized)
    {
        return;
    }

    int integer_value;

    debug = SDL_GetHintBoolean(HACKSDL_HINT_DEBUG,SDL_FALSE);

    HACKSDL_debug("Initializing");

    // joystick index remapping

    char buffer[32];
    int new_index;

    for(int index=0; index<HACKSDL_MAP_INDEX_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_MAP_INDEX_,index);

        const char *hint = SDL_GetHint(buffer);
        if (hint != NULL){
            errno = 0;
            new_index = (int)strtol(hint,NULL,10);

            if (errno != 0){
                HACKSDL_debug("incorrect mapping %s: %s",buffer, hint);
                new_index = index;
            }else{
                HACKSDL_debug("map index %d to %d", index, new_index);
            }
        }else{
            HACKSDL_debug("no mapping set for index %d", index);
            new_index = index;
        }
        index_mapping[index] = new_index;
    }

    // no gamecontroller hack
    no_gamecontroller = SDL_GetHintBoolean(HACKSDL_HINT_NO_GAMECONTROLLER,SDL_FALSE);
    if(no_gamecontroller)
    {
        HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = True");
    }
    else
    {
        HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = False");
    }

    // modifier hack
    const char *modifier_shift_hint = SDL_GetHint(HACKSDL_HINT_MODIFIER_SHIFT);

    errno = 0;
    modifier_shift = (int)strtol(modifier_shift_hint,NULL,10);
    if (modifier_shift < 0 || errno != 0){
        HACKSDL_debug("incorrect HACKSDL_MODIFIER_SHIFT: %s", modifier_shift_hint);
        modifier_shift = 0;
    }
    
    if (modifier_shift > 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_SHIFT = %d", modifier_shift);
        modifier_button = get_gc_button(SDL_GetHint(HACKSDL_HINT_MODIFIER_BUTTON));
    }
    else
    {
        HACKSDL_debug("HACKSDL_MODIFIER_SHIFT disabled");
    }

    HACKSDL_debug("Initialization done");
    initialized = 1;
}

char* get_axis_name(SDL_GameControllerAxis axis)
{
    char* axis_name;
    switch (axis)
    {
    case SDL_CONTROLLER_AXIS_INVALID:
        axis_name = "SDL_CONTROLLER_AXIS_INVALID";
        break;
    case SDL_CONTROLLER_AXIS_LEFTX:
        axis_name = "SDL_CONTROLLER_AXIS_LEFTX";
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        axis_name = "SDL_CONTROLLER_AXIS_LEFTY";
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        axis_name = "SDL_CONTROLLER_AXIS_RIGHTX";
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        axis_name = "SDL_CONTROLLER_AXIS_RIGHTY";
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        axis_name = "SDL_CONTROLLER_AXIS_TRIGGERLEFT";
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        axis_name = "SDL_CONTROLLER_AXIS_TRIGGERRIGHT";
        break;
    case SDL_CONTROLLER_AXIS_MAX:
        axis_name = "SDL_CONTROLLER_AXIS_MAX";
        break;
    default:
        axis_name = "UNKNOWN";
        break;
    }

    return axis_name;
}

int HACKSDL_map_index(int index)
{
    return index_mapping[index];
}

int HACKSDL_map_index_old(int index)
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
    initialize();
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

    return new_SDL_GameControllerOpen(HACKSDL_map_index(joystick_index));;
}

char* SDL_GameControllerMappingForIndex(int mapping_index)
{
    char* (*new_SDL_GameControllerMappingForIndex)(int mapping_index);
    new_SDL_GameControllerMappingForIndex = dlsym(RTLD_NEXT, "SDL_GameControllerMappingForIndex");

    HACKSDL_debug("hook on SDL_GameControllerMappingForIndex(%d)", mapping_index);
    
    return new_SDL_GameControllerMappingForIndex(HACKSDL_map_index(mapping_index));
}

SDL_bool SDL_IsGameController(int joystick_index)
{
    SDL_bool (*new_SDL_IsGameController)(int joystick_index);
    new_SDL_IsGameController = dlsym(RTLD_NEXT, "SDL_IsGameController");

    HACKSDL_debug("hook on SDL_IsGameController(%d)", joystick_index);
    
    if(SDL_GetHintBoolean(HACKSDL_HINT_NO_GAMECONTROLLER,SDL_FALSE)){
        return SDL_FALSE;
    }else{
        return new_SDL_IsGameController(HACKSDL_map_index(joystick_index));
    }
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

void SDL_GameControllerUpdate(void)
{
    void (*new_SDL_GameControllerUpdate)(void);
    new_SDL_GameControllerUpdate = dlsym(RTLD_NEXT, "SDL_GameControllerUpdate");

    HACKSDL_debug("hook on SDL_GameControllerUpdate()");

    new_SDL_GameControllerUpdate();
}

Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis)
{

    Sint16 axis_value = 0;
    Uint8 button_pressed = 0;

    Sint16 (*new_SDL_GameControllerGetAxis)(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis);
    new_SDL_GameControllerGetAxis = dlsym(RTLD_NEXT, "SDL_GameControllerGetAxis");

    axis_value = new_SDL_GameControllerGetAxis(gamecontroller, axis);

    HACKSDL_debug("hook on SDL_GameControllerGetAxis(%s,%s), value=%d (Sint16)", SDL_GameControllerName(gamecontroller), get_axis_name(axis), axis_value);

    if(modifier_shift > 0)
    {
        button_pressed = SDL_GameControllerGetButton(gamecontroller, modifier_button);

        if (button_pressed == 1)
        {
            axis_value = axis_value >> modifier_shift;
            HACKSDL_debug("modifier enabled new value=%d (Sint16)", axis_value);
        }
    }

    return axis_value;

}