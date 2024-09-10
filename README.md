To Do - 11 September:
1. Iterative Insert - Brute Force Linear Scan ==> Woon Yee
2. Bulk Loading Method ==> (Whoever likes coding) x2
3. Create the framework for report ==> (Whoever dont like coding) x2

Next Meeting:
18 September - after SC3020 Tutorial

To run the program:
```
gcc src/main.c src/storage.c src/bptree_iterative.c -I include -o nba_storage

./nba_storage
```

Extra note from Woon Yee:
My operating system is `Linux` and the first few lines in the `main()` function is linux-specific, that is to find my page size for my laptop.

Linux-specific portion:
```
    long block_size = sysconf(_SC_PAGESIZE); 
    if (block_size == -1) {
        perror("Failed to get block size");
        return 1;
    }
    
    printf("Block size: %ld bytes\n", block_size);
```

If you are a window user or mac user, you may check your page size yourself, then set a dummy blocksize by:
`long block_size=XXXX`
