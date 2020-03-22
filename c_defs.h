#ifndef C_DEFS_H_
#define C_DEFS_H_

#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG_ERROR(string, ...)  fprintf(stderr, "%s: %s:%d; " string, "[B2-ERROR]", __FUNCTION__, __LINE__ VA_ARGS(__VA_ARGS__))
#define LOG_DEBUG(STRING, ...)	fprintf(stdout, "%s: %s:%d; " STRING, "[B2-DEBUG]", __FUNCTION__, __LINE__ VA_ARGS(__VA_ARGS__))

#endif //C_DEFS_H_
