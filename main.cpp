#include <iostream>
#include <string.h> //for strerror()

#include<sched.h> //for clone()
#include<sys/wait.h> //for SIGCHLD and waitpid()

#include<sys/mount.h>//for mount and related flags

const int STACK_SIZE = 65536; //64kb

//the child starts execution from here
int child_main(void* arg)
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
    const char* rootfs_path = "/vagrant/Contain-it/alpine-rootfs";
    if(chroot(rootfs_path) != 0)
    {
        std::cerr << "Couldn't chroot into the image" << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "Root directory successfully changed to the image" << std::endl;

    //chroot does not change the current working directory
    chdir("/");
    
    //mounting the proc filesystem to the /proc directory of the image
    //ps now reads the active process from /proc directory
    //only processes under this namespace will be visible
    if(mount("proc","/proc", "proc", 0, NULL) != 0)
    {
        std::cerr << "Failed to mount proc" << strerror(errno) << std::endl;
    }
    std::cout << "proc mounted successfully" << std::endl;
    
    //launching a shell in the child container 
    char* cmd[] = {(char*)"/bin/ash", NULL};
    execvp(cmd[0], cmd);

    //execvp only returns if it fails
    std::cerr << "Error launching shell" << std::endl;
    return -1;
}


int main()
{

    std::cout << "Starting the container engine..." << std::endl;

    //the child process created from clone requires its own stack
    //allocating memory on heap for child's stack
    char* stack = new char[STACK_SIZE];

    //stack grows downwards; pointer to the top of the stack is required
    char* stack_top = stack + STACK_SIZE;

    //flag bit mask for modification of clone() behaviour
    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD;

    //creating the child process using clone()
    std::cout << "Creating an isolated child process using clone..." << std::endl;

    pid_t child_pid = clone(child_main, stack_top, flags, NULL);

    //clone() returns -1 on failure else child pid;
    if(child_pid == -1)
    {
        std::cerr << "Clone() failed" << strerror(errno) << std::endl;
    }

    //waiting for child to terminate
    waitpid(child_pid, NULL, 0);

    std::cout << "Container exited, cleaning up..." << std::endl;

    //freeing up heap allocated memory
    delete[] stack;

    return 0;
}