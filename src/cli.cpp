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
                        << "  -m, --memory <bytes>  Set memory limit (default: 50MB)\n"
                        << "  -r, --rootfs <path>   Path to the root filesystem\n"
                        << "  -c, --cmd <command>   Command to execute (default: /bin/sh)\n"
                        << "  -p, --procs <maxproc> Set process limit (default: 20)\n"
                        << "  -u, --cpu <num_cores> Set core limit (eg. 0.5, 1.0)\n"
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

            while((opt = getopt_long(argc, argv, "m:r:c:p:u:h", long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                    case 'm':
                        config.memory_limit = optarg;
                        break;
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