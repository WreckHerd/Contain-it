#pragma once
#include <string>
#include <filesystem>

//struct to hold all the user defined parameters
struct ContainerConf 
{
    std::string hostname = "Container";
    std::string memory_limit = "52428800";//50MB
    std::string rootfs_path = (std::filesystem::current_path() / "alpine-rootfs").string();
    std::string command =  "/bin/sh";
    std::string process_limit = "20"; 
    int piperd;
    float cpu_core_limit = 0.0;
};

extern ContainerConf config;