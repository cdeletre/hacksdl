void sdterr_yellow ();
void sdterr_red ();
void sdterr_blue ();
void sdterr_reset ();

void HACKSDL_info(char* format, ...);
void HACKSDL_error_func(const char* func_name, char* format, ...);
void HACKSDL_debug_func(const char* func_name, char* format, ...);

#define HACKSDL_debug(...) HACKSDL_debug_func(__func__, __VA_ARGS__)
#define HACKSDL_error(...) HACKSDL_error_func(__func__, __VA_ARGS__)