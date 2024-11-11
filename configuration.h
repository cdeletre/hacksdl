#include <SDL2/SDL.h>


#define SDL_AXIS_MAX                            32767
#define SDL_AXIS_MIN                            -37768

#define HACKSDL_LIBSDL_NAME_LMAX                32
#define HACKSDL_MAP_INDEX_MAX                   16
#define HACKSDL_AXIS_INDEX_MAX                  8
#define HACKSDL_AXIS_DEFAULT_MAX                SDL_AXIS_MAX
#define HACKSDL_AXIS_DEFAULT_MIN                0
#define HACKSDL_AXIS_DEFAULT_THRESHOLD          16384


#define HACKSDL_HINT_LIBSDL_NAME                "HACKSDL_LIBSDL_NAME"
#define HACKSDL_HINT_CONFIG_FILE                "HACKSDL_CONFIG_FILE"
#define HACKSDL_HINT_MAP_INDEX_                 "HACKSDL_MAP_INDEX_%d"
#define HACKSDL_HINT_VERBOSE                    "HACKSDL_VERBOSE"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_MODIFIER_BUTTON            "HACKSDL_MODIFIER_BUTTON"
#define HACKSDL_HINT_MODIFIER_SHIFT_            "HACKSDL_MODIFIER_SHIFT_%d"
#define HACKSDL_HINT_AXIS_DIGITAL_              "HACKSDL_AXIS_DIGITAL_%d"
#define HACKSDL_HINT_AXIS_THRESHOLD_            "HACKSDL_AXIS_THRESHOLD_%d"
#define HACKSDL_HINT_AXIS_MIN_                  "HACKSDL_AXIS_MIN_%d"
#define HACKSDL_HINT_AXIS_MAX_                  "HACKSDL_AXIS_INDEX_MAX_%d"

struct hacksdl_config{
    int verbose;
    char libsdl_name[HACKSDL_LIBSDL_NAME_LMAX];
    int no_gamecontroller;
    SDL_GameControllerButton modifier_button;
    int modifier_shift[HACKSDL_AXIS_INDEX_MAX];
    int index_mapping[HACKSDL_MAP_INDEX_MAX];
    int axis_digital[HACKSDL_AXIS_INDEX_MAX];
    int axis_threshold[HACKSDL_AXIS_INDEX_MAX];
    int axis_min[HACKSDL_AXIS_INDEX_MAX];
    int axis_max[HACKSDL_AXIS_INDEX_MAX];
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