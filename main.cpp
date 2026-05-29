#include <iostream>
#include <string.h> //for strerror()

#include<sched.h> //for clone()
#include<sys/wait.h> //for SIGCHLD and waitpid()

#include<sys/mount.h>//for mount and related flags

#include<sys/stat.h>//for mkdir
#include<fstream>//for ofstream

const int STACK_SIZE = 65536; //64kb
int piperd;

//must be executed before child process is entered
void setup_cgroups(pid_t child_pid)
{

    std::cout << "setting up cgroups" << std::endl;

    const char* cgrp_dir = "/sys/fs/cgroup/container";

    //creating a new cgroup for container process
    mkdir(cgrp_dir, 0755);

    //restricting the memory for the container cgroup to 50MB
    std::string mem_limit = "52428800";//50MB
    std::ofstream mem_file(std::string(cgrp_dir) + "/memory.max"); 
    if(mem_file.is_open())
    {
        mem_file << mem_limit;
        mem_file.close();
    }
    else
    {
        std::cerr << "couldn't set memory limit" << strerror(errno) << std::endl;
    }

    //restricting the max processes for container cgroup to max_proc
    std::string max_proc = "20";
    std::ofstream maxpid_file(std::string(cgrp_dir) + "/pids.max");
    if(maxpid_file.is_open())
    {
        maxpid_file << max_proc;
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

    // This provides /dev/zero, /dev/null, /dev/urandom, etc.
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0) {
        std::cerr << "[Container] Failed to mount /dev: " << strerror(errno) << std::endl;
        return -1;
    }

    //halting the process until the parent process writes into the pipe
    char ch;
    if(read(piperd, &ch, 1) != 1)
    {
        std::cerr << "couldn't read from pipe" << strerror(errno) << std::endl;
    }
    close(piperd);
    
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


    //pipe to manage relative execution of parent and child
    int pipefd[2];
    if(pipe(pipefd) != 0)
    {
        std::cerr << "pipe couldn't be formed " << strerror(errno) << std::endl;       
    }
    piperd = pipefd[0];
    //setup_cgroup function must be executed from the parent
    //and must be executed before shell is launched in the child


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

    //clone() returns child's pid on success else -1;
    if(child_pid == -1)
    {
        std::cerr << "Clone() failed" << strerror(errno) << std::endl;
    }

    //child is blocked at the read line

    //close the read end of the pipe in parent
    close(pipefd[0]);

    //setup cgroups before child executes shell
    setup_cgroups(child_pid);

    //send the "Go" signal to the child
    if(write(pipefd[1], "A" , 1) != 1)
    {
        std::cerr << "Couldn't write into pipe" << strerror(errno) << std::endl;
    }
    close(pipefd[1]);

    //now we wait for child to finish
    waitpid(child_pid, NULL, 0);

    std::cout << "Container exited, cleaning up..." << std::endl;

    rmdir("/sys/fs/cgroup/container");
    delete[] stack;

    return 0;
}