#include <SDL2/SDL.h>


#define SDL_AXIS_MAX                            32767
#define SDL_AXIS_MIN                            -32768

#define HACKSDL_LIBSDL_NAME_LMAX                32
#define HACKSDL_DEVICE_INDEX_MAX                   16
#define HACKSDL_AXIS_INDEX_MAX                  8
#define HACKSDL_AXIS_DEFAULT_DEADZONE           -1


#define HACKSDL_HINT_LIBSDL_NAME                "HACKSDL_LIBSDL_NAME"
#define HACKSDL_HINT_CONFIG_FILE                "HACKSDL_CONFIG_FILE"
#define HACKSDL_HINT_MAP_INDEX_                 "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_HINT_DISABLE_DEVICE_            "HACKSDL_DISABLE_DEVICE_%d"
#define HACKSDL_HINT_VERBOSE                    "HACKSDL_VERBOSE"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_MODIFIER_BUTTON            "HACKSDL_MODIFIER_BUTTON"
#define HACKSDL_HINT_MODIFIER_SHIFT_            "HACKSDL_MODIFIER_SHIFT_%d"
#define HACKSDL_HINT_AXIS_MODE_                 "HACKSDL_AXIS_MODE_%d"
#define HACKSDL_HINT_AXIS_DEADZONE_             "HACKSDL_AXIS_DEADZONE_%d"

struct hacksdl_config{
    int verbose;
    char libsdl_name[HACKSDL_LIBSDL_NAME_LMAX];
    int no_gamecontroller;
    SDL_GameControllerButton modifier_button;
    int disable_device[HACKSDL_DEVICE_INDEX_MAX];
    int modifier_shift[HACKSDL_AXIS_INDEX_MAX];
    int index_mapping[HACKSDL_DEVICE_INDEX_MAX];
    int axis_mode[HACKSDL_AXIS_INDEX_MAX];
    int axis_deadzone[HACKSDL_AXIS_INDEX_MAX];
};

typedef struct hacksdl_config hacksdl_config_t;

void load_config();
void default_config();
int open_config_file();
int load_config_data();

int read_config_string(char* str, const char **value);
int read_config_int(char* str, int *value);
int read_config_int_map(char* str, int *value_map, int length);

int read_config_file_string(char* str, const char **value);
int read_config_file_int(char* str, int *value);
int read_config_file_int_map(char* str, int *value_map, int length);

int read_config_env_string(char* str, const char **value);
int read_config_env_int(char* str, int *value);
int read_config_env_int_map(char* str, int *value_map, int length);

SDL_GameControllerButton get_gc_button(const char* modifier_button_hint);
const char* sdl_controller_button_to_string(SDL_GameControllerButton button);