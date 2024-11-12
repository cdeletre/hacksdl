#include <errno.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include "configuration.h"
#include "debug.h"

int initialized = 0;

/*
    Original SDL2 handler
*/
void *sdl_handler = RTLD_NEXT;

extern hacksdl_config_t config;

/*
    Original hooked function
*/
int (*original_SDL_Init)(Uint32 flags);
int (*original_SDL_NumJoysticks)(void);

// index related functions
int (*original_SDL_JoystickGetDevicePlayerIndex)(int device_index);
SDL_JoystickGUID (*original_SDL_JoystickGetDeviceGUID)(int device_index);
SDL_Joystick* (*original_SDL_JoystickOpen)(int device_index);
const char* (*original_SDL_JoystickNameForIndex)(int device_index);
Uint16 (*original_SDL_JoystickGetDeviceVendor)(int device_index);
Uint16 (*original_SDL_JoystickGetDeviceProduct)(int device_index);
Uint16 (*original_SDL_JoystickGetDeviceProductVersion)(int device_index);
SDL_JoystickType (*original_SDL_JoystickGetDeviceType)(int device_index);
SDL_JoystickID (*original_SDL_JoystickGetDeviceInstanceID)(int device_index);
SDL_GameController* (*original_SDL_GameControllerOpen)(int joystick_index);
char* (*original_SDL_GameControllerMappingForIndex)(int mapping_index);
SDL_bool (*original_SDL_IsGameController)(int joystick_index);
const char* (*original_SDL_GameControllerNameForIndex)(int joystick_index);
char* (*original_SDL_GameControllerMappingForDeviceIndex)(int joystick_index);

// Value reading functions
Sint16 (*original_SDL_GameControllerGetAxis)(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis);

/*
    Dynamic Library open for original SDL functions
*/
int setup_original_SDL_functions(){

    if(strcmp(config.libsdl_name,"RTLD_NEXT") != 0)
    {
        sdl_handler = dlopen(config.libsdl_name, RTLD_LAZY);
        if (!sdl_handler) {
            HACKSDL_error("Error loading SDL2 library (%s): %s", config.libsdl_name, dlerror());
            return 0;
        }
    }

    original_SDL_Init = dlsym(sdl_handler, "SDL_Init");
    original_SDL_NumJoysticks = dlsym(sdl_handler, "SDL_NumJoysticks");
    original_SDL_JoystickGetDevicePlayerIndex = dlsym(sdl_handler, "SDL_JoystickGetDevicePlayerIndex");
    original_SDL_JoystickGetDeviceGUID = dlsym(sdl_handler, "SDL_JoystickGetDeviceGUID");
    original_SDL_JoystickOpen = dlsym(sdl_handler, "SDL_JoystickOpen");
    original_SDL_JoystickNameForIndex = dlsym(sdl_handler, "SDL_JoystickNameForIndex");
    original_SDL_JoystickGetDeviceVendor = dlsym(sdl_handler, "SDL_JoystickGetDeviceVendor");
    original_SDL_JoystickGetDeviceProduct = dlsym(sdl_handler, "SDL_JoystickGetDeviceProduct");
    original_SDL_JoystickGetDeviceProductVersion = dlsym(sdl_handler, "SDL_JoystickGetDeviceProductVersion");
    original_SDL_JoystickGetDeviceType = dlsym(sdl_handler, "SDL_JoystickGetDeviceType");
    original_SDL_JoystickGetDeviceInstanceID = dlsym(sdl_handler, "SDL_JoystickGetDeviceInstanceID");
    original_SDL_GameControllerOpen = dlsym(sdl_handler, "SDL_GameControllerOpen");
    original_SDL_GameControllerMappingForIndex = dlsym(sdl_handler, "SDL_GameControllerMappingForIndex");
    original_SDL_IsGameController = dlsym(sdl_handler, "SDL_IsGameController");
    original_SDL_GameControllerNameForIndex = dlsym(sdl_handler, "SDL_GameControllerNameForIndex");
    original_SDL_GameControllerMappingForDeviceIndex = dlsym(sdl_handler, "SDL_GameControllerMappingForDeviceIndex");

    original_SDL_GameControllerGetAxis = dlsym(sdl_handler, "SDL_GameControllerGetAxis");

}


/*
    initialize: setup the hack
*/
int initialize()
{
    if(initialized)
    {
        return 1;
    }

    load_config();

    if(setup_original_SDL_functions() == 0)
    {
        HACKSDL_error("Cannot setup SDL hooks");
        return 0;
    }

    HACKSDL_info("Initialization done");

    initialized = 1;
}

/*
    get_axis_name: Get the SDL axis name (char*)
*/
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
/*
    HACKSDL_map_index: return the hacked index for a device
*/
int HACKSDL_map_index(int index)
{
    return config.index_mapping[index];
}

/*
    Functions hooks
*/

int SDL_Init(Uint32 flags)
{
    initialize();

    HACKSDL_debug("Hook: flags = %d", flags);

    return original_SDL_Init(flags);
}

int SDL_NumJoysticks(void)
{
    if(config.no_gamecontroller == 1)
    {       
        HACKSDL_debug("Hook + hack: return 0");
        return 0;
    }
    else
    {
        int result = original_SDL_NumJoysticks();
        HACKSDL_debug("Hook: return = %d", result);
        return result;
    }
}

int SDL_JoystickGetDevicePlayerIndex(int device_index)
{
    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDevicePlayerIndex(HACKSDL_map_index(device_index));
}

SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceGUID(HACKSDL_map_index(device_index));

}

SDL_Joystick* SDL_JoystickOpen(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }
    
    return original_SDL_JoystickOpen(HACKSDL_map_index(device_index));
}

const char* SDL_JoystickNameForIndex(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickNameForIndex(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceVendor(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceProduct(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceProductVersion(HACKSDL_map_index(device_index));
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceType(HACKSDL_map_index(device_index));
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{

    if(HACKSDL_map_index(device_index) != device_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", device_index, HACKSDL_map_index(device_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", device_index);
    }

    return original_SDL_JoystickGetDeviceInstanceID(HACKSDL_map_index(device_index));
}

SDL_GameController* SDL_GameControllerOpen(int joystick_index)
{    

    if(HACKSDL_map_index(joystick_index) != joystick_index){
        HACKSDL_debug("Hook + hack: joystick_index %d -> %d)", joystick_index, HACKSDL_map_index(joystick_index));
    }
    else
    {
        HACKSDL_debug("Hook: device_index=%d", joystick_index);
    }

    return original_SDL_GameControllerOpen(HACKSDL_map_index(joystick_index));;
}

char* SDL_GameControllerMappingForIndex(int mapping_index)
{

    if(HACKSDL_map_index(mapping_index) != mapping_index){
        HACKSDL_debug("Hook + hack: device_index %d -> %d)", mapping_index, HACKSDL_map_index(mapping_index));
    }
    else
    {
        HACKSDL_debug("Hook: mapping_index=%d", mapping_index);
    }
    
    return original_SDL_GameControllerMappingForIndex(HACKSDL_map_index(mapping_index));
}

SDL_bool SDL_IsGameController(int joystick_index)
{

    if(config.no_gamecontroller){
        HACKSDL_debug("Hook + hack: return false for joystick_index=%d", joystick_index);
        return SDL_FALSE;
    }else{
        if(HACKSDL_map_index(joystick_index) != joystick_index){
            HACKSDL_debug("Hook + hack: joystick_index %d -> %d)", joystick_index, HACKSDL_map_index(joystick_index));
        }
        else
        {
            HACKSDL_debug("Hook: joystick_index=%d", joystick_index);
        }
        return original_SDL_IsGameController(HACKSDL_map_index(joystick_index));
    }
}

const char* SDL_GameControllerNameForIndex(int joystick_index)
{

    if(HACKSDL_map_index(joystick_index) != joystick_index){
        HACKSDL_debug("Hook + hack: joystick_index %d -> %d)", joystick_index, HACKSDL_map_index(joystick_index));
    }
    else
    {
        HACKSDL_debug("Hook: joystick_index=%d", joystick_index);
    }
    
    return original_SDL_GameControllerNameForIndex(HACKSDL_map_index(joystick_index));
}

char* SDL_GameControllerMappingForDeviceIndex(int joystick_index)
{

    if(HACKSDL_map_index(joystick_index) != joystick_index){
        HACKSDL_debug("Hook + hack: joystick_index %d -> %d)", joystick_index, HACKSDL_map_index(joystick_index));
    }
    else
    {
        HACKSDL_debug("Hook: joystick_index=%d", joystick_index);
    }
    
    return original_SDL_GameControllerMappingForDeviceIndex(HACKSDL_map_index(joystick_index));
}

void SDL_GameControllerUpdate(void)
{
    void (*original_SDL_GameControllerUpdate)(void);
    original_SDL_GameControllerUpdate = dlsym(RTLD_NEXT, "SDL_GameControllerUpdate");

    HACKSDL_debug("hook on SDL_GameControllerUpdate()");

    original_SDL_GameControllerUpdate();
}

Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis)
{

    Sint16 axis_value = 0;
    Uint8 button_pressed = 0;

    axis_value = original_SDL_GameControllerGetAxis(gamecontroller, axis);

    HACKSDL_debug("Hook gamecontroller=%s axis=%s value=%d", SDL_GameControllerName(gamecontroller), get_axis_name(axis), axis_value);

    if(config.modifier_shift[axis] != 0)
    {
        button_pressed = SDL_GameControllerGetButton(gamecontroller, config.modifier_button);

        if (button_pressed == 1)
        {
            axis_value = axis_value >> config.modifier_shift[axis];
            HACKSDL_debug("Modifier enabled new value=%d", axis_value);
        }
    }
    
    if(config.axis_deadzone[axis] > 0)
    {
        if(axis_value < config.axis_deadzone[axis])
        {
            // deadzone
            axis_value = 0;
            HACKSDL_debug("Deadzone new value=%d", axis_value);
        }
        else if(config.axis_mode[axis] = 1)
        {
            // digital mode
            if(axis_value < 0)
            {
                axis_value = SDL_AXIS_MIN;
            }
            else if(axis_value > 0)
            {
                axis_value = SDL_AXIS_MAX;
            }
            HACKSDL_debug("Digital mode new value=%d", axis_value);
        }

    }

    return axis_value;

}