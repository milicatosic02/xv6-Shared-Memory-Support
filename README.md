# xv6 – Shared Memory Support  

This project was developed as part of the **Operating Systems 2024** course.  
The goal was to extend the **xv6 operating system** with support for **shared memory objects**, allowing multiple processes to share data through a common memory space.  

---

## Implemented Changes  

### 1. New System Calls for Shared Memory  
To enable shared memory support, four new system calls were introduced:  

- **`int shm_open(char *name)`**  
  Creates a new shared memory object with the given name if it does not exist, or opens an existing one.  
  Returns a descriptor (similar to file descriptors) used to manage the object.  

- **`int shm_trunc(int shm_od, int size)`**  
  Sets the size of the shared memory object. This can only be done once per object; subsequent attempts return an error.  
  Memory is allocated lazily, aligned to page size, and initialized to zero.  

- **`int shm_map(int shm_od, void **va, int flags)`**  
  Maps the shared memory object into the process’s virtual address space.  
  The mapped address is returned through `*va`. The object must already have a defined size before mapping.  
  Flags (`O_RDONLY`, `O_RDWR`) control whether the object is mapped read-only or read-write.  

- **`int shm_close(int shm_od)`**  
  Closes the shared memory object for the current process and unmaps it if it was mapped.  
  Once all processes close an object, it is destroyed and removed from the system.  

---

### 2. Kernel and Memory Management Modifications  
- Added support for managing up to **64 shared memory objects** at the system level.  
- Each object can have a maximum size of **32 pages**.  
- Each process can have up to **16 simultaneously opened shared memory objects**.  
- Modified process memory layout to allow mapping shared memory into a reserved address space region (`KERNBASE/2 … KERNBASE`).  
- Updated process cleanup routines (`exit()`) to ensure shared memory objects are properly released when a process terminates.  
- Ensured that child processes created via `fork()` inherit access to already opened shared memory objects.  

---

### 3. Concurrency and Error Handling  
- Implemented synchronization to handle concurrent access to shared memory objects by multiple processes.  
- Added robust error handling to ensure system stability in cases of invalid parameters, failed allocations, or improper usage of system calls.  
- Introduced cycle detection and cleanup logic to prevent dangling references and memory leaks.  

---

## Example Workflow  
1. A process calls `shm_open("example")` to create or open a shared memory object.  
2. The process calls `shm_trunc()` to set the object size.  
3. The process maps the object into its address space using `shm_map()`.  
4. Another process calls `shm_open("example")` and `shm_map()`, gaining access to the same memory region.  
5. Both processes can now read and write to the shared region concurrently.  
6. Once finished, each process calls `shm_close()`; when all processes close the object, the system frees the memory.  

---

## Learning Outcomes  
This project provided hands-on experience with:  
- **Shared memory mechanisms** as used in modern operating systems.  
- Designing and implementing **new system calls** in xv6.  
- **Virtual memory management** and page allocation.  
- Handling **resource cleanup and concurrency issues** in kernel development.  
- Ensuring new functionality integrates smoothly without breaking existing xv6 behavior.  

---

By implementing shared memory, xv6 was extended with an essential inter-process communication (IPC) feature, making it closer to real-world operating systems in terms of process collaboration and memory management.  
