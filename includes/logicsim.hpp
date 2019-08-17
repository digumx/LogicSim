/*
 * Parent include file for LogicSim. Contains preprocessor-defined constants.
 */

#ifndef LGS_INCLUDE_LOGIC_SIM
#define LGS_INCLUDE_LOGIC_SIM 

// FOR DEBUGGING
//#define LGS_DEBUG

/*
 * The default number of ticks for which to simulate. Negative values indicate indefinite simulation.
 */
#define LGS_DEFAULT_TIME_LENGTH -1

/*
 * The default step size after which to print intermediate state. 0 or negative values indicate no intermediate state priting.
 * States are compiled into a gif animation whose filename is .out.gif appended to the input filename.
 */
#define LGS_DEFAULT_PRINT_STEPS -1

/*
 * The default time in milliseconds between each frame of the outputted gif.
 */
#define LGS_DEFAULT_FRAMETIME 100

/*
 * The default output image scaling factor
 */
#define LGS_DEFAULT_SCALE_FACTOR 2

/*
 * RGBA Color value to use for 0 state in output gif.
 */
#define LGS_GIF_COLOR_0_R 0
#define LGS_GIF_COLOR_0_G 0
#define LGS_GIF_COLOR_0_B 0
#define LGS_GIF_COLOR_0_A 255

/*
 * RGBA color value to use for 1 state in output gif.
 */
#define LGS_GIF_COLOR_1_R 255
#define LGS_GIF_COLOR_1_G 255
#define LGS_GIF_COLOR_1_B 255
#define LGS_GIF_COLOR_1_A 255

/*
 * The number of ticks over which to average tick timing and the number of ticks after which to print timing data is stored in LGS_PROFILE_TICKS.
 * This provides a default value
 */
#ifdef LGS_PROFILE
#ifndef LGS_PROFILE_TICKS
#define LGS_PROFILE_TICKS 256
#endif
#endif

/*
 * The number of milliseconds to wait between listening for keyboard events. Use the time-keyboard script to find the system-specific optimal value.
 */
#define LGS_KEYBOARD_WAIT_TIME 25

#endif 
