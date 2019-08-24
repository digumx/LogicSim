/*
 * File containing code related to simulating chunks of the logic system on the CPU.
 */

#ifndef LGS_INCLUDE_CPU_WORKER
#define LGS_INCLUDE_CPU_WORKER 

#include <vector>
#include <cstdint>
#include <array>

#include <logicsim.hpp>

#ifdef LGS_PROFILE
#include <chrono>
#endif

namespace lgs
{
        // Forward declaration
        class Peripheral;
#ifdef LGS_PROFILE
        class PrintSection;
#endif

        /*
         * Typedef for packed type
         */
        typedef uint32_t pack_uint_t;

        /*
         * A CPU worker class that simulates a specified chunk of the logic board. Handles its own state memory.
         *
         * Note: States outside the logic board boundary are assumed to be 0.
         *
         */
        class CPUWorker
        {
                private:
                        std::array<pack_uint_t*, 20> circuit_data;
                        const int width;                                            // dimensions after packing
                        const int height;
                        pack_uint_t* state_r;                                          // Last state, to be read.
                        pack_uint_t* state_w;                                          // Next state, to be written.
                        const std::vector<Peripheral*> peripherals;                        
#ifdef LGS_PROFILE
                        int profile_n_ticks;
                        std::chrono::steady_clock::duration profile_time_logic;
                        std::chrono::steady_clock::duration profile_time_peripherals;
                        PrintSection* prof_sec;
#endif 


                        void sim_step(const pack_uint_t* state_r, pack_uint_t* state_w);      // Simulate one step
                public:
                        CPUWorker(const unsigned int* crd,                      // 2d array of uints, each uint contains 20 bits of circuit
                                                                                // data in the order they appear in RGB encoding.
                                        const int w, const int h,               // dimensions of crd
                                        const std::vector<Peripheral*>& ps);    // peripherals
                        ~CPUWorker();

                        void tickSimulation();                                  // Simulate one step
                        bool getStateAt(int x_u, int y_u);                      // Get state at unpacked coords (x_u, y_u).
                        void setStateAt(int x_u, int y_u, bool state);          // Set state at unpacked coords (x_u, y_u).
                        bool* getState();                                       // Returns copy of current(last) state, unpacked. 
                                                                                // Does not allow state to be modified externally. 

        };
}

#endif
