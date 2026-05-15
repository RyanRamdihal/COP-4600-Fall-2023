# LKMASG1 - Linux Kernel Module Assignment

## Overview

This project implements a character-mode Linux device driver as a loadable kernel module (LKM). The driver registers a character device named `lkmasg1` that maintains an internal 1024-byte buffer. Data written to the device is stored in the buffer and can be read back out in FIFO (first-in, first-out) order. As bytes are read, they are removed from the buffer, freeing space for additional writes.

The module was developed for Linux kernel version 5.15 (as adapted by John Aedo) and built using `gcc` against the running kernel's build headers.

## Course Information

- Class: COP4600 - Operating Systems
- Assignment: Kernel Module Assignment (LKM)
- Language: C
- Platform: Linux 5.15 (course VM)

## Files

| File | Description |
|------|-------------|
| `lkmasg1.c` | Source code for the kernel module |
| `test.c` | User-space test utility that exercises the driver |
| `Makefile` | Build rules for both the kernel module and the test program |
| `README.md` | This file |

## Features

The driver fulfills the following requirements:

### Lifecycle

- Registers itself with the kernel and dynamically obtains a major device number via `register_chrdev`.
- Creates a device class and device node (`/dev/lkmasg1`) using `class_create` and `device_create`.
- Cleanly de-initializes by destroying the device, unregistering the class, and freeing the major number on `rmmod`.
- Logs lifecycle events (install, remove, open, close, read, write) using `printk`, viewable through `dmesg`.

### Data Handling

- Maintains a constant-size internal buffer of `BUFFER_SIZE` (1024 bytes), implemented as a circular buffer (`start` index plus `availableBuffer` count).
- `write()` appends user data to the buffer using `copy_from_user`.
- `read()` returns data from the buffer in FIFO order using `copy_to_user`, then advances the `start` index so consumed bytes are removed from the buffer.
- A trailing null terminator (`\0`) is appended to the data delivered to user space to form a proper string (per Hint 2 in the assignment).

### Error / Boundary Handling

- On `write()`, if the request is larger than the available space in the buffer, the driver writes only as many bytes as will fit and returns that count. The remaining user bytes are not stored.
- On `read()`, if the request is larger than the number of bytes currently held in the buffer, the driver returns only what is available (including zero bytes when the buffer is empty).
- The `open()` function uses `cmpxchg` to ensure only one process at a time can hold the device open; subsequent opens receive `-EBUSY`.

### Implemented File Operations

| Function | Purpose |
|----------|---------|
| `open()` | Acquires exclusive access to the device. |
| `close()` (release) | Releases the device for use by other processes. |
| `read()` | Copies up to `len` bytes of buffered data out in FIFO order and removes them from the buffer. |
| `write()` | Copies up to `len` bytes from user space into the buffer (or less if the buffer is near-full). |

## Building

Run `make` from the project directory. The Makefile builds both the kernel module (`lkmasg1.ko`) and the user-space test utility (`test`):

```
make
```

To remove all build artifacts (the `.ko`, intermediate files, and the test binary), run:

```
make clean
```

Note: `make clean` does **not** unload the module if it is currently inserted into the kernel; use `rmmod` for that.

## Installing the Module

All driver operations require root privileges. Use `sudo` for every step.

Insert the module:

```
sudo insmod lkmasg1.ko
```

Verify it loaded and check the assigned major number:

```
sudo dmesg | tail
lsmod | grep lkmasg1
```

A device node will be created automatically at `/dev/lkmasg1`.

## Testing

The included `test` program writes a user-provided string to the driver and then reads it back. Run it as root and pass the device path as an argument:

```
sudo ./test /dev/lkmasg1
```

`test.c` performs writes and reads of 256 bytes at a time. Per the assignment hint, you may modify `test.c` to perform a random sequence of reads and writes with random lengths to exercise edge cases (buffer overrun, empty reads, partial writes, wrap-around in the circular buffer).

You can also test interactively from the shell:

```
# Write to the device
echo "Hello, kernel!" | sudo tee /dev/lkmasg1

# Read from the device
sudo cat /dev/lkmasg1
```

Watch kernel log messages while testing:

```
sudo dmesg -w
```

## Unloading the Module

```
sudo rmmod lkmasg1
```

Confirm removal:

```
sudo dmesg | tail
```

You should see the `lkmasg1: removing module.` and `Goodbye from the LKM!` log messages.

## Implementation Notes

- The internal buffer is treated as a circular queue. `start` tracks the index of the oldest unread byte, and `availableBuffer` tracks how many bytes are currently stored. Writes append at `(start + availableBuffer) % BUFFER_SIZE`; reads consume from `start` and advance it.
- `kmalloc` with `GFP_KERNEL` is used inside `read()` and `write()` to allocate temporary scratch buffers, which keeps the kernel stack frame small.
- `copy_to_user` and `copy_from_user` are used for all transfers across the kernel/user-space boundary (per Hint 3) since the supplied user pointer cannot be dereferenced directly inside the kernel.
- A null terminator is added to the byte stream returned by `read()` so user-space callers can treat the result as a C string.

## Troubleshooting

- **`insmod` fails with "Operation not permitted"** - You must run with `sudo`.
- **`/dev/lkmasg1` does not appear** - Check `dmesg` for class/device creation errors and confirm the module is loaded with `lsmod`.
- **Module is "in use" when removing** - Make sure no process still has the device open (no leftover `cat`, `echo`, or `test` process holding it).
- **Build errors about missing headers** - Install the kernel headers package matching your running kernel (`uname -r`).

## Author

Implementation by the student for COP4600. Skeleton (Makefile, `test.c`, original module scaffolding) provided by John Aedo at https://github.com/johnaedo/kernel-mod-skeleton.
