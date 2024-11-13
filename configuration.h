#include <SDL2/SDL.h>


#define SDL_AXIS_MAX                            32767
#define SDL_AXIS_MIN                            -32768

#define HACKSDL_LIBSDL_NAME_LMAX                32
#define HACKSDL_DEVICE_INDEX_MAX                   16
#define HACKSDL_AXIS_DEFAULT_DEADZONE           -1


#define HACKSDL_HINT_LIBSDL_NAME                "HACKSDL_LIBSDL_NAME"
#define HACKSDL_HINT_CONFIG_FILE                "HACKSDL_CONFIG_FILE"
#define HACKSDL_HINT_MAP_INDEX_                 "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_HINT_DISABLE_DEVICE_            "HACKSDL_DISABLE_DEVICE_%d"
#define HACKSDL_HINT_VERBOSE                    "HACKSDL_VERBOSE"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_MODIFIER_BUTTON            "HACKSDL_MODIFIER_BUTTON"
#define HACKSDL_HINT_MODIFIER_SHIFT_            "HACKSDL_MODIFIER_SHIFT_%s"
#define HACKSDL_HINT_AXIS_DIGITAL_              "HACKSDL_AXIS_DIGITAL_%s"
#define HACKSDL_HINT_AXIS_DEADZONE_             "HACKSDL_AXIS_DEADZONE_%s"

struct hacksdl_config{
    int verbose;
    char libsdl_name[HACKSDL_LIBSDL_NAME_LMAX];
    int no_gamecontroller;
    SDL_GameControllerButton modifier_button;
    int disable_device[HACKSDL_DEVICE_INDEX_MAX];
    int modifier_shift[SDL_CONTROLLER_AXIS_MAX];
    int index_mapping[HACKSDL_DEVICE_INDEX_MAX];
    int axis_digital[SDL_CONTROLLER_AXIS_MAX];
    int axis_deadzone[SDL_CONTROLLER_AXIS_MAX];
};

typedef struct hacksdl_config hacksdl_config_t;

void load_config();
void default_config();
int open_config_file();
int load_config_data();

int read_config_string(char* key, const char **value);
int read_config_int(char* key, int *value);
int read_config_int_map(char* key_prefix, int *value_map, int length);
int read_config_int_map_b(const char* key_prefix, const char** keys, int *value_map, int length);

int read_config_file_string(const char* key, const char **value);
int read_config_file_int(const char* key, int *value);

int read_config_env_string(const char* key, const char **value);
int read_config_env_int(const char* key, int *value);

SDL_GameControllerButton get_gc_button(const char* modifier_button_hint);
const char* sdl_controller_button_name(SDL_GameControllerButton button);
const char* get_axis_name(SDL_GameControllerAxis axis);
const char* get_axis_shortname(SDL_GameControllerAxis axis);
