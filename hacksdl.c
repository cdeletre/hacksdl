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
Uint8 (*original_SDL_GameControllerGetButton)(SDL_GameController *gamecontroller, SDL_GameControllerButton button);
SDL_bool (*original_SDL_GameControllerHasAxis)(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis);

// Display functions
int (*original_SDL_GetCurrentDisplayMode)(int displayIndex, SDL_DisplayMode * mode);
int (*original_SDL_SetWindowDisplayMode)(SDL_Window * window, const SDL_DisplayMode * mode);
void (*original_SDL_GetWindowSize)(SDL_Window * window, int *w, int *h);
void (*original_SDL_SetWindowSize)(SDL_Window * window, int w, int h);
void (*original_SDL_SetWindowPosition)(SDL_Window * window, int x, int y);
SDL_DisplayMode* (*original_SDL_GetClosestDisplayMode)(int displayIndex, const SDL_DisplayMode * mode, SDL_DisplayMode * closest);

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
    original_SDL_GameControllerGetButton = dlsym(sdl_handler, "SDL_GameControllerGetButton");
    original_SDL_GameControllerHasAxis = dlsym(sdl_handler, "SDL_GameControllerHasAxis");

    original_SDL_GetCurrentDisplayMode = dlsym(sdl_handler, "SDL_GetCurrentDisplayMode");
    original_SDL_SetWindowDisplayMode = dlsym(sdl_handler, "SDL_SetWindowDisplayMode");
    original_SDL_GetWindowSize = dlsym(sdl_handler, "SDL_GetWindowSize");
    original_SDL_SetWindowSize = dlsym(sdl_handler, "SDL_SetWindowSize");
    original_SDL_SetWindowPosition = dlsym(sdl_handler, "SDL_SetWindowPosition");
    original_SDL_GetClosestDisplayMode = dlsym(sdl_handler, "SDL_GetClosestDisplayMode");
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
    HACKSDL_map_index: return the hacked index for a device
*/
int HACKSDL_map_index(int index)
{
    return config.controller_index_mapping[index];
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
    if(config.no_gamecontroller)
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

    if(config.no_gamecontroller == 2 || config.device_disable[device_index] == 2)
    {
        HACKSDL_debug("Hook + hack: return NULL for device_index=%d", device_index);
        return NULL;
    }

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
    if(config.no_gamecontroller == 2 || config.device_disable[joystick_index] == 2)
    {
        HACKSDL_debug("Hook + hack: return NULL for joystick_index=%d", joystick_index);
        return NULL;
    }

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

    if(config.no_gamecontroller || config.device_disable[joystick_index]){
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
    int axis_minus_virtual_button_value = 0;
    int axis_plus_virtual_button_value = 0;

    axis_value = original_SDL_GameControllerGetAxis(gamecontroller, axis);

    HACKSDL_debug("Hook gamecontroller=%s axis=%s value=%d", SDL_GameControllerName(gamecontroller), SDL_GameControllerGetStringForAxis(axis), axis_value);

    if ( (config.axis_virtual_minus_map[axis] != SDL_CONTROLLER_BUTTON_INVALID) || (config.axis_virtual_plus_map[axis] != SDL_CONTROLLER_BUTTON_INVALID) )
    {
        axis_minus_virtual_button_value = original_SDL_GameControllerGetButton(gamecontroller, config.axis_virtual_minus_map[axis]);
        axis_plus_virtual_button_value = original_SDL_GameControllerGetButton(gamecontroller, config.axis_virtual_plus_map[axis]);
        
        if(axis_minus_virtual_button_value && (! axis_plus_virtual_button_value))
        {
            axis_value = SDL_AXIS_MIN;
        }else if((! axis_minus_virtual_button_value) && axis_plus_virtual_button_value)
        {
            axis_value = SDL_AXIS_MAX;
        }else{
            axis_value = 0;
        }
        HACKSDL_debug("Virtual axis enabled, new value=%d", axis_value);
    }

    if(config.axis_modifier_shift[axis] != 0)
    {
        button_pressed = original_SDL_GameControllerGetButton(gamecontroller, config.modifier_button);

        if (button_pressed == 1)
        {
            axis_value = axis_value >> config.axis_modifier_shift[axis];
            HACKSDL_debug("Modifier enabled, new value=%d", axis_value);
        }
    }
    
    if(config.axis_deadzone[axis] > 0)
    {
        if(axis_value < config.axis_deadzone[axis])
        {
            // deadzone
            axis_value = 0;
            HACKSDL_debug("Deadzone enabled, new value=%d", axis_value);
        }
        else if(config.axis_digital[axis] = 1)
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
            HACKSDL_debug("Digital mode, new value=%d", axis_value);
        }

    }

    return axis_value;

}

Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller, SDL_GameControllerButton button)
{
    Uint8 button_pressed = 0;

    button_pressed = original_SDL_GameControllerGetButton(gamecontroller, button);

    HACKSDL_debug("Hook gamecontroller=%s button=%s value=%d", SDL_GameControllerName(gamecontroller), SDL_GameControllerGetStringForButton(button), button_pressed);

    if(config.button_disable[button])
    {
        HACKSDL_debug("Button disabled, new value=0");
        return 0;
    }

    return button_pressed;

}

SDL_bool SDL_GameControllerHasAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis)
{
    SDL_bool has_axis = SDL_FALSE;

    has_axis = SDL_GameControllerHasAxis(gamecontroller, axis);

    HACKSDL_debug("Hook gamecontroller=%s button=%s value=%d", SDL_GameControllerName(gamecontroller), SDL_GameControllerGetStringForAxis(axis), has_axis);

    // Return true if this axis is virtual
    if ((config.axis_virtual_minus_map[axis] != SDL_CONTROLLER_BUTTON_INVALID) || (config.axis_virtual_plus_map[axis] != SDL_CONTROLLER_BUTTON_INVALID))
    {
        HACKSDL_debug("Virtual axis enabled, new value=1");
    }

    return has_axis;

}

int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
    int result = 0;
    result = original_SDL_GetCurrentDisplayMode(displayIndex, mode);
    HACKSDL_debug("result=%d", result);
    if(result == 0)
    {
        HACKSDL_debug("mode.w=%d mode.h=%d", mode->w,mode->h);
    }

    if (config.get_display_mode_w != 0)
    {
        mode->w = config.get_display_mode_w;
        HACKSDL_debug("new value mode.w=%d", mode->w);
    }

    if (config.get_display_mode_h != 0)
    {
        mode->h = config.get_display_mode_h;
        HACKSDL_debug("new value mode.h=%d", mode->h);
    }

    return result;

}

int SDL_SetWindowDisplayMode(SDL_Window * window, const SDL_DisplayMode * mode)
{
    HACKSDL_debug("mode.w=%d mode.h=%d", mode->w, mode->h);

    SDL_DisplayMode * new_mode;
    memcpy((void*)mode, (void*)new_mode, sizeof(SDL_DisplayMode)); // UNTESTED !


    if (config.set_display_mode_w != 0)
    {
        new_mode->w = config.set_display_mode_w;
        HACKSDL_debug("new value mode.w=%d", new_mode->w);
    }

    if (config.set_display_mode_h != 0)
    {
        new_mode->h = config.set_display_mode_h;
        HACKSDL_debug("new value mode.h=%d", new_mode->h);
    }
    
    original_SDL_SetWindowDisplayMode(window, new_mode);
}

void SDL_GetWindowSize(SDL_Window * window, int *w, int *h)
{
    original_SDL_GetWindowSize(window, w, h);
    HACKSDL_debug("w=%d h=%d", *w, *h);
    if (config.get_window_size_w != 0)
    {
        *w = config.get_window_size_w;
        HACKSDL_debug("new value w=%d" , *w);
    }
    if (config.get_window_size_h != 0)
    {
        *h = config.get_window_size_h;
        HACKSDL_debug("new value w=%d" , *h);
    }
}

void SDL_SetWindowSize(SDL_Window * window, int w, int h)
{
    HACKSDL_debug("w=%d h=%d", w, h);

    if (config.set_window_size_w != 0)
    {
        w = config.set_window_size_w;
        HACKSDL_debug("new value w=%d", w);
    }

    if (config.set_window_size_h != 0)
    {
        h = config.set_window_size_h;
        HACKSDL_debug("new value h=%d", h);
    }

    original_SDL_SetWindowSize(window, w, h);
}

SDL_DisplayMode * SDL_GetClosestDisplayMode(int displayIndex, const SDL_DisplayMode * mode, SDL_DisplayMode * closest)
{
    SDL_DisplayMode * result;
    result = original_SDL_GetClosestDisplayMode(displayIndex, mode, closest);

        HACKSDL_debug("mode.w=%d mode.h=%d closest.w=%d closest.h=%d", mode->w, mode->h, closest->w, closest->h);
        if (result != NULL)
        {
            HACKSDL_debug("result.w=%d result.h=%d closest.w=%d closest.h=%d", result->w, result->h);
        }

    return result;
}
