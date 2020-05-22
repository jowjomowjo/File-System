# FileSystem

Using a single (large) Unix file to emulate a disk. The file should be thought of as an array of blocks. 
There are 1,024 blocks (numbered from 0 to 1023) and each block is 128 bytes. 

Shell (Shell.cpp and Shell.h) 
-Processes commands from the command line. 
File System (FileSys.cpp and FileSys.h) 
-Provides an interface for file system commands. 
Basic File System (BasicFileSys.cpp and BasicFileSys.h) 
-A low-level interface that interacts with the disk. 
Disk (Disk.cpp and Disk.h) 
-Represents a "virtual" disk that is contained within a file. 

There are two types of files: data files (that store a sequence of characters) and directories. 
Data files consist of an inode and zero or more data blocks. 
Directories consist of a single directory block that stores the contents of the directory. 
 
There are four types of blocks used in the file system: 
• Superblock: There is only one superblock on the disk and that is always block 0. It contains a bitmap on what disk blocks are free. 
              The superblock is used by the Basic File System to implement get_free_block() and reclaim_block().
 
• Directories: Represents a directory. The first field is a magic number which is used to distinguish between directories and inodes. 
               The second field stores the number of files located in the directory. The remaining space is used to store the file entries. 
               Each entry consists of a name and a block number (the directory block for directories and the inode block for data files). 
               Unused entries are indicated by having a block number of 0 (we know that’s not a valid block because 0 is reserved for the 
               superblock). Block 1 always contains the directory for the "home" directory. 
 
• Inodes: Represents an index block for a data file. Here, only direct index pointers are used. The first field is a magic number which is
          used to distinguish between directories and inodes. The second field is the size of the file (in bytes). The remaining space 
          consists of an array of indices to data blocks of the file. Use 0 to represent unused pointer entries (files cannot access the 
          superblock). 
 
• Data blocks: Blocks currently used to store data in files. 
 
The different blocks are defined using these structures defined in Blocks.h. These structures are all BLOCK_SIZE (128) bytes.   
 
Since the blocks are a fixed size, there are limits on the size of files, size of file names, and the number of entries in a directory. 
These constants, along with other file system parameters, are also defined in Blocks.h. 
