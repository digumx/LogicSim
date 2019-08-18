/*
 * Main file for the LogicSim logic circuit simulator.
 *
 * @Author: Diganta Muhopadhyay
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gif.h>
#include <json.hpp>

#include <cpuworker.hpp>
#include <peripherals.hpp>
#include <ncursesio.hpp>

#include <logicsim.hpp>

namespace lgs
{

        void printUsage()
        {
                std::cout << "Logic Sim usage: " << std::endl << std::endl;
                std::cout << "./logicsim [--<option name> <option value>] <circuit path>" << std::endl << std::endl;
                std::cout << "Where the options are" << std::endl;
                std::cout << "\t-l or --simulation-length\tThe arguement to this option is the number of ticks for which to run the simulation. " 
                        << "Negative values indicate simulation is to be run indefinitely. Default is " << LGS_DEFAULT_TIME_LENGTH << std::endl;
                std::cout << "\t-s or --print-stride\tThe arguement to this option is the number of ticks after which to output the state. "
                        << "The states are saved as a gif animation with the filename <circuit path>.out.gif. If 0 or a negative value is passed, " 
                        << "the gif contains only one frame, the final state. Else, each frame of the gif corresponds to the state obtained after "
                        << "<print steps> ticks from the last frame's state. Default is " << LGS_DEFAULT_PRINT_STEPS << std::endl;
                std::cout << "\t-t or --frametime\tThe arguement to this option is the time in milliseconds between each frame in the gif outputted. Default is " 
                        << LGS_DEFAULT_FRAMETIME << std::endl;
                std::cout << "\t-c or --output-scale\tThe arguement to this option is the number of times to scale pixel sizes in the output gif. "
                        << "Very useful for small circuits. Default is " << LGS_DEFAULT_SCALE_FACTOR << std::endl;
                std::cout << "\t<circuit path>\tThis is the path to the json circuit file." << std::endl;
                std::cerr << std::endl << "Bad command line arguements." << std::endl;
                std::exit(EXIT_FAILURE);
        }

        void stateToFrame(uint8_t*& frame, const bool* state, int w, int h, int s)
        {
               for(int y = 0; y < h; y++)
                      for(int x = 0; x < w; x++)
                              for(int i = 0; i < s; i++)
                                      for(int j = 0; j < s; j++)
                                      {
                                              int k = (y*s + j)*w*s + x*s + i;
                                              bool s = state[y*w + x];
                                              frame[k*4] = s ? LGS_GIF_COLOR_1_R : LGS_GIF_COLOR_0_R;
                                              frame[k*4+1] = s ? LGS_GIF_COLOR_1_G : LGS_GIF_COLOR_0_G;
                                              frame[k*4+2] = s ? LGS_GIF_COLOR_1_B : LGS_GIF_COLOR_0_B;
                                              frame[k*4+3] = s ? LGS_GIF_COLOR_1_A : LGS_GIF_COLOR_0_A;
                                      }
        }

        void parseArgs(int argc, char** argv, int& simLength, int& printStep, int& frameTime, int& scaleFactor, char*& imagePath)
        {
                if(argc < 2) printUsage();
                imagePath = argv[argc-1];
                for(int i = 1; i < argc-1; i++)
                {
                        if(argv[i] == std::string("-l") || argv[i] == std::string("--simulation-length")) 
                                simLength = std::atoi(argv[++i]);
                        else if(argv[i] == std::string("-s") || argv[i] == std::string("--print-stride")) 
                                printStep = std::atoi(argv[++i]);
                        else if(argv[i] == std::string("-t") || argv[i] == std::string("--frametime")) 
                                frameTime = std::atoi(argv[++i]);
                        else if(argv[i] == std::string("-c") || argv[i] == std::string("--output-scale"))
                                scaleFactor = std::atoi(argv[++i]);
                        else printUsage();
                }
        }
}

using namespace lgs;

int main(int argc, char** argv)
{
        // Platform checks
        assert(CHAR_BIT == 8);
        assert(sizeof(int) >= 3);

        // Read arguements
        int sim_length = LGS_DEFAULT_TIME_LENGTH;
        int print_step = LGS_DEFAULT_PRINT_STEPS;
        int frametime = LGS_DEFAULT_FRAMETIME;
        int scale_factor = LGS_DEFAULT_SCALE_FACTOR;
        char* json_path;
        parseArgs(argc, argv, sim_length, print_step, frametime, scale_factor, json_path);
        std::string json_path_str(json_path);

        // Enter NCURSES mode
        lgs::initializeNcursesIO();


        lgs::print("Loading circuit json\n");
        nlohmann::json circuit_json;
        std::ifstream circuit_json_file(json_path);
        if(!circuit_json_file.is_open())
        {
                lgs::print("Failed to open circuit json file.\n");
                lgs::exitNcursesMode(true);
        }
        circuit_json_file >> circuit_json;

        std::string image_path_str = circuit_json["Image path"].get<std::string>();
        
        if(image_path_str[0] != '/')               // If image path is not absolute, convert the relative to JSON path to relative to cwd
        {
                std::size_t i = json_path_str.find_last_of('/');
                image_path_str = json_path_str.substr(0, i) + std::string("/") + image_path_str;
        }
        const char* image_path = image_path_str.c_str();
        nlohmann::json peripherals_json = circuit_json["Peripherals"];
        circuit_json_file.close();

        lgs::print("Loading image\n");
        int circuit_width, circuit_height, n_circuit_image_channels;
        unsigned char* circuit_data_rgb = stbi_load(image_path, &circuit_width, &circuit_height, &n_circuit_image_channels, 3);
        if(circuit_data_rgb == NULL)
        {
                lgs::print("Failed to load image file ");
                lgs::print(image_path);
                lgs::print("\n");
                lgs::exitNcursesMode(true);
        }
        if(n_circuit_image_channels != 3)
                lgs::print("WARNING: Possible bad image file format, image must have 3 channels.\n");
        lgs::print("Loaded image file, parsing data\n");
        unsigned int* circuit_data = new unsigned int[circuit_width * circuit_height * 3];
        for(int i = 0; i < circuit_width * circuit_height; i++)
                circuit_data[i] = ((circuit_data_rgb[i*3]&15)<<16) + (circuit_data_rgb[i*3+1]<<8) + circuit_data_rgb[i*3+2];
        stbi_image_free(circuit_data_rgb);
        lgs::print("Loaded circuit\n");

        std::vector<Peripheral*> peripherals;
        peripherals.reserve(peripherals_json.size());
        for(nlohmann::json::iterator i = peripherals_json.begin(); i != peripherals_json.end(); ++i)
                peripherals.push_back(lgs::peripheralFromJson(*i));
        lgs::print("Loaded peripherals\n");

        GifWriter out_writer;
        std::string out_path = std::string(json_path) + std::string(".out.gif");
        GifBegin(&out_writer, out_path.c_str(), circuit_width * scale_factor, circuit_height * scale_factor, frametime);

        lgs::print("Press any key to start simulation.\n");
        lgs::waitForKey();
        lgs::clearScreen();

#ifdef LGS_PROFILE
        // Initialize for profiling
        PrintSection* prof_sec = new PrintSection();
        std::chrono::steady_clock::time_point tp1, tp2;
        int n_samps = 0;
        std::chrono::microseconds tick_time, sim_step_time;
#endif

        // Start simulation
        CPUWorker worker(circuit_data, circuit_width, circuit_height, peripherals);
        int n_ticks_out = 0;
        uint8_t* frame = new uint8_t[circuit_width * circuit_height * 4 * scale_factor * scale_factor];
        for(int i = 0; i != sim_length; i++)
        {
#ifdef LGS_PROFILE
                tp1 = std::chrono::steady_clock::now();
#endif
                worker.tickSimulation();
#ifdef LGS_PROFILE
                tp2 = std::chrono::steady_clock::now();
                tick_time += std::chrono::duration_cast<std::chrono::microseconds>(tp2-tp1);
#endif
                n_ticks_out++;
                if(n_ticks_out == print_step)
                {
                        n_ticks_out = 0;
                        const bool* state = worker.getState();
                        lgs::stateToFrame(frame, state, circuit_width, circuit_height, scale_factor);
                        GifWriteFrame(&out_writer, frame, circuit_width * scale_factor, circuit_height * scale_factor, frametime);
                }
#ifdef LGS_PROFILE
                tp2 = std::chrono::steady_clock::now();
                sim_step_time += std::chrono::duration_cast<std::chrono::microseconds>(tp2-tp1);
                n_samps++;
                if(n_samps >= LGS_PROFILE_N_SAMPLES)
                {
                        float sim_step_avg = sim_step_time.count() / (n_samps * 1.0f);
                        float tick_avg = tick_time.count() / (n_samps * 1.0f);
                        std::stringstream str;
                        str << "Main profiler.\n";
                        str << "Simulation step time: " << sim_step_avg << " microseconds, Tick time: " << tick_avg << " microseconds, ";
                        str << "averaged over " << n_samps;
                        prof_sec->setText(str.str());
                        n_samps = 0;
                        sim_step_time = std::chrono::microseconds(0);
                        tick_time = std::chrono::microseconds(0);
                }
#endif
        }
        const bool* state = worker.getState();
        lgs::stateToFrame(frame, state, circuit_width, circuit_height, scale_factor);
        GifWriteFrame(&out_writer, frame, circuit_width * scale_factor, circuit_height * scale_factor, frametime);
        delete[] frame;
        lgs::print("Finished simulation\n");

        GifEnd(&out_writer);
        lgs::exitNcursesMode(false);

        return EXIT_SUCCESS;
}

