# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # Use the official Ubuntu 22.04 LTS image
  config.vm.box = "ubuntu/jammy64"

  # Sync your host project directory to /vagrant inside the VM
  # This allows you to edit code on your host, but compile/run inside the VM
  config.vm.synced_folder ".", "/vagrant"

  # Optimize the VirtualBox provider settings for C++ development
  config.vm.provider "virtualbox" do |vb|
    # Display the VirtualBox GUI when booting the machine (optional, usually false for servers)
    vb.gui = false
    
    # Name the VM so it looks clean in the VirtualBox manager
    vb.name = "contain-it-sandbox"

    # Allocate 2GB of RAM to prevent the g++ compiler from running out of memory
    vb.memory = "2048"

    # Allocate 2 CPU cores for faster compilation
    vb.cpus = 2
  end

  # Optional: Automatically install build-essential when the VM boots for the first time
  # This saves the user from having to run 'sudo apt install build-essential' manually
  config.vm.provision "shell", inline: <<-SHELL
    echo "Updating apt repositories and installing C++ build tools..."
    apt-get update
    apt-get install -y build-essential
  SHELL
end