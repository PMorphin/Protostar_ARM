## Heap Exploitation

In this directory we will see Protostar challenges related to Heap Exploitation.

### Compiling
```
gcc -g -o heapX heapX.c
```

ASRL must be disabled
```
pi@raspberrypi:~ $ echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

### Resources

[Heap&Stack](https://gribblelab.org/CBootCamp/7_Memory_Stack_vs_Heap.html)  
[Malloc Internals](https://sourceware.org/glibc/wiki/MallocInternals)  
[Once upon a free() - Phrack Volume 0x0b, Issue 0x39 ](http://phrack.org/issues/57/9.html)
