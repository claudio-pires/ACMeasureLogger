#ifndef UTIL_H_
#define UTIL_H_

#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

#endif /* UTIL_H_ */