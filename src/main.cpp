#include "config.h"
#include "cli.h"
#include "cgroups.h"
#include "engine.h"
#include <iostream>
#include <string.h> //for strerror()

#include<sched.h> //for clone()
#include<sys/wait.h> //for SIGCHLD and waitpid()

const int STACK_SIZE = 65536; //64kb

ContainerConf config;


int main(int argc, char* argv[])
{
    containit::cli::parser(argc, argv);

    std::cout << "[INFO]    Starting the container engine" << std::endl;


    //pipe to manage relative execution of parent and child
    int pipefd[2];
    if(pipe(pipefd) != 0)
    {
        std::cerr << "[ERROR]   pipe couldn't be formed " << strerror(errno) << std::endl;       
    }
    config.piperd = pipefd[0];
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
    std::cout << "[INFO]    Creating an isolated child process using clone..." << std::endl;
    pid_t child_pid = clone(containit::engine::child_main, stack_top, flags, NULL);

    //clone() returns child's pid on success else -1;
    if(child_pid == -1)
    {
        std::cerr << "[ERROR]   Clone() failed" << strerror(errno) << std::endl;
    }

    //child is blocked at the read line

    //close the read end of the pipe in parent
    close(pipefd[0]);

    //setup cgroups before child executes shell
    containit::cgroups::setup_cgroups(child_pid);

    //send the "Go" signal to the child
    if(write(pipefd[1], "A" , 1) != 1)
    {
        std::cerr << "[ERROR]   Couldn't write into pipe" << strerror(errno) << std::endl;
    }
    close(pipefd[1]);

    //now we wait for child to finish
    waitpid(child_pid, NULL, 0);

    std::cout << "[DONE]    Container exited, cleaning up..." << std::endl;

    rmdir("/sys/fs/cgroup/container");
    delete[] stack;

    return 0;
}