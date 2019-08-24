/*
 * Parent include file for LogicSim. Contains preprocessor-defined constants.
 */

#ifndef LGS_INCLUDE_LOGIC_SIM
#define LGS_INCLUDE_LOGIC_SIM 

/*
 * Turn this on for the profiler
 */
//#define LGS_PROFILE

#ifdef LGS_PROFILE

/*
 * The number of samples to average over
 */
#define LGS_PROFILE_N_SAMPLES 100

#endif

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
 * The number of milliseconds to wait between listening for keyboard events. Use the time-keyboard script to find the 
 * system-specific optimal value.
 */
#define LGS_KEYBOARD_WAIT_TIME 25

/*
 * To optimise tick rate, bools are packed into uints as a LGS_PACK_WIDTH X LGS_PACK_HEIGHT block. Note, must satisfy 
 * LGS_PACK_WIDTH X LGS_PACK_HEIGHT <= sizeof(lgs::pack_uint_t), equality is most optimal. See cpuworker.hpp for definition
 * of lgs::pack_uint_t.
 */
#define LGS_PACK_WIDTH 8
#define LGS_PACK_HEIGHT 4


#endif 
