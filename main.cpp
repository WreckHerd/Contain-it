#include <iostream>
#include <sched.h>     // For clone()
#include <sys/wait.h>  // For waitpid()
#include <unistd.h>    // For sethostname(), execvp()
#include <string.h>    // For strerror()

// We need to allocate a block of memory for the child process's stack
const int STACK_SIZE = 65536; // 64 KB

// This is the function that the containerized process will run
int container_main(void* arg) {
    std::cout << "[Container] Inside the container!" << std::endl;

    // 1. Change the hostname (this only affects this isolated UTS namespace)
    std::string new_hostname = "nebula-container";
    if (sethostname(new_hostname.c_str(), new_hostname.length()) != 0) {
        std::cerr << "[Container] Error setting hostname: " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "[Container] Hostname changed to: " << new_hostname << std::endl;

    // 2. Launch a bash shell so we can interact with it
    char* cmd[] = {(char*)"/bin/bash", NULL};
    execvp(cmd[0], cmd);
    
    // execvp only returns if it fails
    std::cerr << "[Container] Error launching shell!" << std::endl;
    return -1;
}

int main() {
    std::cout << "[Host] Starting Contain-It Engine..." << std::endl;

    // 1. Allocate memory for the child's stack on the heap
    char* stack = new char[STACK_SIZE];
    
    // Note: Stacks grow downwards on x86, so we pass a pointer to the TOP of the stack
    char* stack_top = stack + STACK_SIZE;

    // 2. Define the isolation flags (CLONE_NEWUTS isolates the hostname)
    int flags = CLONE_NEWUTS | SIGCHLD;

    // 3. Clone the process
    std::cout << "[Host] Spawning isolated process using clone()..." << std::endl;
    pid_t child_pid = clone(container_main, stack_top, flags, NULL);

    if (child_pid == -1) {
        std::cerr << "[Host] clone() failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // 4. Wait for the container to exit (when you type 'exit' in the bash shell)
    waitpid(child_pid, NULL, 0);
    
    std::cout << "[Host] Container exited. Cleaning up..." << std::endl;
    delete[] stack;

    return 0;
}