#include "cli.h"
#include "config.h"
#include <iostream>
#include <getopt.h>

namespace containit{
    namespace cli{

        void print_usage()
        {
            std::cout << "Usage: contain-it run [OPTIONS]\n"
                        << "Options:\n"
                        << "  -n, --hostname <name> Set container hostname (default: Container)\n"
                        << "  -m, --memory <megabytes>  Set memory limit (default: 50MB)\n"
                        << "  -r, --rootfs <path>   Path to the root filesystem\n"
                        << "  -c, --cmd <command>   Command to execute (default: /bin/sh)\n"
                        << "  -p, --procs <num_proc> Set process limit (default: 20)\n"
                        << "  -u, --cpu <num_cores> Set core limit eg. 0.5, 1.0 (default: nolimit)\n"
                        << "  -h, --help            Show this help message\n";
            exit(1);
        }

        void parser(int argc, char* argv[]) 
        {
            if (argc < 2)
            {
                print_usage();
            }

            if(std::string(argv[1]) != "run")
            {
                std::cerr << "Unknown command " << argv[1] << std::endl;
                print_usage();
            }

            //struct option is the data type 
            //long_options is the indetifier, [] specifies array
            struct option long_options[] = {
                {"hostname", required_argument, 0, 'n'},
                {"memory", required_argument, 0, 'm'},
                {"rootfs", required_argument, 0, 'r'},
                {"cmd", required_argument, 0, 'c'},
                {"procs", required_argument, 0, 'p'},
                {"cpu", required_argument, 0, 'u'},
                {"help", no_argument, 0, 'h'},
                {0, 0, 0, 0}
            };

            int opt;
            int option_index = 0;

            //parsing starts from argv[2]; skipping ""./contain-it" and "run".
            optind = 2;

            while((opt = getopt_long(argc, argv, "n:m:r:c:p:u:h", long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                    case 'n':
                        config.hostname = optarg;
                        break;
                    case 'm': { 
                            try {
                                // 1. Convert the user's string input (e.g., "50") to a long integer
                                long long input_mb = std::stoll(optarg);
                                
                                // 2. Multiply by 1024 * 1024 to get raw bytes
                                long long bytes = input_mb * 1024 * 1024;
                                
                                // 3. Save it to your config as a string so cgroups.cpp can write it directly
                                config.memory_limit = std::to_string(bytes);
                            } 
                            catch (const std::invalid_argument& e) {
                                std::cerr << "[ERROR] Invalid memory format. Please provide an integer (e.g., 50 for 50MB).\n";
                                exit(EXIT_FAILURE);
                            }
                            catch (const std::out_of_range& e) {
                                std::cerr << "[ERROR] Memory value is too large.\n";
                                exit(EXIT_FAILURE);
                            }
                            break;
                        }
                    case 'r':
                        config.rootfs_path = optarg;
                        break;
                    case 'c':
                        config.command = optarg;
                        break;
                    case 'p':
                        config.process_limit = optarg;
                        break;
                    case 'u':
                        try{
                            config.cpu_core_limit = std::stof(optarg);
                        } catch (const std::exception& e) {
                            std::cerr << "Invalid CPU limit provided.\n";
                            exit(1);
                        }
                        break;
                    case 'h':
                    default:
                        print_usage();
                        break;
                }
            }
        }

    }
}