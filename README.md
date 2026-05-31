# Contain-It: A Custom Linux Container Engine

Contain-It is a lightweight, CLI-driven container runtime written entirely in C++. It demonstrates the fundamental operating system primitives that power modern tools like Docker and Podman, directly interfacing with the Linux kernel to isolate processes, virtualize filesystems, and throttle hardware resources.

## Core Features
* **Namespace Isolation:** Full isolation of Hostname (UTS), Process IDs (PID), and Mount points (NS) via the `clone()` syscall.
* **Filesystem Jailing:** Secure filesystem swapping using `chroot` and private mount propagation to prevent host-system pollution.
* **Cgroups v2 Resource Limiting:** Dynamic restriction of container memory usage via the `/sys/fs/cgroup` virtual filesystem.
* **IPC Synchronization:** Eliminates parent/child execution race conditions using anonymous kernel pipes.

## Architecture Details
When `./contain-it run` is executed, the engine performs the following lifecycle:
1. Parses CLI flags using `getopt_long` to determine limits and targets.
2. Creates an anonymous IPC pipe to act as a synchronization barrier.
3. Spawns a child process using `clone()` with isolation flags.
4. The parent process writes the child's true PID to `/sys/fs/cgroup/contain-it/cgroup.procs` and sets resource limits.
5. The parent writes a byte to the IPC pipe, waking the child.
6. The child severs mount propagation (`MS_PRIVATE`), `chroot`s into the downloaded Alpine rootfs, and mounts its own virtual `/proc` and `/dev`.
7. The child calls `execvp()` to replace its process image with the requested shell.

## Testing
Because this engine manipulates core Linux kernel state and requires `root` privileges, it is recommended to test it inside a Virtual Machine.

The project contains a Makefile and can be complied using `make`
