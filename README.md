Visualization (View in edit mode):
+----------------+          +---------------------+          +---------------------+
|  BPlusTree     |          |   Root Node         |          |   Child1 (Leaf)     |
|----------------|          |---------------------|          |---------------------|
| - root: ------>| -------> | isLeaf: false       |          | isLeaf: true        |
+----------------+          | numKeys: 2          |          | numKeys: 3          |
                            | keys: [20.0, 40.0]  |          | keys: [10.0, 15.0]  |
                            | children:           |          | data: [ptr1, ptr2]  |
                            |   Child1            |          | next: ------> Child2|
                            |   Child2            |          +---------------------+
                            |   Child3            |          +---------------------+
                            +---------------------+          |   Child2 (Leaf)     |
                                                             | isLeaf: true        |
                                                             | numKeys: 2          |
                                                             | keys: [20.0, 35.0]  |
                                                             | data: [ptr3, ptr4]  |
                                                             | next: ------> Child3|
                                                             +---------------------+
                                                             +---------------------+
                                                             |   Child3 (Leaf)     |
                                                             | isLeaf: true        |
                                                             | numKeys: 1          |
                                                             | keys: [40.0, 45.0]  |
                                                             | data: [ptr5, ptr6]  |
                                                             | next: NULL          |
                                                             +---------------------+
                                                             
                                                             
## To Do - 19 September (WY):
1. Merge two method in main.c - Done
2. Add time for tree constructions - Done
3. Add images to report - WIP
4. Save index on the disk - using Json maybe - Added

## Bulk insertion -- Bottom up approach. 
Fill the leaf layer first, only split node when max key reached. On last node, if number of keys < Min_Key, this node will take from the previous node. Recursively Construct the next layer based on the leaf layer, until the root layer.
run code:
```
gcc -I include src/main.c src/storage.c src/bptree_bulk_loading.c src/bptree_iterative.c -o bulk
./bulk
```

## To Do - 11 September:
1. Iterative Insert - Brute Force Linear Scan ==> Woon Yee `Done`
2. Bulk Loading Method ==> (Whoever likes coding) x2 yichen alex
3. Create the framework for report ==> (Whoever doesnt like coding) x2 - ming ru, andrew

**Next Meeting: 18 September - after SC3020 Tutorial**

## 18 September Meeting Update  
1. Use `bool` for isLeaf - Done - `Woon Yee`
2. add `** data` to point to the NBA Records - Done - `Woon Yee`
3. To explain `**children` is pointing to a list of children, if we use `*children`, it is only pointing to a children. (A node should have pointers pointing to multiple children)
4. Iterative Done, cross check iterative result with bulk loading result.

### Question to ask prof:
1. The B+ tree must point to the datablock in disk - Which means we load the NBA records from disk to memory, then we connect it with B+ tree?

### Others
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
