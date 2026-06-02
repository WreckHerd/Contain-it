#include "cgroups.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

namespace containit {
    namespace cgroups {
        
        //must be executed before child process is entered
        void setup_cgroups(pid_t child_pid)
        {

            std::cout << "[INFO]    Setting up cgroups" << std::endl;


            //creating a new cgroup for container process
            const char* cgrp_dir = "/sys/fs/cgroup/container";
            mkdir(cgrp_dir, 0755);


            //restricting the memory for the container cgroup to 50MB
            std::ofstream mem_file(std::string(cgrp_dir) + "/memory.max"); 
            if(mem_file.is_open())
            {
                mem_file << config.memory_limit;
                mem_file.close();
                std::cout << "  --->    Memory limit set to: " << config.memory_limit << "bytes" << std::endl;
            }
            else
            {
                std::cerr << "[ERROR]   couldn't set memory limit" << strerror(errno) << std::endl;
            }

            // Block Swap Memory (Force OOM Killer)
            std::ofstream swap_file(std::string(cgrp_dir) + "/memory.swap.max");
            if (swap_file.is_open()) {
                swap_file << "0"; // 0 bytes of swap allowed
                swap_file.close();
                std::cout << "  --->    Swap memory restricted to 0 bytes.\n";
            } else {
                // Some systems don't have swap enabled at the kernel level, 
                // so this file might not exist. We can safely ignore it if so.
                std::cout << "[WARN] Swap limit file not found (system swap might be disabled).\n";
            }

            //restricting the max processes for container cgroup to max_proc
            std::ofstream maxpid_file(std::string(cgrp_dir) + "/pids.max");
            if(maxpid_file.is_open())
            {
                maxpid_file << config.process_limit;
                maxpid_file.close();
                std::cout << "  --->    Max processes inside container limited to: " << config.process_limit << std::endl;
            }
            else
            {
                std::cerr << "[ERROR]   couldn't set max process limit" << strerror(errno) << std::endl;
            }

            // Apply CPU Limits
            // We translate the user's core request (e.g., 0.5) into a CFS time budget.
            if (config.cpu_core_limit > 0.0) 
            {
                std::ofstream cpu_file(std::string(cgrp_dir) + "/cpu.max");
                if (cpu_file.is_open()) 
                {
                    int period = 100000; // The standard 100ms CFS window
                    int max_quota = static_cast<int>(config.cpu_core_limit * period);
                    
                    // Format is: "$MAX $PERIOD"
                    cpu_file << max_quota << " " << period;
                    cpu_file.close();
                    std::cout << "  --->    CPU limit set to " << config.cpu_core_limit << " cores." << std::endl;
                }
                else
                {
                    std::cerr << "[ERROR]   Warning: Could not set CPU limit." << std::endl;
                }
            }


            //putting the child process into the newly created container cgroup
            std::ofstream proc_file(std::string(cgrp_dir) + "/cgroup.procs");
            if(proc_file.is_open())
            {
                proc_file << child_pid;
                proc_file.close();
                std::cout << "[INFO]    putting child process into container cgroup" << std::endl;
            }
            else
            {
                std::cerr << "[ERROR]   couldn't put child process in container cgroup" << strerror(errno) << std::endl;
            }

        }

        void cleanup() {
            // Remove the directory to prevent cluttering the host's cgroup filesystem
            rmdir("/sys/fs/cgroup/container");
        }

    } 
} 