# Contain-It: A Custom Linux Container Engine

Contain-It is a lightweight, CLI-driven container runtime written entirely in C++. It demonstrates the fundamental operating system primitives that power modern tools like Docker and Podman, directly interfacing with the Linux kernel to isolate processes, virtualize filesystems, and throttle hardware resources.

## Quickstart

__Disclaimer: The engine requires root privileges and is recommended to test it inside a Virtual Machine__

1. Clone the git repo and cd into it
```
git clone https://github.com/WreckHerd/Contain-it
cd Contain-it
```
2. Setup the alpine-rootfs and initialize test programs into it
```
make setup-rootfs   
```
3. Compile the source code
```
make
```
4. Checkout container flags and verify it works as expected
```
sudo ./contain-it run -h
sudo ./contain-it run "./verify.sh"
```
5. Create a container and drop into a shell inside it
```
sudo ./contain-it run --cpu "1.5"
```

## Demo video
[Watch the YouTube Video](https://youtu.be/78qh5jSexVE)


## Core Features
* **Namespace Isolation:** Full isolation of Hostname (UTS), Process IDs (PID), and Mount points (NS) via the `clone()` syscall.
* **Filesystem Jailing:** Secure filesystem swapping using `chroot` and private mount propagation to prevent host-system pollution.
* **Cgroups v2 Resource Limiting:** Dynamic restriction of container memory, cpu and processes via the `/sys/fs/cgroup` virtual filesystem.
* **IPC Synchronization:** Eliminates parent/child execution race conditions using anonymous kernel pipes.

## Architectural flow
When `./contain-it run` is executed, the engine performs the following lifecycle:
1. Parses CLI flags using `getopt_long` to determine limits and targets.
2. Creates an anonymous IPC pipe to act as a synchronization barrier.
3. Spawns a child process using `clone()` with isolation flags.
4. The parent process writes the child's true PID to `/sys/fs/cgroup/contain-it/cgroup.procs` and sets memory, cpu and max process limits
5. The parent writes a byte to the IPC pipe, waking the child.
6. The child severs mount propagation (`MS_PRIVATE`), `chroot`s into the downloaded Alpine rootfs, and mounts its own virtual `/proc` and `/dev`.
7. The child calls `execvp()` to replace its process image with the requested shell.

## Test guidelines

- It is recommended to run this inside a VM. In case you don't have one and want to set one up, a Vagrantfile is provided that spins up an isolated Ubuntu 22.04 LTS sandbox. if you want to take this path you can refer the Vagrant environment setup section at the end of this file.

- Makefile includes instructions to set up a root file system to serve as the base for the container. These can be executed using
```
make setup-rootfs
```
additionaly this will add 2 executables to the root file system `verify.sh`(to test pid isolation, /proc mounting and memory limit enforcement) and `mem-limit-visual`(to give a visual test to see memory limit enforcement). 

### Various features and related notes
The flags usable with the container along with default value are given below:
```
Usage: contain-it run [OPTIONS]
Options:
-n, --hostname <name>         Set container hostname              (default: Container)
-m, --memory <megabytes>      Set memory limit                    (default: 50MB)
-r, --rootfs <path>           Path to the root filesystem
-c, --cmd <command>           Command to execute                  (default: /bin/sh)
-p, --procs <num_proc>        Set process limit                   (default: 20)
-u, --cpu <num_cores>         Set core limit eg. 0.5, 1.0         (default: nolimit)
-h, --help                    Show this help message
```
- --rootfs: The container has been primarily tested on the alpine-rootfs image.
- --cmd: The container has primarily been tested while the cmd executed being a shell. An interrupt handler is also coded to gracefully exit while executing commands like `sudo ./contain-it --cmd ping 8.8.8.8`.  


## Limitations
- No User Namespace isolation: CLONE_NEWUSER not yet implemented, containers root is same as root on hostmachine.
- NOT OCI Compliant: Engine relies on downloading raw tarballs and extracting them. You cannot run contain-it pull ubuntu:latest or load standard Dockerfiles.
- No Network virtualization: CLONE_NEWNET not implemented.



## Vagrant environment setup

###  Instructions for installing a VM
You will need to install Vagrant and VirtualBox (the files are quite large, may take some time)

eg. in ubuntu/debian
```
sudo apt update
sudo apt install virtualbox vagrant
```

Once the dependencies are installed, navigate to the root directory of this cloned repository and bring the virtual machine online. Vagrant will automatically read the Vagrantfile, download the Ubuntu image, and configure the shared /vagrant directory.
```
#Download the OS image and boot the vm then drop into a secure shell inside vm
vagrant up
vagrant ssh
```
Once you run vagrant ssh, your terminal prompt will change to vagrant@ubuntu-jammy.

The directory you cloned on your host machine is automatically synced to the /vagrant folder inside the virtual machine. You do not need to push/pull code to test it. You can use your favorite code editor on your host machine, and simply compile and run the engine inside the VM terminal:
```
cd /vagrant
make
sudo ./contain-it run ...
```

