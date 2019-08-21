/*
 * Implementation for cpuworker.hpp
 */

#include <vector>

#include <peripherals.hpp>
#include <logicsim.hpp>

#ifdef LGS_PROFILE
#include <chrono>
#include <string>
#include <sstream>
#include <ncursesio.hpp>
#endif

#include <cpuworker.hpp>

lgs::CPUWorker::CPUWorker(const unsigned int* crd, const int w, const int h, const std::vector<Peripheral*>& ps)
        : circuit_data(crd), width(w), height(h), peripherals(ps)
{
        state_r = new bool[w*h];
        state_w = new bool[w*h];
        for(int i = 0; i < w*h; i++)
        {
                state_r[i] = false;
                state_w[i] = false;
        }
#ifdef LGS_PROFILE
        prof_sec = new lgs::PrintSection();
#endif
}

lgs::CPUWorker::~CPUWorker()
{
        delete[] state_r;
        delete[] state_w;
#ifdef LGS_PROFILE
        delete prof_sec;
#endif
}

void lgs::CPUWorker::sim_step(const bool* state_r, bool* state_w)
{
       for(int y = 0; y < height; y++)
               for(int x = 0; x < width; x++)
               {
                       int x0 = x + 1 + ((circuit_data[y*width+x]>>16) % 2);
                       int y1 = y - 1 - ((circuit_data[y*width+x]>>17) % 2);
                       int x2 = x - 1 - ((circuit_data[y*width+x]>>18) % 2);
                       int y3 = y + 1 + ((circuit_data[y*width+x]>>19) % 2);
                       bool a0 = x0 < width ? state_r[y*width + x0] : false;
                       bool a1 = y1 >= 0 ? state_r[y1*width + x] : false;
                       bool a2 = x2 >= 0 ? state_r[y*width + x2] : false;
                       bool a3 = y3 < height ? state_r[y3*width + x] : false;
                       int ws = a3 ? circuit_data[y*width + x] / 256 : circuit_data[y*width + x];
                       ws = a2 ? ws / 16 : ws;
                       ws = a1 ? ws / 4 : ws;
                       ws = a0 ? ws / 2 : ws;
                       state_w[y*width + x] = ws % 2 == 1;
             } 
}

void lgs::CPUWorker::tickSimulation()
{
#ifdef LGS_PROFILE
        std::chrono::steady_clock::time_point t0, t1;
        t0 = std::chrono::steady_clock::now();
#endif 
        this->sim_step(state_r, state_w);
#ifdef LGS_PROFILE 
        t1 = std::chrono::steady_clock::now();
        profile_time_logic += t1 - t0;
#endif

        for(std::vector<Peripheral*>::const_iterator peri = peripherals.begin(); peri != peripherals.end(); ++peri)
        {
                PeripheralInterface& in = (*peri)->getInputInterface();
                PeripheralInterface& out = (*peri)->getOutputInterface();
                for(PeripheralInterface::iterator i = in.begin(); i != in.end(); ++i)
                        if(std::get<0>(*i) >= 0 && std::get<0>(*i) < width && std::get<1>(*i) >= 0 && std::get<1>(*i) < height)
                                std::get<2>(*i) = state_r[width * std::get<1>(*i) + std::get<0>(*i)];
                (*peri)->tick();
                for(PeripheralInterface::iterator i = out.begin(); i != out.end(); ++i)
                        if(std::get<0>(*i) >= 0 && std::get<0>(*i) < width && std::get<1>(*i) >= 0 && std::get<1>(*i) < height)
                                state_w[width * std::get<1>(*i) + std::get<0>(*i)] = std::get<2>(*i);

        }

#ifdef LGS_PROFILE
        t0 = std::chrono::steady_clock::now();
        profile_time_peripherals += t0 - t1;
        ++profile_n_ticks;
        if(profile_n_ticks >= LGS_PROFILE_N_SAMPLES)
        {
                profile_time_logic /= profile_n_ticks;
                profile_time_peripherals /= profile_n_ticks;
                std::stringstream str;
                str << "CPU Worker Profiling.\n";
                str << "Average tick time over " << profile_n_ticks << " ticks for logic tick is ";
                str << std::chrono::duration_cast<std::chrono::microseconds>(profile_time_logic).count();
                str << " microseconds and for peripheral tick is ";
                str << std::chrono::duration_cast<std::chrono::microseconds>(profile_time_peripherals).count();
                str << "microseconds.\n";
                prof_sec->setText(str.str());
                profile_n_ticks = 0;
                profile_time_logic = std::chrono::steady_clock::duration(0);
                profile_time_peripherals = std::chrono::steady_clock::duration(0);
        }
#endif
        bool* tmp = state_r;
        state_r = state_w;
        state_w = tmp;
}

const bool* lgs::CPUWorker::getState()
{
        return state_r;
}
