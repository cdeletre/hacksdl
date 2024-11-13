#include <errno.h>
#include <libconfig.h>
#include <SDL2/SDL.h>
#include "configuration.h"
#include "debug.h"
#include <limits.h>

hacksdl_config_t config;
config_t cfg;
int file_config_present = 0;


const char* SDL_CONTROLLER_AXIS_FULLNAME[] = {
    "SDL_CONTROLLER_AXIS_LEFTX",            // 0
    "SDL_CONTROLLER_AXIS_LEFTY",
    "SDL_CONTROLLER_AXIS_RIGHTX",
    "SDL_CONTROLLER_AXIS_RIGHTY",
    "SDL_CONTROLLER_AXIS_TRIGGERLEFT",
    "SDL_CONTROLLER_AXIS_TRIGGERRIGHT",     // 5
};

const char* SDL_CONTROLLER_AXIS_SHORTNAME[] = {
    "LEFTX",            // 0
    "LEFTY",
    "RIGHTX",
    "RIGHTY",
    "TRIGGERLEFT",
    "TRIGGERRIGHT",     // 5
};

const char* SDL_CONTROLLER_BUTTON_FULLNAME[] = {
    "SDL_CONTROLLER_BUTTON_A",                  // 0
    "SDL_CONTROLLER_BUTTON_B",
    "SDL_CONTROLLER_BUTTON_X",
    "SDL_CONTROLLER_BUTTON_Y",
    "SDL_CONTROLLER_BUTTON_BACK",
    "SDL_CONTROLLER_BUTTON_GUIDE",
    "SDL_CONTROLLER_BUTTON_START",
    "SDL_CONTROLLER_BUTTON_LEFTSTICK",
    "SDL_CONTROLLER_BUTTON_RIGHTSTICK",
    "SDL_CONTROLLER_BUTTON_LEFTSHOULDER",
    "SDL_CONTROLLER_BUTTON_RIGHTSHOULDER",
    "SDL_CONTROLLER_BUTTON_DPAD_UP",
    "SDL_CONTROLLER_BUTTON_DPAD_DOWN",
    "SDL_CONTROLLER_BUTTON_DPAD_LEFT",
    "SDL_CONTROLLER_BUTTON_DPAD_RIGHT",
    "SDL_CONTROLLER_BUTTON_MISC1",    /* Xbox Series X share button", PS5 microphone button", Nintendo Switch Pro capture button", Amazon Luna microphone button */
    "SDL_CONTROLLER_BUTTON_PADDLE1",  /* Xbox Elite paddle P1 (upper left", facing the back) */
    "SDL_CONTROLLER_BUTTON_PADDLE2",  /* Xbox Elite paddle P3 (upper right", facing the back) */
    "SDL_CONTROLLER_BUTTON_PADDLE3",  /* Xbox Elite paddle P2 (lower left", facing the back) */
    "SDL_CONTROLLER_BUTTON_PADDLE4",  /* Xbox Elite paddle P4 (lower right", facing the back) */
    "SDL_CONTROLLER_BUTTON_TOUCHPAD", /* PS4/PS5 touchpad button */
};

const char* SDL_CONTROLLER_BUTTON_SHORTNAME[] = {
    "A",                  // 1
    "B",
    "X",
    "Y",
    "BACK",
    "GUIDE",
    "START",
    "LEFTSTICK",
    "RIGHTSTICK",
    "LEFTSHOULDER",
    "RIGHTSHOULDER",
    "DPAD_UP",
    "DPAD_DOWN",
    "DPAD_LEFT",
    "DPAD_RIGHT",
    "MISC1",    /* Xbox Series X share button", PS5 microphone button", Nintendo Switch Pro capture button", Amazon Luna microphone button */
    "PADDLE1",  /* Xbox Elite paddle P1 (upper left", facing the back) */
    "PADDLE2",  /* Xbox Elite paddle P3 (upper right", facing the back) */
    "PADDLE3",  /* Xbox Elite paddle P2 (lower left", facing the back) */
    "PADDLE4",  /* Xbox Elite paddle P4 (lower right", facing the back) */
    "TOUCHPAD", /* PS4/PS5 touchpad button */
};

/*
    default_config: init config with default values
*/
void default_config()
{
    config.verbose = 1;
    strcpy(config.libsdl_name,"RTLD_NEXT");
    config.no_gamecontroller = 0;
    config.modifier_button = SDL_CONTROLLER_BUTTON_INVALID;

    for(int axis=0;axis < SDL_CONTROLLER_AXIS_MAX; axis++)
    {
        config.modifier_shift[axis] = 0;
        config.axis_digital[axis] = 0;
        config.axis_deadzone[axis] = HACKSDL_AXIS_DEFAULT_DEADZONE;

    }

    for(int index=0;index < HACKSDL_DEVICE_INDEX_MAX; index++)
    {
        config.disable_device[index] = 0;
        config.index_mapping[index] = index;
    }
}

/*
    print_config: print the loaded configuration
*/
void print_config()
{
    HACKSDL_info("---- Configuration BEGIN ----");
    HACKSDL_info("libsdl_name=%s",config.libsdl_name);
    HACKSDL_info("no_gamecontroller=%d",config.no_gamecontroller);
    HACKSDL_info("modifier_button=%s",sdl_controller_button_name(config.modifier_button));
    for(int axis=0;axis < SDL_CONTROLLER_AXIS_MAX; axis++)
    {
        if(config.modifier_shift[axis] > 0)
        {
            HACKSDL_info("axis=%s modifier_shift=%d", get_axis_shortname(axis), config.modifier_shift[axis]);
        }
        HACKSDL_info("axis=%s digital=%d deadzone=%d", get_axis_shortname(axis), config.axis_digital[axis], config.axis_deadzone[axis]);
    }

    for(int index=0;index < HACKSDL_DEVICE_INDEX_MAX; index++)
    {
        if(config.index_mapping[index] != index)
        {
            HACKSDL_info("index_mapping[%d]=%d",index, config.index_mapping[index]);
        }
        HACKSDL_info("disable_device[%d]=%d",index, config.disable_device[index]);
    }
    HACKSDL_info("---- Configuration END ----");
}

/*
    load_config: config loading setup
*/
void load_config()
{
    
    default_config();
    read_config_env_int(HACKSDL_HINT_VERBOSE, &config.verbose);
    HACKSDL_info("Loading config");
    //SDL_GetHint(HACKSDL_HINT_VERBOSE);

    HACKSDL_debug("Tring to load from file");

    if(open_config_file())
    {
        HACKSDL_debug("Loading from file");
    }
    else
    {
        HACKSDL_debug("Loading from env");
    }
    load_config_data();

    print_config();
}

/*
    open_config_file: open config file
*/
int open_config_file()
{
    const char *config_file_path = SDL_GetHint(HACKSDL_HINT_CONFIG_FILE);

    if (config_file_path == NULL)
    {
        return 0;
    }

    HACKSDL_info("config_file_path = %s",config_file_path);

    config_init(&cfg);
    if(! config_read_file(&cfg, config_file_path))
    {
        HACKSDL_error("%s:%d - %s", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return 0;
    }
    file_config_present = 1;

    HACKSDL_info("Config file loaded");
    return 1;
}

/*
    load_config_data: load config data from file or env
*/
int load_config_data()
{

    const char *value;

    read_config_int(HACKSDL_HINT_VERBOSE, &config.verbose);

    if(read_config_string(HACKSDL_HINT_LIBSDL_NAME, &value))
    {
        strncpy(config.libsdl_name, value,HACKSDL_LIBSDL_NAME_LMAX);
        HACKSDL_info("HACKSDL_LIBSDL_NAME = %s", config.libsdl_name);
    }
    else    
    {
        HACKSDL_info("No HACKSDL_LIBSDL_NAME set, will use RTLD_NEXT handler");
    }

    // remap hack (HACKSDL_MAP_INDEX_)
    read_config_int_map(HACKSDL_HINT_MAP_INDEX_, &config.index_mapping[0], HACKSDL_DEVICE_INDEX_MAX);

    // disable device
    read_config_int_map(HACKSDL_HINT_DISABLE_DEVICE_, &config.disable_device[0], HACKSDL_DEVICE_INDEX_MAX);

    // no controller hack (HACKSDL_NO_GAMECONTROLLER)
    read_config_int(HACKSDL_HINT_NO_GAMECONTROLLER, &config.no_gamecontroller);

    // modifier hack (HACKSDL_MODIFIER_SHIFT_, HACKSDL_MODIFIER_BUTTON)
    read_config_int_map_b(HACKSDL_HINT_MODIFIER_SHIFT_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.modifier_shift[0], SDL_CONTROLLER_AXIS_MAX);

    if(read_config_string(HACKSDL_HINT_MODIFIER_BUTTON, &value))
    {
        config.modifier_button = get_gc_button(value);
    }
    else
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON not set");
    }

    // trigger hack (HACKSDL_AXIS_MODE/DEADZONE)
    read_config_int_map_b(HACKSDL_HINT_AXIS_DIGITAL_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_digital[0], SDL_CONTROLLER_AXIS_MAX);
    read_config_int_map_b(HACKSDL_HINT_AXIS_DEADZONE_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_deadzone[0], SDL_CONTROLLER_AXIS_MAX);
}

/*
    config value access functions (generic)
*/

/*
    read_config_string: read a string for the key entry
*/
int read_config_string(char* key, const char **value)
{
    if (file_config_present)
    {
        return read_config_file_string(key, value);
    }
    else
    {
        return read_config_env_string(key, value);
    }
}

/*
    read_config_int: read an int for the key entry
*/
int read_config_int(char* key, int *value)
{
    if (file_config_present)
    {
        return read_config_file_int(key, value);
    }
    else
    {
        return read_config_env_int(key, value);
    }
}

/*
    read_config_int_map: read a map of int for the key_prefix entry indexed with numbers
*/
int read_config_int_map(char* key_prefix, int *value_map, int length)
{

    char buffer[64];
    int int_value;
    for(int index=0; index<length; index++ )
    {
        sprintf(buffer,key_prefix,index);

        if(read_config_int(buffer, &int_value))
        {
            value_map[index] = int_value;
        }
    }
    return 1;

}

/*
    read_config_int_map_b: read a map of int for the key_prefix entry indexed with key from the keys list
*/
int read_config_int_map_b(const char* key_prefix, const char** keys, int *value_map, int length)
{

    char buffer[64];
    int int_value;
    for(int index=0; index<length; index++ )
    {
        sprintf(buffer,key_prefix,keys[index]);
        if(read_config_int(buffer, &int_value))
        {
            value_map[index] = int_value;
        }
    }

    return 1;

}

/*
    config value access functions (file)
*/
int read_config_file_string(const char* key, const char **value)
{
    if(config_lookup_string(&cfg, key, value))
    {
        HACKSDL_debug("%s = %s", key, *value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", key);
        return 0;
    }
}

int read_config_file_int(const char* key, int *value)
{
    const char* str_value;
    char *endptr = NULL; 
    long long_value = 0;

    if(config_lookup_string(&cfg, key, &str_value))
    {
        errno = 0;
        long_value = strtol(str_value,&endptr,10);

        if (str_value == endptr)
        {
            HACKSDL_error("%s : %s invalid  (no digits found)", key, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MIN)
        {
            HACKSDL_error("%s : %s invalid  (underflow occurred)", key, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MAX)
        {
            HACKSDL_error("%s : %s invalid  (overflow occurred)", key, str_value);
            }
        else if (errno == EINVAL)  /* not in all c99 implementations - gcc OK */
        {
            HACKSDL_error("%s : %s invalid  (base contains unsupported value)", key, str_value);
        }
        else if (errno != 0 && long_value == 0)
        {
            HACKSDL_error("%s : %s invalid  (unspecified error occurred)", key, str_value);
        }
        else if (errno == 0 && str_value)
        {
            *value = (int)long_value;
            HACKSDL_debug("%s = %d",key, (int)long_value);
            return 1;
        }
    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }

    return 0;
}

/*
    config value access functions (env)
*/
int read_config_env_string(const char* key, const char **value){

    *value = SDL_GetHint(key);
    if(*value != NULL)
    {
        HACKSDL_debug("%s = %s", key, *value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }
    return 0;

}

int read_config_env_int(const char *key, int *value)
{
    const char* str_value = SDL_GetHint(key);

    char *endptr = NULL; 
    long long_value = 0;

    if(str_value != NULL)
    {

        errno = 0;
        long_value = strtol(str_value,&endptr,10);

        if (str_value == endptr)
        {
            HACKSDL_error("%s : %s invalid  (no digits found)", key, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MIN)
        {
            HACKSDL_error("%s : %s invalid  (underflow occurred)", key, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MAX)
        {
            HACKSDL_error("%s : %s invalid  (overflow occurred)", key, str_value);
        }
        else if (errno == EINVAL)  /* not in all c99 implementations - gcc OK */
        {
            HACKSDL_error("%s : %s invalid  (base contains unsupported value)", key, str_value);
        }
        else if (errno != 0 && long_value == 0)
        {
            HACKSDL_error("%s : %s invalid  (unspecified error occurred)", key, str_value);
        }
        else if (errno == 0 && str_value)
        {
            *value = (int)long_value;
            HACKSDL_debug("%s = %d",key, (int)long_value);
            return 1;
        }

    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }

    return 0;
}

/*
    get_gc_button: get the SDL_GameControllerButton from the button name (A,B...)
*/
SDL_GameControllerButton get_gc_button(const char* modifier_button_str)
{
    for(int i=SDL_CONTROLLER_BUTTON_A; i<SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        if( strcmp(modifier_button_str, SDL_CONTROLLER_BUTTON_SHORTNAME[i]) == 0)
        {
            return i;
        }
    }
    return SDL_CONTROLLER_BUTTON_INVALID;
}

/*
    sdl_controller_button_to_string: get the button name (A,B...) from the SDL_GameControllerButton value
*/
const char* sdl_controller_button_name(SDL_GameControllerButton button)
{
    if(button < SDL_CONTROLLER_BUTTON_A || button > SDL_CONTROLLER_BUTTON_MAX)
        return "INVALID";
    return SDL_CONTROLLER_BUTTON_SHORTNAME[button];
}

/*
    get_gc_axis: get the SDL_GameControllerAxis from the axis name (LEFTX,LEFTY...)
*/
SDL_GameControllerAxis get_gc_axis(const char* axis_str)
{
    for(int i=SDL_CONTROLLER_AXIS_LEFTX; i<SDL_CONTROLLER_AXIS_MAX; i++)
    {
        if( strcmp(axis_str, SDL_CONTROLLER_AXIS_SHORTNAME[i]) == 0)
            HACKSDL_debug("AXIS = %s", SDL_CONTROLLER_AXIS_SHORTNAME[i]);
            return i;
    }
    return SDL_CONTROLLER_AXIS_INVALID;
}

/*
    get_axis_name: Get the SDL axis name (char*)
*/
const char* get_axis_name(SDL_GameControllerAxis axis)
{
    if(axis < SDL_CONTROLLER_AXIS_LEFTX || axis > SDL_CONTROLLER_AXIS_MAX)
        return "SDL_CONTROLLER_AXIS_INVALID";
    return SDL_CONTROLLER_AXIS_FULLNAME[axis];
}

/*
    get_axis_shortname: Get the SDL axis name (char*)
*/
const char* get_axis_shortname(SDL_GameControllerAxis axis)
{

    if(axis < SDL_CONTROLLER_AXIS_LEFTX || axis > SDL_CONTROLLER_AXIS_MAX)
        return "INVALID";
    return SDL_CONTROLLER_AXIS_SHORTNAME[axis];
}