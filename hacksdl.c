#include <errno.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <libconfig.h>

//#include <SDL2/SDL_joystick.h>
//#include <SDL2/SDL_gamecontroller.h>

#define HACKSDL_MAP_INDEX_MAX                   16
#define HACKSDL_AXIS_MAX                         8

#define HACKSDL_HINT_LIBSDL_NAME                "HACKSDL_LIBSDL_NAME"
#define HACKSDL_HINT_CONFIG_FILE                "HACKSDL_CONFIG_FILE"
#define HACKSDL_HINT_MAP_INDEX_                 "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_HINT_DEBUG                      "HACKSDL_DEBUG"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_MODIFIER_BUTTON            "HACKSDL_MODIFIER_BUTTON"
#define HACKSDL_HINT_MODIFIER_SHIFT_            "HACKSDL_MODIFIER_SHIFT_%d"
#define HACKSDL_HINT_AXIS_THRESHOLD_            "HACKSDL_AXIS_THRESHOLD_%d"
#define HACKSDL_HINT_AXIS_MIN_                  "HACKSDL_AXIS_MIN_%d"
#define HACKSDL_HINT_AXIS_MAX_                  "HACKSDL_AXIS_MAX_%d"

/*
    Original SDL2 handler
*/
void *sdl_handler = RTLD_NEXT;

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


// Configuration data
int initialized = 0;
char* libsdl_name = NULL;
int debug = 0;
int no_gamecontroller = 0;
SDL_GameControllerButton modifier_button = SDL_CONTROLLER_BUTTON_INVALID;
int modifier_shift[HACKSDL_AXIS_MAX];
int index_mapping[HACKSDL_MAP_INDEX_MAX];
int axis_digital[HACKSDL_AXIS_MAX];
int axis_threshold[HACKSDL_AXIS_MAX];
int axis_min[HACKSDL_AXIS_MAX];
int axis_max[HACKSDL_AXIS_MAX];


/*
    HACKSDL_debug: print to stdout when debug is enabled
*/
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

/*
    get_gc_button: get the SDL_GameControllerButton from the button name (A,B...)
*/
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

/*
    Dynamic Library open for original SDL functions
*/
int setup_original_SDL_functions(){

    if(libsdl_name != NULL)
    {
        sdl_handler = dlopen(libsdl_name, RTLD_LAZY);
        if (!sdl_handler) {
            HACKSDL_debug("Error loading SDL2 library (%s): %s", libsdl_name, dlerror());
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
    read_config_file: read config from file
*/
int read_config_file()
{
    const char *config_file_path = SDL_GetHint(HACKSDL_HINT_CONFIG_FILE);

    if (config_file_path == NULL)
    {
        return 0;
    }

    HACKSDL_debug("config_file_path = %s",config_file_path);

    config_t cfg;
    const char *value;
    config_init(&cfg);
    if(! config_read_file(&cfg, config_file_path))
    {
        HACKSDL_debug("%s:%d - %s", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return 0;
    }

    HACKSDL_debug("Config file loaded");

    if(config_lookup_string(&cfg, HACKSDL_HINT_LIBSDL_NAME, &value))
    {
        libsdl_name = strdup(value);
        HACKSDL_debug("HACKSDL_LIBSDL_NAME = %s", libsdl_name);
    }
    else
    {
        HACKSDL_debug("No HACKSDL_LIBSDL_NAME set, will use RTLD_NEXT handler");
    }


    char buffer[32];
    int original_index;
    for(int index=0; index<HACKSDL_MAP_INDEX_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_MAP_INDEX_,index);

        if(config_lookup_string(&cfg, buffer, &value))
        {
            errno = 0;
            original_index = (int)strtol(value,NULL,10);

            if (errno != 0){
                HACKSDL_debug("Incorrect mapping %s: %s",buffer, value);
                original_index = index;
            }else{
                HACKSDL_debug("Map index %d to %d", index, original_index);
            }
        }
        else
        {
            HACKSDL_debug("No mapping set for index %d", index);
            original_index = index;
        }
        
        index_mapping[index] = original_index;
    }

    // no gamecontroller hack (HACKSDL_NO_GAMECONTROLLER)
    if(config_lookup_string(&cfg, HACKSDL_HINT_NO_GAMECONTROLLER, &value))
    {
        if(strcmp(value,"1") == 0)
        {
            HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = True");
            no_gamecontroller = 1;
        }
        else
        {
            HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = False");
        }
    }
    else
    {
        HACKSDL_debug("No HACKSDL_NO_GAMECONTROLLER set");
    }

   // modifier hack (HACKSDL_MODIFIER_BUTTON and HACKSDL_MODIFIER_SHIFT)
    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        modifier_shift[index] = 0;
        sprintf(buffer,HACKSDL_HINT_MODIFIER_SHIFT_,index);

        if(config_lookup_string(&cfg, buffer, &value))
        {
            errno = 0;
            modifier_shift[index] = (int)strtol(value,NULL,10);

            if (errno != 0){
                HACKSDL_debug("incorrect shift value HACKSDL_MODIFIER_SHIFT_%d: %s",index, value);
                
            }else{
                HACKSDL_debug("HACKSDL_MODIFIER_SHIFT_%d = %d", index, modifier_shift[index]);
            }
        }
        else
        {
            HACKSDL_debug("No HACKSDL_MODIFIER_SHIFT_%d set", index);
        }
    }

    if(config_lookup_string(&cfg, HACKSDL_HINT_MODIFIER_BUTTON, &value))
    {
        modifier_button = get_gc_button(value);
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = %s", value);
    }

     // modifier hack (HACKSDL_AXIS_THRESHOLD, HACKSDL_AXIS_MIN, HACKSDL_AXIS_MAX)
    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_AXIS_THRESHOLD_,index);
        axis_digital[index] = 0;
        axis_threshold[index] = 0;

        if(config_lookup_string(&cfg, buffer, &value))
        {
            errno = 0;
            axis_threshold[index] = (int)strtol(value,NULL,10);
            if (axis_threshold[index] < -32768 || axis_threshold[index] > 32767|| errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_AXIS_THRESHOLD_%d: %s", index, value);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_THRESHOLD_%d = %d", index, axis_threshold[index]);
                axis_digital[index] = 1;
            }
            
        }
        else
        {
            HACKSDL_debug("No HACKSDL_AXIS_THRESHOLD_%d", index);
        }
    }

    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        axis_min[index] = 0;
        if(axis_digital[index] == 0)
        {
            continue;
        }

        sprintf(buffer,HACKSDL_HINT_AXIS_MIN_,index);

        if(config_lookup_string(&cfg, buffer, &value))
        {
            errno = 0;
            axis_min[index] = (int)strtol(value,NULL,10);
            if (axis_min[index] < -32768 || axis_min[index] > 32767|| errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_AXIS_MIN_%d: %s", index, value);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_MIN_%d = %d", index, axis_min[index]);
            }
            
        }
        else
        {
            HACKSDL_debug("Default HACKSDL_AXIS_MIN_%d = %d", index, axis_min[index]);
        }
    }

    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        axis_max[index] = 32767;
        if(axis_digital[index] == 0)
        {
            continue;
        }

        sprintf(buffer,HACKSDL_HINT_AXIS_MAX_,index);

        if(config_lookup_string(&cfg, buffer, &value))
        {
            errno = 0;
            axis_max[index] = (int)strtol(value,NULL,10);
            if (axis_max[index] < -32768 || axis_max[index] > 32767|| errno != 0){
                HACKSDL_debug("incorrect HACKSDL_AXIS_MAX_%d: %s", index, value);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_MAX_%d = %d", index, axis_max[index]);
            }
            
        }
        else
        {
            HACKSDL_debug("Default HACKSDL_AXIS_MAX_%d = %d", index, axis_max[index]);
        }
    }

}

/*
    read_config_env: read config from env
*/
int read_config_env()
{

    const char* hint = SDL_GetHint(HACKSDL_HINT_LIBSDL_NAME);
    if(hint != NULL)
    {
        libsdl_name = strdup(hint);
        HACKSDL_debug("HACKSDL_LIBSDL_NAME = %s", libsdl_name);
    }
    else
    {
        HACKSDL_debug("No HACKSDL_LIBSDL_NAME set, will use RTLD_NEXT handler");
    }

    // joystick index remapping (HACKSDL_MAP_INDEX_<n>)

    char buffer[32];
    int original_index;

    for(int index=0; index<HACKSDL_MAP_INDEX_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_MAP_INDEX_,index);

        const char *hint = SDL_GetHint(buffer);
        if (hint != NULL){
            errno = 0;
            original_index = (int)strtol(hint,NULL,10);

            if (errno != 0){
                HACKSDL_debug("Incorrect mapping %s: %s",buffer, hint);
                original_index = index;
            }else{
                HACKSDL_debug("Map index %d to %d", index, original_index);
            }
        }else{
            HACKSDL_debug("No mapping set for index %d", index);
            original_index = index;
        }
        index_mapping[index] = original_index;
    }

    // no gamecontroller hack (HACKSDL_NO_GAMECONTROLLER)
    no_gamecontroller = SDL_GetHintBoolean(HACKSDL_HINT_NO_GAMECONTROLLER,SDL_FALSE);
    if(no_gamecontroller)
    {
        HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = True");
    }
    else
    {
        HACKSDL_debug("HACKSDL_NO_GAMECONTROLLER = False");
    }

    // modifier hack (HACKSDL_MODIFIER_BUTTON and HACKSDL_MODIFIER_SHIFT)
    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_MODIFIER_SHIFT_,index);
        const char *modifier_shift_hint = SDL_GetHint(buffer);

        if(modifier_shift_hint != NULL)
        {
            errno = 0;
            modifier_shift[index] = (int)strtol(modifier_shift_hint,NULL,10);
            if (modifier_shift < 0 || errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_MODIFIER_SHIFT_%d: %s", index, modifier_shift_hint);
                modifier_shift[index] = 0;
            }
            
            if (modifier_shift[index] > 0)
            {
                HACKSDL_debug("HACKSDL_MODIFIER_SHIFT_%d = %d", index, modifier_shift[index]);
            }
            else
            {
                HACKSDL_debug("HACKSDL_MODIFIER_SHIFT disabled");
            }
        }
    }

    modifier_button = get_gc_button(SDL_GetHint(HACKSDL_HINT_MODIFIER_BUTTON));
    HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = %s", SDL_GetHint(HACKSDL_HINT_MODIFIER_BUTTON));

    // modifier hack (HACKSDL_AXIS_THRESHOLD, HACKSDL_AXIS_MIN, HACKSDL_AXIS_MAX)
    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        sprintf(buffer,HACKSDL_HINT_AXIS_THRESHOLD_,index);
        const char *axis_threshold_hint = SDL_GetHint(buffer);
        axis_digital[index] = 0;
        axis_threshold[index] = 0;

        if(axis_threshold_hint != NULL)
        {
            errno = 0;
            axis_threshold[index] = (int)strtol(axis_threshold_hint,NULL,10);
            if (axis_threshold[index] < -32768 || axis_threshold[index] > 32767|| errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_AXIS_THRESHOLD_%d: %s", index, axis_threshold_hint);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_THRESHOLD_%d = %d", index, axis_threshold[index]);
                axis_digital[index] = 1;
            }
            
        }
        else
        {
            HACKSDL_debug("No HACKSDL_AXIS_THRESHOLD_%d", index);
        }
    }

    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        axis_min[index] = 0;
        if(axis_digital[index] == 0)
        {
            continue;
        }

        sprintf(buffer,HACKSDL_HINT_AXIS_MIN_,index);
        const char *axis_min_hint = SDL_GetHint(buffer);

        if(axis_min_hint != NULL)
        {
            errno = 0;
            axis_min[index] = (int)strtol(axis_min_hint,NULL,10);
            if (axis_min[index] < -32768 || axis_min[index] > 32767|| errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_AXIS_MIN_%d: %s", index, axis_min_hint);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_MIN_%d = %d", index, axis_min[index]);
            }
            
        }
        else
        {
            HACKSDL_debug("Default HACKSDL_AXIS_MIN_%d = ", index, axis_min[index]);
        }
    }

    for(int index=0; index<HACKSDL_AXIS_MAX; index++ )
    {
        axis_max[index] = 32767;
        if(axis_digital[index] == 0)
        {
            continue;
        }

        sprintf(buffer,HACKSDL_HINT_AXIS_MAX_,index);
        const char *axis_max_hint = SDL_GetHint(buffer);

        if(axis_max_hint != NULL)
        {
            errno = 0;
            axis_max[index] = (int)strtol(axis_max_hint,NULL,10);
            if (axis_max[index] < -32768 || axis_max[index] > 32767|| errno != 0){
                HACKSDL_debug("Incorrect HACKSDL_AXIS_MAX_%d: %s", index, axis_max_hint);
            }
            else
            {
                HACKSDL_debug("HACKSDL_AXIS_MAX_%d = %d", index, axis_max[index]);
            }
            
        }
        else
        {
            HACKSDL_debug("Default HACKSDL_AXIS_MAX_%d = ", index, axis_max[index]);
        }
    }

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

    debug = SDL_GetHintBoolean(HACKSDL_HINT_DEBUG,SDL_FALSE);

    HACKSDL_debug("Initializing");

    if(read_config_file() == 0)
    {
        read_config_env();
    }

    if(setup_original_SDL_functions() == 0)
    {
        return 0;
    }

    HACKSDL_debug("Initialization done");

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
    return index_mapping[index];
}

/*
    Functions hooks
*/

int SDL_Init(Uint32 flags)
{
    initialize();

    HACKSDL_debug("hook on SDL_Init");

    return original_SDL_Init(flags);
}

int SDL_NumJoysticks(void)
{
    HACKSDL_debug("hook on SDL_NumJoysticks");
    if(no_gamecontroller == 1)
    {
        return 0;
    }
    else
    {
        return original_SDL_NumJoysticks();
    }
}

int SDL_JoystickGetDevicePlayerIndex(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDevicePlayerIndex(%d)", device_index);

    return original_SDL_JoystickGetDevicePlayerIndex(HACKSDL_map_index(device_index));
}

SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceGUID(%d)", device_index);

    return original_SDL_JoystickGetDeviceGUID(HACKSDL_map_index(device_index));

}

SDL_Joystick* SDL_JoystickOpen(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickOpen(%d)", device_index);
    
    return original_SDL_JoystickOpen(HACKSDL_map_index(device_index));
}

const char* SDL_JoystickNameForIndex(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickNameForIndex(%d)", device_index);
    
    return original_SDL_JoystickNameForIndex(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceVendor(%d)", device_index);
    
    return original_SDL_JoystickGetDeviceVendor(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceProduct(%d)", device_index);
    
    return original_SDL_JoystickGetDeviceProduct(HACKSDL_map_index(device_index));
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceProductVersion(%d)", device_index);
    
    return original_SDL_JoystickGetDeviceProductVersion(HACKSDL_map_index(device_index));
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceType(%d)", device_index);
    
    return original_SDL_JoystickGetDeviceType(HACKSDL_map_index(device_index));
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{

    HACKSDL_debug("hook on SDL_JoystickGetDeviceInstanceID(%d)", device_index);
    
    return original_SDL_JoystickGetDeviceInstanceID(HACKSDL_map_index(device_index));
}

SDL_GameController* SDL_GameControllerOpen(int joystick_index)
{    

    HACKSDL_debug("hook on SDL_GameControllerOpen(%d)", joystick_index);

    return original_SDL_GameControllerOpen(HACKSDL_map_index(joystick_index));;
}

char* SDL_GameControllerMappingForIndex(int mapping_index)
{

    HACKSDL_debug("hook on SDL_GameControllerMappingForIndex(%d)", mapping_index);
    
    return original_SDL_GameControllerMappingForIndex(HACKSDL_map_index(mapping_index));
}

SDL_bool SDL_IsGameController(int joystick_index)
{

    HACKSDL_debug("hook on SDL_IsGameController(%d)", joystick_index);
    
    if(SDL_GetHintBoolean(HACKSDL_HINT_NO_GAMECONTROLLER,SDL_FALSE)){
        return SDL_FALSE;
    }else{
        return original_SDL_IsGameController(HACKSDL_map_index(joystick_index));
    }
}

const char* SDL_GameControllerNameForIndex(int joystick_index)
{

    HACKSDL_debug("hook on SDL_GameControllerNameForIndex(%d)", joystick_index);
    
    return original_SDL_GameControllerNameForIndex(HACKSDL_map_index(joystick_index));
}

char* SDL_GameControllerMappingForDeviceIndex(int joystick_index)
{

    HACKSDL_debug("hook on SDL_GameControllerMappingForDeviceIndex(%d)", joystick_index);
    
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

    //HACKSDL_debug("hook on SDL_GameControllerGetAxis(%s,%s), value=%d (Sint16)", SDL_GameControllerName(gamecontroller), get_axis_name(axis), axis_value);

    if(modifier_shift[axis] != 0)
    {
        button_pressed = SDL_GameControllerGetButton(gamecontroller, modifier_button);

        if (button_pressed == 1)
        {
            axis_value = axis_value >> modifier_shift[axis];
            HACKSDL_debug("modifier enabled new value=%d (Sint16)", axis_value);
        }
    }
    
    if(axis_digital[axis] == 1)
    {
        if(axis_value > axis_threshold[axis]){
            axis_value = axis_max[axis];
        }
        else
        {
            axis_value = axis_min[axis];
        }
    }

    return axis_value;

}