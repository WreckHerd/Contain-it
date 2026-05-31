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

            std::cout << "setting up cgroups" << std::endl;


            //creating a new cgroup for container process
            const char* cgrp_dir = "/sys/fs/cgroup/container";
            mkdir(cgrp_dir, 0755);

            //restricting the memory for the container cgroup to 50MB
            std::ofstream mem_file(std::string(cgrp_dir) + "/memory.max"); 
            if(mem_file.is_open())
            {
                mem_file << config.memory_limit;
                mem_file.close();
            }
            else
            {
                std::cerr << "couldn't set memory limit" << strerror(errno) << std::endl;
            }

            //restricting the max processes for container cgroup to max_proc
            std::ofstream maxpid_file(std::string(cgrp_dir) + "/pids.max");
            if(maxpid_file.is_open())
            {
                maxpid_file << config.process_limit;
                maxpid_file.close();
            }
            else
            {
                std::cerr << "couldn't set max process limit" << strerror(errno) << std::endl;
            }

            //putting the child process into the newly created container cgroup
            std::ofstream proc_file(std::string(cgrp_dir) + "/cgroup.procs");
            if(proc_file.is_open())
            {
                proc_file << child_pid;
                proc_file.close();
            }
            else
            {
                std::cerr << "couldn't put child process in container cgroup" << strerror(errno) << std::endl;
            }

        }

        void cleanup() {
            // Remove the directory to prevent cluttering the host's cgroup filesystem
        }

    } 
} 