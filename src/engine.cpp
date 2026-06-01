#include "engine.h"
#include "config.h"
#include <iostream>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>//for mkdir
#include <string.h>

namespace containit {
    namespace engine {
        
        //the child starts execution from here
        int child_main(void*)
        {    

            std::cout << "Inside the child process" << std::endl;   


            //changing the hostname, this process has an isolated uts namespace
            //hence the change only affects this process
            std::string n_hostname = "childContainer";
            if(sethostname(n_hostname.c_str(), n_hostname.length()) != 0)
            {
                std::cerr << "Host name couldn't be changed " << strerror(errno) << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Host name changed successfully. The new hostname is: ";

                char buf[256];

                //gethostname fetches the hostname of the current uts namespace
                if(gethostname(buf, sizeof(buf)) != 0)
                {
                    std::cerr << "Couldn't get hostname " << strerror(errno) << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << buf << std::endl;
                }
            }

            //making this mount namespace private 
            //ie. mounts/unmounts in host/child don't affect one another
            mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL);

            //chrooting into the alpine image 
            if(chroot(config.rootfs_path.c_str()) != 0)
            {
                std::cerr << "Couldn't chroot into the image" << strerror(errno) << std::endl;
                return -1;
            }
            std::cout << "Root directory successfully changed to the image" << std::endl;

            //chroot does not change the current working directory
            chdir("/");

            // CREATE MOUNT POINTS IF MISSING
            // We use 0755 for standard rwxr-xr-x directory permissions.
            // We don't check for errors here because if mkdir fails with EEXIST 
            // (folder already exists), that is perfectly fine!
            mkdir("/proc", 0755);
            mkdir("/dev", 0755);
            
            //mounting the proc filesystem to the /proc directory of the image
            //ps now reads the active process from /proc directory
            //only processes under this namespace will be visible
            if(mount("proc","/proc", "proc", 0, NULL) != 0)
            {
                std::cerr << "Failed to mount proc " << strerror(errno) << std::endl;
                return -1;
            }
            std::cout << "proc mounted successfully" << std::endl;

            // This provides /dev/zero, /dev/null, /dev/urandom, etc.
            if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0) {
                std::cerr << "[Container] Failed to mount /dev: " << strerror(errno) << std::endl;
                return -1;
            }

            //halting the process until the parent process writes into the pipe
            char ch;
            if(read(config.piperd, &ch, 1) != 1)
            {
                std::cerr << "couldn't read from pipe" << strerror(errno) << std::endl;
            }
            close(config.piperd);
            
            //launching a shell in the child container 
            char* cmd[] = {(char*)config.command.c_str(), NULL};
            execvp(cmd[0], cmd);

            //execvp only returns if it fails
            std::cerr << "Error launching shell" << std::endl;
            return -1;
        }

    } 
} 