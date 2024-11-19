#include <SDL2/SDL.h>


#define SDL_AXIS_MAX                            32767
#define SDL_AXIS_MIN                            -32768
#define SDL_BUTTON_COUNT                        SDL_CONTROLLER_BUTTON_MAX - SDL_CONTROLLER_BUTTON_A

#define HACKSDL_LIBSDL_NAME_LMAX                32
#define HACKSDL_DEVICE_INDEX_MAX                8
#define HACKSDL_AXIS_DEFAULT_DEADZONE           -1


#define HACKSDL_HINT_VERBOSE                    "HACKSDL_VERBOSE"
#define HACKSDL_HINT_LIBSDL_NAME                "HACKSDL_LIBSDL_NAME"
#define HACKSDL_HINT_CONFIG_FILE                "HACKSDL_CONFIG_FILE"
#define HACKSDL_HINT_DEVICE_MAP_INDEX_          "HACKSDL_DEVICE_MAP_INDEX_%d"
#define HACKSDL_HINT_DEVICE_DISABLE_            "HACKSDL_DEVICE_DISABLE_%d"
#define HACKSDL_HINT_NO_GAMECONTROLLER          "HACKSDL_NO_GAMECONTROLLER"
#define HACKSDL_HINT_AXIS_MODIFIER_BUTTON       "HACKSDL_AXIS_MODIFIER_BUTTON"
#define HACKSDL_HINT_AXIS_MODIFIER_SHIFT_       "HACKSDL_AXIS_MODIFIER_SHIFT_%s"
#define HACKSDL_HINT_AXIS_DIGITAL_              "HACKSDL_AXIS_DIGITAL_%s"
#define HACKSDL_HINT_AXIS_DEADZONE_             "HACKSDL_AXIS_DEADZONE_%s"
#define HACKSDL_HINT_AXIS_VIRTUAL_MINUS_MAP_    "HACKSDL_AXIS_VIRTUAL_MINUS_MAP_%s"
#define HACKSDL_HINT_AXIS_VIRTUAL_PLUS_MAP_     "HACKSDL_AXIS_VIRTUAL_PLUS_MAP_%s"
#define HACKSDL_HINT_AXIS_VIRTUAL_SHARE_        "HACKSDL_AXIS_VIRTUAL_SHARE_%s"
#define HACKSDL_HINT_DPAD_VIRTUAL_X_MAP         "HACKSDL_DPAD_VIRTUAL_X_MAP"
#define HACKSDL_HINT_DPAD_VIRTUAL_Y_MAP         "HACKSDL_DPAD_VIRTUAL_Y_MAP"
#define HACKSDL_HINT_DPAD_VIRTUAL_SHARE         "HACKSDL_DPAD_VIRTUAL_SHARE"

struct hacksdl_config{
    int verbose;                                            // 0: none, 1: info, 2: debug
    char libsdl_name[HACKSDL_LIBSDL_NAME_LMAX];
    int no_gamecontroller;                                  // 0: disabled, 1: mode 1, 2: mode 2
    SDL_GameControllerButton modifier_button;
    int device_disable[HACKSDL_DEVICE_INDEX_MAX];           // 0: disabled, 1: mode 1, 2: mode 2
    int axis_modifier_shift[SDL_CONTROLLER_AXIS_MAX];       // right shift
    int controller_index_mapping[HACKSDL_DEVICE_INDEX_MAX];
    int axis_digital[SDL_CONTROLLER_AXIS_MAX];              // SDL_TRUE / SDL_FALSE
    int axis_deadzone[SDL_CONTROLLER_AXIS_MAX];
    int axis_virtual_minus_map[SDL_CONTROLLER_AXIS_MAX];
    int axis_virtual_plus_map[SDL_CONTROLLER_AXIS_MAX];
    int axis_virtual_share[SDL_CONTROLLER_AXIS_MAX];
    int button_disable[SDL_BUTTON_COUNT];                   // SDL_TRUE / SDL_FALSE
    int dpad_x_virtual_map;                                 //TODO
    int dpad_y_virtual_map;                                 //TODO
    int dpad_virtual_share;                                 //TODO
    int axis_disable[SDL_CONTROLLER_AXIS_MAX];              //TODO
};

typedef struct hacksdl_config hacksdl_config_t;

void load_config();
void default_config();
int open_config_file();
int load_config_data();

int read_config_string(char* key, const char **str_value);
int read_config_button(char* key, int *int_value);
int read_config_int(char* key, int *int_value);
int read_config_int_map_indexes(char* key_prefix, int *value_map, int length);
int read_config_int_map_keys(const char* key_prefix, const char** keys, int *value_map, int length);
int read_config_button_map_keys(const char* key_prefix, const char** keys, int *value_map, int length);

int read_config_file_string(const char* key, const char **str_value);
int read_config_file_int(const char* key, int *int_value);

int read_config_env_string(const char* key, const char **str_value);
int read_config_env_int(const char* key, int *int_value);
