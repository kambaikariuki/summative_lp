# Summative Project - Linux Programming

## Question 1 - Reverse Engineering your own ELF binary

### Compilation and stripping

Compile using `gcc -Wall -O0 -fno-inline -o program program.c`

Strip with `strip program`

### Execution

Run the program with `./program`

### Analysis with readelf

When you run `readelf -h program`, the following output is observed:

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1060
  Start of program headers:          64 (bytes into file)
  Start of section headers:          12624 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         14
  Size of section headers:           64 (bytes)
  Number of section headers:         28
  Section header string table index: 27
```

**Architecture:** x86-64
**Entry point:** 0x1060


### Sections

`.text` - Contains executable code
`.data` - Initialized global variables
`.bss` - uninitialized globals
`.plt` - Procedure Linkage Table
`.got` - Global Offset Table

### Dynamic Linking

Running `readelf -l program | grep interpreter` shows:
`[Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]`
indicating that it is dynamically linked.

### Objdump Analysis

Run `objdump -d program`

**main function:**
```
  1221:       55                      push   %rbp
    1222:       48 89 e5                mov    %rsp,%rbp
    1225:       48 83 ec 10             sub    $0x10,%rsp
    1229:       c7 45 f0 05 00 00 00    movl   $0x5,-0x10(%rbp)
    1230:       8b 45 f0                mov    -0x10(%rbp),%eax
    1233:       48 98                   cltq
    1235:       48 c1 e0 02             shl    $0x2,%rax
    1239:       48 89 c7                mov    %rax,%rdi
    123c:       e8 0f fe ff ff          call   1050 <malloc@plt>
    1241:       48 89 45 f8             mov    %rax,-0x8(%rbp)
    1245:       48 83 7d f8 00          cmpq   $0x0,-0x8(%rbp)
    124a:       75 07                   jne    1253 <malloc@plt+0x203>
    124c:       b8 01 00 00 00          mov    $0x1,%eax
    1251:       eb 4c                   jmp    129f <malloc@plt+0x24f>
    1253:       8b 55 f0                mov    -0x10(%rbp),%edx
    1256:       48 8b 45 f8             mov    -0x8(%rbp),%rax
    125a:       89 d6                   mov    %edx,%esi
    125c:       48 89 c7                mov    %rax,%rdi
    125f:       e8 f5 fe ff ff          call   1159 <malloc@plt+0x109>
    1264:       8b 55 f0                mov    -0x10(%rbp),%edx
    1267:       48 8b 45 f8             mov    -0x8(%rbp),%rax
    126b:       89 d6                   mov    %edx,%esi
    126d:       48 89 c7                mov    %rax,%rdi
    1270:       e8 23 ff ff ff          call   1198 <malloc@plt+0x148>
    1275:       89 45 f4                mov    %eax,-0xc(%rbp)
    1278:       8b 45 f4                mov    -0xc(%rbp),%eax
    127b:       89 05 ab 2d 00 00       mov    %eax,0x2dab(%rip)        # 402c <malloc@plt+0x2fdc>
    1281:       8b 05 a5 2d 00 00       mov    0x2da5(%rip),%eax        # 402c <malloc@plt+0x2fdc>
    1287:       89 c7                   mov    %eax,%edi
    1289:       e8 6c ff ff ff          call   11fa <malloc@plt+0x1aa>
    128e:       48 8b 45 f8             mov    -0x8(%rbp),%rax
    1292:       48 89 c7                mov    %rax,%rdi
    1295:       e8 96 fd ff ff          call   1030 <free@plt>
    129a:       b8 00 00 00 00          mov    $0x0,%eax
    129f:       c9                      leave
    12a0:       c3                      ret
```


**init_array function:**
```
   1164:       c7 45 fc 00 00 00 00    movl   $0x0,-0x4(%rbp)
    116b:       eb 1f                   jmp    118c <malloc@plt+0x13c>
    116d:       8b 45 fc                mov    -0x4(%rbp),%eax
    1170:       48 98                   cltq
    1172:       48 8d 14 85 00 00 00    lea    0x0(,%rax,4),%rdx
    1179:       00
    117a:       48 8b 45 e8             mov    -0x18(%rbp),%rax
    117e:       48 01 d0                add    %rdx,%rax
    1181:       8b 55 fc                mov    -0x4(%rbp),%edx
    1184:       01 d2                   add    %edx,%edx
    1186:       89 10                   mov    %edx,(%rax)
    1188:       83 45 fc 01             addl   $0x1,-0x4(%rbp)
    118c:       8b 45 fc                mov    -0x4(%rbp),%eax
    118f:       3b 45 e4                cmp    -0x1c(%rbp),%eax
    1192:       7c d9                   jl     116d <malloc@plt+0x11d>
    1194:       90                      nop
    1195:       90                      nop
    1196:       5d                      pop    %rbp
    1197:       c3                      ret
```

Loop instruction at: 

    1192:       7c d9                   jl     116d <malloc@plt+0x11d>


**process_array function:**
```
  1198:       55                      push   %rbp
    1199:       48 89 e5                mov    %rsp,%rbp
    119c:       48 89 7d e8             mov    %rdi,-0x18(%rbp)
    11a0:       89 75 e4                mov    %esi,-0x1c(%rbp)
    11a3:       c7 45 f8 00 00 00 00    movl   $0x0,-0x8(%rbp)
    11aa:       c7 45 fc 00 00 00 00    movl   $0x0,-0x4(%rbp)
    11b1:       eb 3a                   jmp    11ed <malloc@plt+0x19d>
    11b3:       8b 45 fc                mov    -0x4(%rbp),%eax
    11b6:       48 98                   cltq
    11b8:       48 8d 14 85 00 00 00    lea    0x0(,%rax,4),%rdx
    11bf:       00
    11c0:       48 8b 45 e8             mov    -0x18(%rbp),%rax
    11c4:       48 01 d0                add    %rdx,%rax
    11c7:       8b 00                   mov    (%rax),%eax
    11c9:       83 e0 03                and    $0x3,%eax
    11cc:       85 c0                   test   %eax,%eax
    11ce:       75 19                   jne    11e9 <malloc@plt+0x199>
    11d0:       8b 45 fc                mov    -0x4(%rbp),%eax
    11d3:       48 98                   cltq
    11d5:       48 8d 14 85 00 00 00    lea    0x0(,%rax,4),%rdx
    11dc:       00
    11dd:       48 8b 45 e8             mov    -0x18(%rbp),%rax
    11e1:       48 01 d0                add    %rdx,%rax
    11e4:       8b 00                   mov    (%rax),%eax
    11e6:       01 45 f8                add    %eax,-0x8(%rbp)
    11e9:       83 45 fc 01             addl   $0x1,-0x4(%rbp)
    11ed:       8b 45 fc                mov    -0x4(%rbp),%eax
    11f0:       3b 45 e4                cmp    -0x1c(%rbp),%eax
    11f3:       7c be                   jl     11b3 <malloc@plt+0x163>
    11f5:       8b 45 f8                mov    -0x8(%rbp),%eax
    11f8:       5d                      pop    %rbp
    11f9:       c3                      ret
```

Conditional jump at :

    11ce:       75 19                   jne    11e9 <malloc@plt+0x199>



**print_result() function:**
```
    120f:       89 c6                   mov    %eax,%esi
    1211:       48 89 d7                mov    %rdx,%rdi
    1214:       b8 00 00 00 00          mov    $0x0,%eax
    1219:       e8 22 fe ff ff          call   1040 <printf@plt>
    121e:       90                      nop
    121f:       c9                      leave
    1220:       c3                      ret
```

Print statement at: 

    1219:       e8 22 fe ff ff          call   1040 <printf@plt>

### Runtime behaviour with strace

Run `strace ./program`

Output:
```
execve("./program", ["./program"], 0x7ffd8c5583f0 /* 92 vars */) = 0
brk(NULL)                               = 0x563f86786000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=133879, ...}) = 0
mmap(NULL, 133879, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f196e76d000
close(3)                                = 0
openat(AT_FDCWD, "/usr/lib/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360w\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 840, 64) = 840
fstat(3, {st_mode=S_IFREG|0755, st_size=2010392, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f196e76b000
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 840, 64) = 840
mmap(NULL, 2034736, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f196e57a000
mmap(0x7f196e59e000, 1511424, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x24000) = 0x7f196e59e000
mmap(0x7f196e70f000, 319488, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x195000) = 0x7f196e70f000
mmap(0x7f196e75d000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e2000) = 0x7f196e75d000
mmap(0x7f196e763000, 31792, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f196e763000
close(3)                                = 0
mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f196e577000
arch_prctl(ARCH_SET_FS, 0x7f196e577740) = 0
set_tid_address(0x7f196e577a10)         = 18789
set_robust_list(0x7f196e577a20, 24)     = 0
rseq(0x7f196e577680, 0x20, 0, 0x53053053) = 0
mprotect(0x7f196e75d000, 16384, PROT_READ) = 0
mprotect(0x563f512b9000, 4096, PROT_READ) = 0
mprotect(0x7f196e7cc000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=16384*1024, rlim_max=RLIM64_INFINITY}) = 0
getrandom("\xd4\x8c\xd4\x01\xd3\x56\xf1\xeb", 8, GRND_NONBLOCK) = 8
munmap(0x7f196e76d000, 133879)          = 0
brk(NULL)                               = 0x563f86786000
brk(0x563f867a7000)                     = 0x563f867a7000
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0), ...}) = 0
write(1, "Result: 12\n", 11Result: 12
)            = 11
exit_group(0)                           = ?
+++ exited with 0 +++
```

**Memory Management calls**:
`brk`
`mmap`
`mprotect`
`munmap`

**Terminal I/O calls**
`write`

**Program start/end**:
`execve`
`exit_group`

**Dynamic linking:**
`openat`
`read`
`close`
`fstat`

**Malloc** appears at system level as `mmap` for large allocations and `brk` for **heap** expansion and **printf** at system level is `write`.

### Dynamic control flow with gdb

Run `gdb ./program`

Then run :
```
break _start
break main
break init_array
```

To view stack frame changes run `bt` which shows:
```
(gdb) bt
#0  0x000055555555519c in process_array ()
#1  0x0000555555555275 in main ()
```

The register state during a conditional branch shows:
```
rip            0x55555555519c      0x55555555519c <process_array+4>
eflags         0x246               [ PF ZF IF ]
cs             0x33                51
```


### Integrated Analysis Summary
Control flow program: 
```
_start
  ↓
libc startup
  ↓
main
  ├── init_array
  ├── process_array
  └── print_result
  ↓
exit
```


**Memory Classification**
```

| Memory Type | Example          |
| ----------- | ---------------- |
| Stack       | `size`, `result` |
| Heap        | `arr`            |
| Global      | `global_counter` |
```

**Function interaction**

`main` allocates memory

`init_array` fills array

`process_array` computes sum

`print_result` outputs result

**PLT/GOT Observations**
`printf` and `malloc` use PLT

First call: resolved via dynamic linker

Address stored in GOT

Subsequent calls: direct jump via GOT



## Question 2

Reads `temperature_data.txt`, counts:
 1. Total lines (including empty ones)
 2. Non-empty lines (valid readings)

Handles both LF (\n) and CRLF (\r\n)
Includes error handling

**Assemble:**
   `nasm -f elf64 temperature.asm`
 Link:
   `ld -o temperature temperature.o`
 Run:
 `./temperature`

**Output:**
```
Total readings: 61
Valid readings: 44
```



## Question 3

A custom C extension module for Python that performs efficient statistical and signal analysis on vibration readings.

#### Setup

Clone the repo and create a virtual environment using:
  `python3 -m venv venv`

Activate the virtual environment:
  `source venv/bin/activate`

To install the required packages from `requirements.txt` run:
  `python3 -m install -r requirements.txt`

You can now build the C extension module using:
```
  python3 setup build
  python3 setup install
```

Finally, you can run the python file with the imported C extension module with:
  `python3 test_vibration.py`



## Question 4: Airport Baggage Handling System using multithreading in C (POSIX threads).

The system consists of:

**Conveyor Belt Loader (Producer Thread)**

Continuously loads luggage onto a shared conveyor buffer.

Takes 2 seconds to load one luggage item.

Each luggage item has a unique ID number (incremental).

The conveyor belt has a maximum capacity of 5 items.

If the belt is full, the loader must wait until space becomes available.

Automatically resumes loading when space is available.

**Aircraft Loader (Consumer Thread)**

Continuously removes luggage from the conveyor belt and loads it onto the aircraft.

Takes 4 seconds to load one luggage item into the aircraft.

If the conveyor belt is empty, the aircraft loader must wait until luggage is available.


**Compilation:**

  `gcc baggage_system.c -o baggage -pthread`

Run the executable:
  `./baggage`


**Console output:**
```
[MONITOR REPORT]
Total Loaded: 4
Total Dispatched: 2
Current Belt Size: 2

Loaded luggage ID 5 | Belt size: 3
Dispatched luggage ID 3 | Belt size: 2
Loaded luggage ID 6 | Belt size: 3
Loaded luggage ID 7 | Belt size: 4

[MONITOR REPORT]
Total Loaded: 7
Total Dispatched: 3
Current Belt Size: 4

Dispatched luggage ID 4 | Belt size: 3
Loaded luggage ID 8 | Belt size: 4
Loaded luggage ID 9 | Belt size: 5
```



## Question 5: A Real-Time Client-Server System

The system simulates a Digital Library Reservation Platform where multiple users connect to a central server to authenticate, view available books, reserve a book, and receive confirmation. This project models a realistic distributed system involving authentication, shared resource control, concurrent clients, and synchronized access to shared data.

**Compilation:**

Client side:
  `gcc client.c -o client`

Server application:
  `gcc server.c -o server -pthread`

**Console output:**

Authentication:
```
Enter Library ID: 1002
Authenticated successfully
Available books:
0. Book_A [AVAILABLE]
1. Book_B [AVAILABLE]
2. Book_C [AVAILABLE]
3. Book_D [AVAILABLE]
4. Book_E [AVAILABLE]

Enter book number to reserve:
```

Server side:
```
--- SERVER STATUS ---
Active users:
 - 1002
Books:
 Book_A : AVAILABLE
 Book_B : AVAILABLE
 Book_C : AVAILABLE
 Book_D : AVAILABLE
 Book_E : AVAILABLE
----------------------
```

Successful reservation of book:
```
User 1002 reserved Book_A

--- SERVER STATUS ---
Active users:
Books:
 Book_A : RESERVED
 Book_B : AVAILABLE
 Book_C : AVAILABLE
 Book_D : AVAILABLE
 Book_E : AVAILABLE
----------------------
Server: RESERVED
Session closed. Goodbye, 1002
```

Active user tracking:
```
--- SERVER STATUS ---
Active users:
 - 1001
 - 1002
 - 1005
 - 1004
 - 1003
Books:
 Book_A : RESERVED
 Book_B : AVAILABLE
 Book_C : AVAILABLE
 Book_D : AVAILABLE
 Book_E : AVAILABLE
----------------------
```
