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
    "A",                  // 0
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
        config.axis_modifier_shift[axis] = 0;
        config.axis_digital[axis] = SDL_FALSE;
        config.axis_deadzone[axis] = HACKSDL_AXIS_DEFAULT_DEADZONE;
        config.axis_minus_virtual_map[axis] = SDL_CONTROLLER_BUTTON_INVALID;
        config.axis_plus_virtual_map[axis] = SDL_CONTROLLER_BUTTON_INVALID;
        config.axis_virtual_share[axis] = SDL_FALSE;
    }

    for(int button=SDL_CONTROLLER_BUTTON_A; button<SDL_CONTROLLER_BUTTON_MAX; button++)
    {
        config.button_disable[button] = SDL_FALSE;
    }

    for(int index=0;index < HACKSDL_DEVICE_INDEX_MAX; index++)
    {
        config.device_disable[index] = 0;
        config.controller_index_mapping[index] = index;
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
    HACKSDL_info("axis_modifier_button=%s",SDL_GameControllerGetStringForButton(config.modifier_button));
    for(int axis=0;axis < SDL_CONTROLLER_AXIS_MAX; axis++)
    {
        HACKSDL_info("axis.%s", SDL_GameControllerGetStringForAxis(axis));

        HACKSDL_info("    axis_modifier_shift=%d", config.axis_modifier_shift[axis]);

        HACKSDL_info("    axis_digital=%d", config.axis_digital[axis]);
        HACKSDL_info("    deadzone=%d", config.axis_deadzone[axis]);
        if (config.axis_minus_virtual_map[axis] != SDL_CONTROLLER_BUTTON_INVALID)
        {
            HACKSDL_info("    (-) virtual_map=%s", SDL_GameControllerGetStringForButton(config.axis_minus_virtual_map[axis]));
        }
        else
        {
            HACKSDL_info("    (-) virtual_map=%s", "no");
        }

        if (config.axis_plus_virtual_map[axis] != SDL_CONTROLLER_BUTTON_INVALID)
        {
            HACKSDL_info("    (+) virtual_map=%s", SDL_GameControllerGetStringForButton(config.axis_plus_virtual_map[axis]));
        }
        else
        {
            HACKSDL_info("    (+) virtual_map=%s", "no");
        }

        HACKSDL_info("    virtual_share=%d", config.axis_virtual_share[axis]);


    }

    HACKSDL_info("button_disable");
    for(int button=SDL_CONTROLLER_BUTTON_A; button<SDL_CONTROLLER_BUTTON_MAX; button++)
    {
        HACKSDL_info("    button.%s=%d", SDL_GameControllerGetStringForButton(button), config.button_disable[button]);
    }

    HACKSDL_info("controller_index_mapping");
    for(int index=0;index < HACKSDL_DEVICE_INDEX_MAX; index++)
    {   
        HACKSDL_info("    device.%d=%d",index, config.controller_index_mapping[index]);
    }
    HACKSDL_info("device_disable");
    for(int index=0;index < HACKSDL_DEVICE_INDEX_MAX; index++)
    {     
        HACKSDL_info("    device.%d=%d",index, config.device_disable[index]);
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
    read_config_int_map_indexes(HACKSDL_HINT_DEVICE_MAP_INDEX_, &config.controller_index_mapping[0], HACKSDL_DEVICE_INDEX_MAX);

    // disable device
    read_config_int_map_indexes(HACKSDL_HINT_DEVICE_DISABLE_, &config.device_disable[0], HACKSDL_DEVICE_INDEX_MAX);

    // no controller hack (HACKSDL_NO_GAMECONTROLLER)
    read_config_int(HACKSDL_HINT_NO_GAMECONTROLLER, &config.no_gamecontroller);

    // modifier hack (HACKSDL_AXIS_MODIFIER_SHIFT_, HACKSDL_AXIS_MODIFIER_BUTTON)
    read_config_int_map_keys(HACKSDL_HINT_AXIS_MODIFIER_SHIFT_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_modifier_shift[0], SDL_CONTROLLER_AXIS_MAX);

    if(read_config_string(HACKSDL_HINT_AXIS_MODIFIER_BUTTON, &value))
    {
        config.modifier_button = SDL_GameControllerGetButtonFromString(value);
    }
    else
    {
        HACKSDL_debug("HACKSDL_AXIS_MODIFIER_BUTTON not set");
    }

    // trigger hack (HACKSDL_AXIS_MODE/DEADZONE)
    read_config_int_map_keys(HACKSDL_HINT_AXIS_DIGITAL_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_digital[0], SDL_CONTROLLER_AXIS_MAX);
    read_config_int_map_keys(HACKSDL_HINT_AXIS_DEADZONE_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_deadzone[0], SDL_CONTROLLER_AXIS_MAX);

    // axis virtual hack(HACKSDL_AXIS_MINUS_VIRTUAL_MAP/HACKSDL_AXIS_PLUS_VIRTUAL_MAP/HACKSDL_AXIS_VIRTUAL_SHARE)
    read_config_button_map_keys(HACKSDL_HINT_AXIS_MINUS_VIRTUAL_MAP_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_minus_virtual_map[0], SDL_CONTROLLER_AXIS_MAX);
    read_config_button_map_keys(HACKSDL_HINT_AXIS_PLUS_VIRTUAL_MAP_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_plus_virtual_map[0], SDL_CONTROLLER_AXIS_MAX);
    read_config_int_map_keys(HACKSDL_HINT_AXIS_VIRTUAL_SHARE_, SDL_CONTROLLER_AXIS_SHORTNAME, &config.axis_virtual_share[0], SDL_CONTROLLER_AXIS_MAX);

    // disable button if the virtual axis is in not in shared mode
    int button;
    for(int axis=0; axis<SDL_CONTROLLER_AXIS_MAX; axis++)
    {
        if(config.axis_virtual_share[axis] == SDL_FALSE)
        {
            button = config.axis_minus_virtual_map[axis];
            if((button = config.axis_minus_virtual_map[axis]) != SDL_CONTROLLER_BUTTON_INVALID)
            {
                config.button_disable[button] = SDL_TRUE;
            }
            else if((button = config.axis_plus_virtual_map[axis]) != SDL_CONTROLLER_BUTTON_INVALID)
            {
                config.button_disable[button] = SDL_TRUE;
            }

        }
    }
}

/*
    config value access functions (generic)
*/

/*
    read_config_string: read a string for the key entry
*/
int read_config_string(char* key, const char **str_value)
{
    if (file_config_present)
    {
        return read_config_file_string(key, str_value);
    }
    else
    {
        return read_config_env_string(key, str_value);
    }
}

/*
    read_config_button: read a string and return an SDL button
*/

int read_config_button(char* key, int *int_value)
{
    const char *str_value;
    if(read_config_string(key,&str_value))
    {
        *int_value = SDL_GameControllerGetButtonFromString(str_value);
        return 1;
    }
    else
    {
        *int_value = SDL_CONTROLLER_BUTTON_INVALID;
        return 0;
    }
}
/*
    read_config_int: read an int for the key entry
*/
int read_config_int(char* key, int *int_value)
{
    if (file_config_present)
    {
        return read_config_file_int(key, int_value);
    }
    else
    {
        return read_config_env_int(key, int_value);
    }
}

/*
    read_config_int_map: read a map of int for the key_prefix entry indexed with numbers
*/
int read_config_int_map_indexes(char* key_prefix, int *value_map, int length)
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
int read_config_int_map_keys(const char* key_prefix, const char** keys, int *value_map, int length)
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

int read_config_button_map_keys(const char* key_prefix, const char** keys, int *value_map, int length)
{

    char buffer[64];
    const char* value;
    int int_value;
    for(int index=0; index<length; index++ )
    {
        sprintf(buffer,key_prefix,keys[index]);
        read_config_button(buffer, &value_map[index]);
    }

    return 1;

}

/*
    config value access functions (file)
*/
int read_config_file_string(const char* key, const char **str_value)
{
    if(config_lookup_string(&cfg, key, str_value))
    {
        HACKSDL_debug("%s = %s", key, *str_value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", key);
        return 0;
    }
}

int read_config_file_int(const char* key, int *int_value)
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
            *int_value = (int)long_value;
            HACKSDL_debug("%s = %d",key, (int)long_value);
            return 1;
        }
    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }
    *int_value = 0;
    return 0;
}

/*
    config value access functions (env)
*/
int read_config_env_string(const char* key, const char **str_value){

    *str_value = SDL_GetHint(key);
    if(*str_value != NULL)
    {
        HACKSDL_debug("%s = %s", key, *str_value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }
    return 0;

}

int read_config_env_int(const char *key, int *int_value)
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
            *int_value = (int)long_value;
            HACKSDL_debug("%s = %d",key, (int)long_value);
            return 1;
        }

    }
    else
    {
        HACKSDL_debug("%s not found", key);
    }
    int_value = 0;
    return 0;
}