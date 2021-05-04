/*

 This file contains some utilities to call "sleep()" only if it exists,
 so that we don't get compilation errors before it is implemented.

 */
#include <stdio.h>
#include <syscall.h>


// If we don't have sleep yet, provide a small macro that prevents compilation errors.
#ifdef NO_SLEEP_YET
#define sleep(x)                                                        \
    do {                                                                \
        printf("---------------------\n"								\
			"You can not run this program before you have implemented the\n"	\
            "system call sleep(). If you have implemented sleep(), remove the line\n" \
            "  #define NO_SLEEP_YET\n"									\
            "in the file examples/sleep.h and re-compile examples.\n"	\
			"---------------------\n");									\
        exit(-1);                                                       \
    } while (false);
#endif
