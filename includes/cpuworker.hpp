/*
 * File containing code related to simulating chunks of the logic system on the CPU.
 */

#ifndef LGS_INCLUDE_CPU_WORKER
#define LGS_INCLUDE_CPU_WORKER 

#include <vector>

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
         * A CPU worker class that simulates a specified chunk of the logic board. Handles its own state memory.
         *
         * Note: States outside the logic board boundary are assumed to be 0.
         *
         */
        class CPUWorker
        {
                private:
                        const unsigned int* circuit_data;
                        const int width;
                        const int height;
                        bool* state_r;                                          // Last state, to be read.
                        bool* state_w;                                          // Next state, to be written.
                        const std::vector<Peripheral*> peripherals;                        
#ifdef LGS_PROFILE
                        int profile_n_ticks;
                        std::chrono::steady_clock::duration profile_time_logic;
                        std::chrono::steady_clock::duration profile_time_peripherals;
                        PrintSection* prof_sec;
#endif 


                        void sim_step(const bool* state_r, bool* state_w);      // Simulate one step
                public:
                        CPUWorker(const unsigned int* crd, const int w, const int h, const std::vector<Peripheral*>& ps);    // crd is circuit_data
                        ~CPUWorker();

                        void tickSimulation();                                  // Simulate one step
                        const bool* getState();                                 // Returns current(last) state. Does not allow state to be modified externally. 

        };
}

#endif
