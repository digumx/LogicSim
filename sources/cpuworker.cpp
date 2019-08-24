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

#ifdef LGS_DEBUG
// It's debug, and NRVO is a thing. Thus return by valueing.
std::string to_hex_code(lgs::pack_uint_t n)
{
        char digs[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        constexpr size_t s = sizeof(lgs::pack_uint_t) * 2;
        char repr[s];
        for(size_t i = 0; i < s; i++)
        {
                repr[i] = digs[n % 16];
                n /= 16;
        }
        return std::string(repr);
}
#endif

lgs::CPUWorker::CPUWorker(const unsigned int* crd, const int w, const int h, const std::vector<Peripheral*>& ps)
        : peripherals(ps), width(w % LGS_PACK_WIDTH == 0 ? w/LGS_PACK_WIDTH : w/LGS_PACK_WIDTH + 1),
            height(h % LGS_PACK_HEIGHT == 0 ? h/LGS_PACK_HEIGHT : h/LGS_PACK_HEIGHT + 1)
{
        state_r = new pack_uint_t[width*height];
        state_w = new pack_uint_t[width*height];
        for(int i = 0; i < 20; i++)
            circuit_data[i] = new uint32_t[width*height];
        for(int i = 0; i < width*height; i++)
        {
                state_r[i] = 0;
                state_w[i] = 0;
        }
        for(int x_p = 0; x_p < width; x_p++)
                for(int y_p = 0; y_p < height; y_p++)
                {
                        for(int i = 0; i < 20; ++i) circuit_data[i][y_p * width + x_p] = 0u;
                        for(int x_s = 0; x_s < LGS_PACK_WIDTH; ++x_s)
                                for(int y_s = 0; y_s < LGS_PACK_HEIGHT; ++y_s)
                                        for(int i = 0; i < 20; i++)
                                                circuit_data[i][y_p * width + x_p] |=
                                                        ((crd[(y_p * LGS_PACK_WIDTH + y_s) * w + (x_p * LGS_PACK_HEIGHT + x_s)] >> i) | 1u ) 
                                                        << (y_s * LGS_PACK_WIDTH + x_s);
#ifdef LGS_DEBUG
                        lgs::print("Circuit data at ");
                        lgs::print(std::to_string(x_p));
                        lgs::print(", ");
                        lgs::print(std::to_string(y_p));
                        for(int i = 0; i < 20; ++i) 
                                lgs::print(to_hex_code(circuit_data[i][y_p * width + x_p]));
#endif
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

void lgs::CPUWorker::sim_step(const pack_uint_t* state_r, pack_uint_t* state_w)
{
       for(int y = 0; y < height; y++)
       {
               for(int x = 0; x < width; x++)
               {
                       pack_uint_t p = state_r[y*width + x];
                       pack_uint_t p0, p1, p2, p3;
                       p0 = (x+1 < width) ? state_r[y*width + x+1] : 0u;
                       p1 = (y-1 >= 0) ? state_r[(y-1) * width + x] : 0u;
                       p2 = (x-1 >= 0) ? state_r[y*width + x - 1] : 0u;
                       p3 = (y+1 < height) ? state_r[(y+1) * width + x] : 0u;

                       pack_uint_t v00, v01, v10, v11, v20, v21, v30, v31;
                       v00 = ((p >> 1)  & 0x7f7f7f7fu)  | ((p0 << 7)    & 0x80808080u);         // TODO: Rewrite constants in terms of defines.
                       v01 = ((p >> 2)  & 0x3f3f3f3fu)  | ((p0 << 6)    & 0xc0c0c0c0u);
                       v10 = ((p << 8)  & 0xffffff00u)  | ((p1 >> 24)   & 0x000000ffu);
                       v11 = ((p << 16) & 0xffff0000u)  | ((p1 >> 16)   & 0x0000ffffu);
                       v20 = ((p << 1)  & 0xfefefefeu)  | ((p2 >> 7)    & 0x01010101u);
                       v21 = ((p << 2)  & 0xfcfcfcfcu)  | ((p2 >> 6)    & 0x03030303u);
                       v30 = ((p >> 8)  & 0x00ffffffu)  | ((p3 << 24)   & 0xff000000u);
                       v31 = ((p >> 16) & 0x0000ffffu)  | ((p3 << 16)   & 0x000000ffu);

                       pack_uint_t crd[20];
                       for(int i = 0; i < 20; ++i)
                               crd[i] = circuit_data[i][y*width + x];

                       pack_uint_t u0[2];
                       pack_uint_t u1[2];
                       pack_uint_t u2[2];
                       pack_uint_t u3[2];
                       u0[1] = (crd[16] & v01) | (~crd[16] & v00);
                       u1[1] = (crd[17] & v11) | (~crd[17] & v10);
                       u2[1] = (crd[18] & v21) | (~crd[18] & v20);
                       u3[1] = (crd[19] & v31) | (~crd[19] & v30);
                       u0[0] = ~u0[1];
                       u1[0] = ~u1[1];
                       u2[0] = ~u2[1];
                       u3[0] = ~u3[1];

                       pack_uint_t ret = 0u;
                       for(int i3 = 0; i3 < 2; ++i3) for(int i2 = 0; i2 < 2; ++i2)
                               for(int i1 = 0; i1 < 2; ++i1) for(int i0 = 0; i0 < 2; ++i0)
                                       ret |= u0[i0] & u1[i1] & u2[i2] & u3[i3] & crd[i0 + 2*i1 + 4*i2 + 8*i3];

                       state_w[y*width + x] = ret;
#ifdef LGS_DEBUG
                       lgs::print(" ");
                       lgs::print(to_hex_code(ret));
#endif
                }
#ifdef LGS_DEBUG
               lgs::print("\n");
#endif               
       }
#ifdef LGS_DEBUG
       lgs::print("End of tick, press any key.");
       lgs::waitForKey();
#endif
}

bool lgs::CPUWorker::getStateAt(int x_u, int y_u)
{
        return (state_r[(y_u/LGS_PACK_HEIGHT) * width + x_u/LGS_PACK_WIDTH] >> ((y_u%LGS_PACK_HEIGHT)*LGS_PACK_WIDTH + (x_u%LGS_PACK_WIDTH)))
                % 2 == 1;
}

void lgs::CPUWorker::setStateAt(int x_u, int y_u, bool state)
{
        state_w[(y_u/LGS_PACK_HEIGHT) * width + x_u/LGS_PACK_WIDTH] = 
                (state_w[(y_u/LGS_PACK_HEIGHT) * width + x_u/LGS_PACK_WIDTH] & ~(2u ^ ((y_u%LGS_PACK_HEIGHT)*LGS_PACK_WIDTH + (x_u%LGS_PACK_WIDTH))))
                | (state ? 2u ^ ((y_u%LGS_PACK_HEIGHT)*LGS_PACK_WIDTH + (x_u%LGS_PACK_WIDTH)) : 0u);
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
                                std::get<2>(*i) = this->getStateAt(std::get<0>(*i), std::get<1>(*i));
                (*peri)->tick();
                for(PeripheralInterface::iterator i = out.begin(); i != out.end(); ++i)
                        if(std::get<0>(*i) >= 0 && std::get<0>(*i) < width && std::get<1>(*i) >= 0 && std::get<1>(*i) < height)
                                this->setStateAt(std::get<0>(*i), std::get<1>(*i), std::get<2>(*i));

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
        pack_uint_t* tmp = state_r;
        state_r = state_w;
        state_w = tmp;
}

bool* lgs::CPUWorker::getState()
{
        bool* ret = new bool[width * height * LGS_PACK_WIDTH * LGS_PACK_HEIGHT];
        for(int x_u = 0; x_u < width * LGS_PACK_WIDTH; ++x_u)
                for(int y_u = 0; y_u < height * LGS_PACK_HEIGHT; ++y_u)
                        ret[y_u * width * LGS_PACK_WIDTH + x_u] = this->getStateAt(x_u, y_u);
        return ret;
}
