# RISC-V-Operating-System

## Description

**Energy Toolbase March 20th, 2025 Hackathon**

This project involves building an operating system kernel in C for the RISC-V architecture. Instead of physical hardware, we'll use the QEMU hardware emulator to simulate the RISC-V environment. Please read [this](https://operating-system-in-1000-lines.vercel.app/en/) short book on writing an operating system before the hackathon if you can—but no worries if you don't finish it. Since this is purely a learning experience (and it's gonna be pretty chill), we can use the book as our guide if we get stuck.

## Project Goals

At a minimum let’s try and implement the features that the book walks through:

- **Multitasking**: process scheduling (we can implement our own algorithm for this, round robin?)
- **Exceptions**: events requiring OS intervention
- **Paging**: isolated memory space for each process
- **System Calls:** how processes interact with the OS kernel
- **Device Drivers**: hardware device interactions (emulated in our case)
- **File System:** built using the hardware drivers, used to manage files on disk
- **Command-line Shell**: user interface that we can use

If we can get this done, we can try and extend it however we like.