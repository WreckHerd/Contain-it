# Contain-It: A Custom Linux Container Engine

Contain-It is a lightweight, CLI-driven container runtime written entirely in C++. It demonstrates the fundamental operating system primitives that power modern tools like Docker and Podman, directly interfacing with the Linux kernel to isolate processes, virtualize filesystems, and throttle hardware resources.

## Core Features
* **Namespace Isolation:** Full isolation of Hostname (UTS), Process IDs (PID), and Mount points (NS) via the `clone()` syscall.
* **Filesystem Jailing:** Secure filesystem swapping using `chroot` and private mount propagation to prevent host-system pollution.
* **Cgroups v2 Resource Limiting:** Dynamic restriction of container memory, cpu and processes via the `/sys/fs/cgroup` virtual filesystem.
* **IPC Synchronization:** Eliminates parent/child execution race conditions using anonymous kernel pipes.

## Architecture Details
When `./contain-it run` is executed, the engine performs the following lifecycle:
1. Parses CLI flags using `getopt_long` to determine limits and targets.
2. Creates an anonymous IPC pipe to act as a synchronization barrier.
3. Spawns a child process using `clone()` with isolation flags.
4. The parent process writes the child's true PID to `/sys/fs/cgroup/contain-it/cgroup.procs` and sets memory, cpu and max process limits
5. The parent writes a byte to the IPC pipe, waking the child.
6. The child severs mount propagation (`MS_PRIVATE`), `chroot`s into the downloaded Alpine rootfs, and mounts its own virtual `/proc` and `/dev`.
7. The child calls `execvp()` to replace its process image with the requested shell.

## Testing
Because this engine manipulates core Linux kernel state and requires `root` privileges, __it is recommended to test it inside a Virtual Machine__. If you don't have a Virtual Machine set up a pre-configured Vagrantfile is provided that spins up an isolated Ubuntu 22.04 LTS sandbox.

### Running the container
The project contains a make file and can be compiled by cloning the directory and running `make`.
This project also includes an alpine-rootfs directory containing the os image for the container to boot into.
Execute the engine with `sudo` to grant the necessary kernel capabilities
eg. 
```
sudo ./contain-it run --memory 52428800 --cpu 1.0 --procs 20
```
Available CLI flags:
```
Usage: contain-it run [OPTIONS]
Options:
        -m, --memory <bytes>  Set memory limit (default: 50MB)
        -r, --rootfs <path>   Path to the root filesystem
        -c, --cmd <command>   Command to execute (default: /bin/sh)
        -p, --procs <maxproc> Set process limit (default: 20)
        -u, --cpu <num_cores> Set core limit (eg. 0.5, 1.0)
        -h, --help            Show this help message
```

Once inside the container shell, you can run the following to prove the isolation is working:

  `ps aux` - Should only display your isolated container processes, not the host's systemd processes.

  `hostname` - Will display the isolated UTS namespace hostname.



## Optional

###  Instructions for installing a VM
You will need to install Vagrant and VirtualBox

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

