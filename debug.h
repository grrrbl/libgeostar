#if DEBUG_LVL > 0
#define INFO(...) printf(__VA_ARGS__);
#else
#define INFO(...) 
#endif

#if DEBUG_LVL > 1
#define VERBOSE(...) \
    { \
    char msg[64]; \
    printf(msg, __VA_ARGS__); \
    }
#else
#define VERBOSE(...) 
#endif

#if DEBUG_LVL > 2
#define DEBUG(...) printf(__VA_ARGS__);
#else
#define DEBUG(...) 
#endif

