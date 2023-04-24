#ifndef ESP8266LOG_H
#define ESP8266LOG_H

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_VERBOSE 5

#ifndef CORE_DEBUG_LEVEL
#define CORE_DEBUG_LEVEL LOG_LEVEL_VERBOSE
#endif

#ifndef CONFIG_ARDUHAL_LOG_COLORS
#define CONFIG_ARDUHAL_LOG_COLORS 0
#endif

#if CONFIG_ARDUHAL_LOG_COLORS
#define ARDUHAL_LOG_COLOR_BLACK   "30"
#define ARDUHAL_LOG_COLOR_RED     "31" //ERROR
#define ARDUHAL_LOG_COLOR_GREEN   "32" //INFO
#define ARDUHAL_LOG_COLOR_YELLOW  "33" //WARNING
#define ARDUHAL_LOG_COLOR_BLUE    "34"
#define ARDUHAL_LOG_COLOR_MAGENTA "35"
#define ARDUHAL_LOG_COLOR_CYAN    "36" //DEBUG
#define ARDUHAL_LOG_COLOR_GRAY    "37" //VERBOSE
#define ARDUHAL_LOG_COLOR_WHITE   "38"

#define ARDUHAL_LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define ARDUHAL_LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define ARDUHAL_LOG_RESET_COLOR   "\033[0m"

#define ARDUHAL_LOG_COLOR_E       ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_RED)
#define ARDUHAL_LOG_COLOR_W       ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_YELLOW)
#define ARDUHAL_LOG_COLOR_I       ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_GREEN)
#define ARDUHAL_LOG_COLOR_D       ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_CYAN)
#define ARDUHAL_LOG_COLOR_V       ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_GRAY)
#define ARDUHAL_LOG_COLOR_PRINT(letter) log_printf(ARDUHAL_LOG_COLOR_ ## letter)
#define ARDUHAL_LOG_COLOR_PRINT_END log_printf(ARDUHAL_LOG_RESET_COLOR)
#else
#define ARDUHAL_LOG_COLOR_E
#define ARDUHAL_LOG_COLOR_W
#define ARDUHAL_LOG_COLOR_I
#define ARDUHAL_LOG_COLOR_D
#define ARDUHAL_LOG_COLOR_V
#define ARDUHAL_LOG_RESET_COLOR
#define ARDUHAL_LOG_COLOR_PRINT(letter)
#define ARDUHAL_LOG_COLOR_PRINT_END
#endif

#define log_n(fmt, ...)
#define log_e(fmt, ...) if(CORE_DEBUG_LEVEL >= LOG_LEVEL_ERROR) { Serial.printf(ARDUHAL_LOG_COLOR_E "[%6u][E][%s:%d] %s(): " fmt ARDUHAL_LOG_RESET_COLOR "\r\n",(unsigned long) (micros() / 1000ULL), __FILE__, __LINE__, __func__, ##__VA_ARGS__); }
#define log_w(fmt, ...) if(CORE_DEBUG_LEVEL >= LOG_LEVEL_WARN) { Serial.printf(ARDUHAL_LOG_COLOR_W "[%6u][W][%s:%d] %s(): " fmt ARDUHAL_LOG_RESET_COLOR "\r\n",(unsigned long) (micros() / 1000ULL), __FILE__, __LINE__, __func__, ##__VA_ARGS__); }
#define log_i(fmt, ...) if(CORE_DEBUG_LEVEL >= LOG_LEVEL_INFO) { Serial.printf(ARDUHAL_LOG_COLOR_I "[%6u][I][%s:%d] %s(): " fmt ARDUHAL_LOG_RESET_COLOR "\r\n",(unsigned long) (micros() / 1000ULL), __FILE__, __LINE__, __func__, ##__VA_ARGS__); }
#define log_d(fmt, ...) if(CORE_DEBUG_LEVEL >= LOG_LEVEL_DEBUG) { Serial.printf(ARDUHAL_LOG_COLOR_D "[%6u][D][%s:%d] %s(): " fmt ARDUHAL_LOG_RESET_COLOR "\r\n",(unsigned long) (micros() / 1000ULL), __FILE__, __LINE__, __func__, ##__VA_ARGS__); }
#define log_v(fmt, ...) if(CORE_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE) { Serial.printf(ARDUHAL_LOG_COLOR_V "[%6u][V][%s:%d] %s(): " fmt ARDUHAL_LOG_RESET_COLOR "\r\n",(unsigned long) (micros() / 1000ULL), __FILE__, __LINE__, __func__, ##__VA_ARGS__); }

#endif // ESP8266LOG_H
