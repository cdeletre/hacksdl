#include <errno.h>
#include <libconfig.h>
#include <SDL2/SDL.h>
#include "configuration.h"
#include "debug.h"
#include <limits.h>

hacksdl_config_t config;
config_t cfg;
int file_config_present = 0;

/*
    default_config: init config with default values
*/
void default_config()
{
    config.verbose = 1;
    strcpy(config.libsdl_name,"RTLD_NEXT");
    config.no_gamecontroller = 0;
    config.modifier_button = SDL_CONTROLLER_BUTTON_INVALID;

    for(int axis=0;axis < HACKSDL_AXIS_INDEX_MAX; axis++)
    {
        config.modifier_shift[axis] = 0;
        config.axis_mode[axis] = 0;
        config.axis_deadzone[axis] = HACKSDL_AXIS_DEFAULT_DEADZONE;

    }

    for(int index=0;index < HACKSDL_MAP_INDEX_MAX; index++)
    { 
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
    HACKSDL_info("modifier_button=%s",sdl_controller_button_to_string(config.modifier_button));
    for(int axis=0;axis < HACKSDL_AXIS_INDEX_MAX; axis++)
    {
        if(config.modifier_shift[axis] > 0)
        {
            HACKSDL_info("axis=%d modifier_shift=%d", axis, config.modifier_shift[axis]);
        }
        HACKSDL_info("axis=%d mode=%d deadzone=%d", axis, config.axis_mode[axis], config.axis_deadzone[axis]);
    }

    for(int index=0;index < HACKSDL_MAP_INDEX_MAX; index++)
    {
        if(config.index_mapping[index] != index)
        {
            HACKSDL_info("index_mapping[%d]=%d",index, config.index_mapping[index]);
        }
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
    read_config_int_map(HACKSDL_HINT_MAP_INDEX_, &config.index_mapping[0], HACKSDL_MAP_INDEX_MAX);

    // no controller hack (HACKSDL_NO_GAMECONTROLLER)
    read_config_int(HACKSDL_HINT_NO_GAMECONTROLLER, &config.no_gamecontroller);

    // modifier hack (HACKSDL_MODIFIER_SHIFT_, HACKSDL_MODIFIER_BUTTON)
    read_config_int_map(HACKSDL_HINT_MODIFIER_SHIFT_, &config.modifier_shift[0], HACKSDL_AXIS_INDEX_MAX);

    if(read_config_string(HACKSDL_HINT_MODIFIER_BUTTON, &value))
    {
        config.modifier_button = get_gc_button(value);
    }
    else
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON not set");
    }

    // trigger hack (HACKSDL_AXIS_MODE/DEADZONE)
    read_config_int_map(HACKSDL_HINT_AXIS_MODE_, &config.axis_mode[0], HACKSDL_AXIS_INDEX_MAX);
    read_config_int_map(HACKSDL_HINT_AXIS_DEADZONE_, &config.axis_deadzone[0], HACKSDL_AXIS_INDEX_MAX);
}

/*
    config value access functions (generic)
*/
int read_config_string(char* str, const char **value)
{
    if (file_config_present)
    {
        return read_config_file_string(str, value);
    }
    else
    {
        return read_config_env_string(str, value);
    }
}

int read_config_int(char* str, int *value)
{
    if (file_config_present)
    {
        return read_config_file_int(str, value);
    }
    else
    {
        return read_config_env_int(str, value);
    }
}

int read_config_int_map(char* str, int *value_map, int length)
{
    if (file_config_present)
    {
        return read_config_file_int_map(str, value_map, length);
    }
    else
    {
        return read_config_env_int_map(str, value_map, length);
    }
}


/*
    config value access functions (file)
*/
int read_config_file_string(char* str, const char **value)
{
    if(config_lookup_string(&cfg, str, value))
    {
        HACKSDL_debug("%s = %s", str, *value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", str);
        return 0;
    }
}

int read_config_file_int(char* str, int *value)
{
    const char* str_value;
    char *endptr = NULL; 
    long long_value = 0;

    if(config_lookup_string(&cfg, str, &str_value))
    {
        errno = 0;
        long_value = strtol(str_value,&endptr,10);

        if (str_value == endptr)
        {
            HACKSDL_error("%s : %s invalid  (no digits found)", str, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MIN)
        {
            HACKSDL_error("%s : %s invalid  (underflow occurred)", str, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MAX)
        {
            HACKSDL_error("%s : %s invalid  (overflow occurred)", str, str_value);
            }
        else if (errno == EINVAL)  /* not in all c99 implementations - gcc OK */
        {
            HACKSDL_error("%s : %s invalid  (base contains unsupported value)", str, str_value);
        }
        else if (errno != 0 && long_value == 0)
        {
            HACKSDL_error("%s : %s invalid  (unspecified error occurred)", str, str_value);
        }
        else if (errno == 0 && str_value)
        {
            *value = (int)long_value;
            HACKSDL_debug("%s = %d",str, (int)long_value);
            return 1;
        }
    }
    else
    {
        HACKSDL_debug("%s not found", str);
    }

    return 0;
}

int read_config_file_int_map(char* str, int *value_map, int length)
{
    HACKSDL_debug("Loading %s map", str);
    char buffer[32];
    int int_value;
    for(int index=0; index<length; index++ )
    {
        sprintf(buffer,str,index);

        if(read_config_file_int(buffer, &int_value))
        {
            value_map[index] = int_value;
        }
    }
}

/*
    config value access functions (env)
*/
int read_config_env_string(char* str, const char **value){

    *value = SDL_GetHint(str);
    if(*value != NULL)
    {
        HACKSDL_debug("%s = %s", str, *value);
        return 1;
    }
    else
    {
        HACKSDL_debug("%s not found", str);
    }
    return 0;

}

int read_config_env_int(char *str, int *value)
{
    const char* str_value = SDL_GetHint(str);

    char *endptr = NULL; 
    long long_value = 0;

    if(str_value != NULL)
    {

        errno = 0;
        long_value = strtol(str_value,&endptr,10);

        if (str_value == endptr)
        {
            HACKSDL_error("%s : %s invalid  (no digits found)", str, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MIN)
        {
            HACKSDL_error("%s : %s invalid  (underflow occurred)", str, str_value);
        }
        else if (errno == ERANGE && long_value == LONG_MAX)
        {
            HACKSDL_error("%s : %s invalid  (overflow occurred)", str, str_value);
        }
        else if (errno == EINVAL)  /* not in all c99 implementations - gcc OK */
        {
            HACKSDL_error("%s : %s invalid  (base contains unsupported value)", str, str_value);
        }
        else if (errno != 0 && long_value == 0)
        {
            HACKSDL_error("%s : %s invalid  (unspecified error occurred)", str, str_value);
        }
        else if (errno == 0 && str_value)
        {
            *value = (int)long_value;
            HACKSDL_debug("%s = %d",str, (int)long_value);
            return 1;
        }

    }
    else
    {
        HACKSDL_debug("%s not found", str);
    }

    return 0;
}

int read_config_env_int_map(char* str, int *value_map, int length)
{
    HACKSDL_debug("Loading %s map", str);
    char buffer[32];
    int int_value;
    for(int index=0; index<length; index++ )
    {
        sprintf(buffer,str,index);

        if(read_config_env_int(buffer, &int_value))
        {
            *value_map = int_value;
        }
        else
        {
            *value_map = index;
        }
        value_map++;
    }
}

/*
    get_gc_button: get the SDL_GameControllerButton from the button name (A,B...)
*/
SDL_GameControllerButton get_gc_button(const char* modifier_button_str)
{
    if(strcmp(modifier_button_str,"A") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = A");
        return SDL_CONTROLLER_BUTTON_A;
    }
    else if(strcmp(modifier_button_str,"B") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = B");
        return SDL_CONTROLLER_BUTTON_B;
    }
    else if(strcmp(modifier_button_str,"X") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = X");
        return SDL_CONTROLLER_BUTTON_X;
    }
    else if(strcmp(modifier_button_str,"Y") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = Y");
        return SDL_CONTROLLER_BUTTON_Y;
    }
    else if(strcmp(modifier_button_str,"BACK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = BACK");
        return SDL_CONTROLLER_BUTTON_BACK;
    }
    else if(strcmp(modifier_button_str,"GUIDE") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = GUIDE");
        return SDL_CONTROLLER_BUTTON_GUIDE;
    }
    else if(strcmp(modifier_button_str,"START") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = START");
        return SDL_CONTROLLER_BUTTON_START;
    }
    else if(strcmp(modifier_button_str,"LEFTSTICK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = LEFTSTICK");
        return SDL_CONTROLLER_BUTTON_LEFTSTICK;
    }
    else if(strcmp(modifier_button_str,"RIGHTSTICK") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = RIGHTSTICK");
        return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    }
    else if(strcmp(modifier_button_str,"LEFTSHOULDER") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = LEFTSHOULDER");
        return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    }
    else if(strcmp(modifier_button_str,"RIGHTSHOULDER") == 0)
    {
        HACKSDL_debug("HACKSDL_MODIFIER_BUTTON = RIGHTSHOULDER");
        return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    }
    else
    {
        HACKSDL_error("incorrect HACKSDL_MODIFIER_BUTTON: %s", modifier_button_str);
    }
}

/*
    sdl_controller_button_to_string: get the button name (A,B...) from the SDL_GameControllerButton value
*/
const char* sdl_controller_button_to_string(SDL_GameControllerButton button)
{
    switch(button)
    {
        case SDL_CONTROLLER_BUTTON_A:
            return "A";
            break;
        case SDL_CONTROLLER_BUTTON_B:
            return "B";
            break;
        case SDL_CONTROLLER_BUTTON_X:
            return "X";
            break;
        case SDL_CONTROLLER_BUTTON_Y:
            return "Y";
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            return "BACK";
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return "GUIDE";
            break;
        case SDL_CONTROLLER_BUTTON_START:
            return "START";
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return "LEFTSTICK";
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return "RIGHTSTICK";
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return "LEFTSHOULDER";
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "RIGHTSHOULDER";
            break;
        case SDL_CONTROLLER_BUTTON_INVALID:
            return "INVALID";
            break;
        default:
            return "UNKNOWN";
    } 
}