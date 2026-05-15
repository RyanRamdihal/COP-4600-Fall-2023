# lkmasg2 - Linux Kernel Module Assignment 2

## Overview

This project re-implements the character device driver from the previous assignment as **two separate Linux kernel modules**:

- `lkmasg2writer` - an input (write-only) device module
- `lkmasg2reader` - an output (read-only) device module

The two modules communicate through a shared 1024-byte circular buffer. Access to that buffer is a critical section, so it is guarded with a kernel mutex from `linux/mutex.h`.

## Learning Objective

To gain a fuller understanding of critical sections in the Linux kernel by programming them.

## Development Environment

- Language: C
- Compiler: gcc
- Kernel: Linux 5.15 (the version provided in the course VM)

## File Layout

| File | Description |
| --- | --- |
| `lkmasg2writer.c` | Writer kernel module. Owns the shared buffer, mutex, and circular-buffer indices, exporting them with `EXPORT_SYMBOL`. Implements `write()`. |
| `lkmasg2reader.c` | Reader kernel module. Picks up the shared symbols via `extern`. Implements `read()`. |
| `test.c` | User-space test program that opens both devices and performs 1000 read/write cycles to exercise the circular buffer. |
| `Makefile` | Builds both kernel modules and the test program. |

## Shared Memory Between Modules

The writer module declares the shared state and exports it:

```c
struct mutex lkmasg2_mutex;
EXPORT_SYMBOL(lkmasg2_mutex);
volatile size_t availableBuffer = 0;
EXPORT_SYMBOL(availableBuffer);
volatile size_t start = 0;
EXPORT_SYMBOL(start);
volatile char deviceBuffer[BUFFER_SIZE];
EXPORT_SYMBOL(deviceBuffer);
```

The reader module imports them:

```c
extern struct mutex lkmasg2_mutex;
extern volatile size_t availableBuffer;
extern volatile size_t start;
extern volatile char deviceBuffer[BUFFER_SIZE];
```

Because the writer exports the symbols, it must be loaded **before** the reader.

## Critical Section Protection

The shared buffer is a critical section. Both `read()` and `write()` acquire `lkmasg2_mutex` with `mutex_lock()` on entry and release it with `mutex_unlock()` before returning, so only one of the two modules ever manipulates `deviceBuffer`, `start`, or `availableBuffer` at a time.

## Building

From the project directory inside the VM:

```bash
make
```

This invokes the in-tree kernel build system to produce `lkmasg2writer.ko` and `lkmasg2reader.ko`, and compiles `test.c` into `test`.

To clean up:

```bash
make clean
```

## Loading and Unloading the Modules

The writer must be inserted first (it exports the shared symbols the reader depends on):

```bash
sudo insmod lkmasg2writer.ko
sudo insmod lkmasg2reader.ko
```

To unload, remove in reverse order:

```bash
sudo rmmod lkmasg2reader
sudo rmmod lkmasg2writer
```

Confirm the devices were created:

```bash
ls -l /dev/lkmasg2writer /dev/lkmasg2reader
```

If the device nodes are not readable/writable by your user, give them permissions for testing:

```bash
sudo chmod 666 /dev/lkmasg2writer /dev/lkmasg2reader
```

## Running the Test Program

`test.c` takes the read device path and the write device path as arguments:

```bash
./test /dev/lkmasg2reader /dev/lkmasg2writer
```

It first runs an automated `basicReadWrite1k` test (1000 write/read cycles that exercise wraparound in the circular buffer), then enters an interactive mode where you can type a string, have it written to the writer device, and read it back through the reader device.

## Viewing Kernel Log Output

All status and error conditions are reported via `printk`. To watch them in real time:

```bash
sudo dmesg -w
```

## Expected `printk` Output

All conditions are written to the system log. The expected messages are:

| Condition | Output |
| --- | --- |
| Writer module install | `lkmasg2 Writer module successfully installed` |
| Entering `write()` | `lkmasg2 Writer - Entered write().` |
| Writer acquires the lock | `lkmasg2 Writer - Acquired the lock.` |
| Successful write to the buffer | `lkmasg2 Writer - Wrote %d bytes to the buffer.` |
| Attempt to write to a full buffer | `lkmasg2 Writer - Buffer is full, unable to write.` |
| Truncated write (insufficient space) | `lkmasg2 Writer - Buffer has %d bytes remaining, attempting to write %d, truncating input.` |
| Exiting `write()` | `lkmasg2 Writer - Exiting write() function` |
| Reader module install | `lkmasg2 Reader module successfully installed` |
| Entering `read()` | `lkmasg2 Reader - Entered read().` |
| Reader acquires the lock | `lkmasg2 Reader - Acquired the lock.` |
| Successful read from the buffer | `lkmasg2 Reader - Read %d bytes from the buffer.` |
| Attempt to read from an empty buffer | `lkmasg2 Reader - Buffer is empty, unable to read.` |
| Truncated read (requested more than available) | `lkmasg2 Reader - Buffer has %d bytes of content, requested %d.` |
| Exiting `read()` | `lkmasg2 Reader - Exiting read() function` |
| Any error state | `lkmasg2 Reader/Writer - Error encountered: %s` |

