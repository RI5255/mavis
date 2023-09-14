# Mavis

## What is Mavis?
Mavis is a microkernel with WASM "userland".

All programs are WASM binaries and they all run on ring0. There is no "virtual memory" or "system calls". The kernel provides an IPC (Inter-Process Communication) mechanism, and several WASM "servers" work together to implement the "Operation System". See the following image for more details. 

The purpose of this project is to demonstrate the idea of "using WASM runtime as a kernel" and to explore the possibilities of WASM runtime and microkernel.

<img src="https://github.com/RI5255/mavis/blob/images/overview.jpg" width="500px" alt="overview">

## Building & Running
Clone this repository and execute the "make" command. 
If you are lucky, the build will succeed ^^.

Execute the "make run" command to launch qemu.
If successful, you can see the shell as shown in the following image.

You can execute the "Hello World!" program by executing the "hello" command.
The "exit" command will cause the kernel panic because there are no more tasks to execute.

![shell-hello](https://github.com/RI5255/mavis/blob/images/shell-hello.PNG)

## About pull requests
This project is completely for my personal interest, so it is always buggy and destructive changes are made. 
Therefore, I basically do not accept pull requests. 
Of course, extensions based on this project and discussions are always welcome.
