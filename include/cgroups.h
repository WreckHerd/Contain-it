#pragma once
#include <sys/types.h>

namespace containit {
    namespace cgroups {

        void setup_cgroups(pid_t child_pid);

        void cleanup();
    }
}