#include <cstring>
#include <iostream>
#include <cmath>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount() {
  bfs.mount();
  curr_dir = 1;
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
}

// make a directory
// ERR_CHECK done
void FileSys::mkdir(const char *name)
{
  // Read current directory into "curr" block
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  // ERR_CHECK: full directory
  if (curr.num_entries == MAX_DIR_ENTRIES)
  {
    cout << "Directory is full" << endl;
    return;
  }

  // ERR_CHECK: file name is too long
  if (strlen(name) >= MAX_FNAME_SIZE)
  {
    cout << "File name is too long" << endl;
    return;
  }
  
  // ERR_CHECK: file exists
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
    if (strcmp(curr.dir_entries[i].name, name) == 0 && curr.dir_entries[i].block_num != 0)
    {
      cout << "File exists" << endl;
      return;
    }
  
  // Check for first unused block
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    if (curr.dir_entries[i].block_num == 0)
    {
      // Initialize new dir block
      dirblock_t new_dir;
      new_dir.magic = DIR_MAGIC_NUM;
      new_dir.num_entries = 0;
      for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        new_dir.dir_entries[i].block_num = 0;
      } 
      short new_block_num = bfs.get_free_block();

      // ERR_CHECK: disk is full
      if (new_block_num == 0)
      {
        cout << "Disk is full" << endl;
        return;
      }

      bfs.write_block(new_block_num, &new_dir);

      // Insert new dir block into curr, then rewrite current block with "curr"
      curr.dir_entries[i].block_num = new_block_num;
      strcpy(curr.dir_entries[i].name, name); 
      curr.num_entries++;
      bfs.write_block(curr_dir, &curr);
      return;
    }
  }
}

// switch to a directory
// ERR_CHECK done
void FileSys::cd(const char *name)
{
  // Read current directory into "curr"
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  // Check each entry for name matches, if so change curr_dir to block_num
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;

      //ERR_CHECK: file is not a directory
      if (!is_directory(block_num))
      {
        cout << "File is not a directory" << endl;
        return;
      }

      curr_dir = block_num;
      return;
    }
  }

  // ERR_CHECK: file does not exist  
  cout << "File does not exist" << endl;
}

// switch to home directory
// ERR_CHECK N/A
void FileSys::home() {
  curr_dir = 1;
}

// remove a directory
// ERR_CHECK done
void FileSys::rmdir(const char *name)
{
  // Read current directory into "curr" block  
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    // Check for name matches
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;
      
      // ERR_CHECK: file is not a directory
      if (!is_directory(block_num))
      {
        cout << "File is not a directory" << endl;
        return;
      }

      // Read candidate to be removed
      dirblock_t candidate;
      bfs.read_block(block_num, &candidate);

      //ERR_CHECK: directory is not empty
      if (candidate.num_entries != 0)
      {
        cout << "Directory is not empty" << endl;
        return;
      }

      // Set block_num in current dir_entries to be unused
      curr.dir_entries[i].block_num = 0;
      curr.num_entries--;

      // Reclaim then rewrite current block
      bfs.reclaim_block(block_num);
      bfs.write_block(curr_dir, &curr);
      return;
    }
  }

  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;
}

// list the contents of current directory
// ERR_CHECK N/A
void FileSys::ls()
{
  // Read current directory
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  // Loop over all of dir_entries and print if block is used
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    if (curr.dir_entries[i].block_num == 0)
      continue;
    cout.write(curr.dir_entries[i].name, strlen(curr.dir_entries[i].name)) << endl;
  }
}

// create an empty i-node
// ERR_CHECK done
void FileSys::create(const char *name)
{
  // Read current directory into "curr" block
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  // ERR_CHECK: full directory
  if (curr.num_entries == MAX_DIR_ENTRIES)
  {
    cout << "Directory is full" << endl;
    return;
  }

  // ERR_CHECK: file name is too long
  if (strlen(name) >= MAX_FNAME_SIZE)
  {
    cout << "File name is too long" << endl;
    return;
  }
  
  // ERR_CHECK: file exists
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
    if (strcmp(curr.dir_entries[i].name, name) == 0 && curr.dir_entries[i].block_num != 0)
    {
      cout << "File exists" << endl;
      return;
    }

  for(int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
     if (curr.dir_entries[i].block_num == 0)
    {
      // Initialize new i-node
      inode_t new_inode;
      new_inode.magic = INODE_MAGIC_NUM;
      new_inode.size = 0;
     
      for (int i = 0; i < MAX_DATA_BLOCKS; i++)
        new_inode.blocks[i] = 0;
 
      short new_block_num = bfs.get_free_block();

      // ERR_CHECK: disk is full
      if (new_block_num == 0)
      {
        cout << "Disk is full" << endl;
        return;
      }
      bfs.write_block(new_block_num, &new_inode);
      
      // Insert the new file into dir_entries
      curr.dir_entries[i].block_num = new_block_num;
      curr.num_entries++;
      strcpy(curr.dir_entries[i].name, name);
      bfs.write_block(curr_dir, &curr);
      return;
    }
  }
}

// append data to a data file
// ERR_CHECK done
void FileSys::append(const char *name, const char *data)
{ 
  // Read current directory
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  // Loop over all dir entries
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    // If name match found, check for existing data blocks
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;

      // ERR_CHECK: file is a directory 
      if (is_directory(block_num))
      {
        cout << "File is a directory" << endl;
        return;
      }

      // Read matched i-node
      inode_t curr_inode;
      bfs.read_block(block_num, &curr_inode);

      // ERR_CHECK: Append exceeds maximum file size
      if (curr_inode.size + strlen(data) > MAX_FILE_SIZE)
      {
        cout << "Append exceeds maximum file size" << endl;
        return;
      }

      // Read size of i-node then go to appropriate data block
      short insertat = floor((double) curr_inode.size / BLOCK_SIZE);

      int needed = strlen(data);
      int copied = 0;

      // Check if block is used, if so fill it first
      if (curr_inode.blocks[insertat] != 0)
      {
        // Read current data block and find remaining space
        datablock_t curr_data;
        bfs.read_block(curr_inode.blocks[insertat], &curr_data);
        short remaining = BLOCK_SIZE - (curr_inode.size % BLOCK_SIZE);

        // insert new data into remaining space
        for (int i = 0; i < remaining && needed != 0 ; i++)
        {
          curr_data.data[BLOCK_SIZE - remaining + i] = data[copied];
          copied++;
          curr_inode.size++;
          needed--;
        }

        // Rewrite blocks
        bfs.write_block(curr_inode.blocks[insertat], &curr_data);
        bfs.write_block(block_num, &curr_inode);
        insertat++;
      }    

      // If there's still data left, create new block
      while (needed != 0)
      {
        datablock_t new_data;

        for (int i = 0; i < BLOCK_SIZE && needed != 0; i++)
        {
          new_data.data[i] = data[copied];
          copied++;
          curr_inode.size++;
          needed--;
        }
        short new_block_num = bfs.get_free_block();
        
        // ERR_CHECK: disk is full
        if (new_block_num == 0)
        {
          cout << "Disk is full" << endl;
          return;
        }

        bfs.write_block(new_block_num, &new_data);
        curr_inode.blocks[insertat] = new_block_num;
        insertat++;
        bfs.write_block(block_num, &curr_inode);
      }
      return;
    }  
  }
  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;
}

// display the contents of a data file
// ERR_CHECK done
void FileSys::cat(const char *name)
{
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    // Check for name matches
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;
      
      // ERR_CHECK: file is a directory 
      if (is_directory(block_num))
      {
        cout << "File is a directory" << endl;
        return;
      }

      // Read matched i-node
      inode_t curr_inode;
      bfs.read_block(block_num, &curr_inode);
      short used = ceil((double) curr_inode.size / BLOCK_SIZE);
      short printed = 0;

      for (int i = 0; i < used; i++)
      {
        datablock_t curr_data;
        bfs.read_block(curr_inode.blocks[i], &curr_data);
        
        for (int i = 0; i < BLOCK_SIZE && printed != curr_inode.size; i++)
        {
          cout << curr_data.data[i];
          printed++;
        }
      }
      cout << endl;
      return;
    }
  }

  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;
}


// display the last N bytes of the file
// ERR_CHECK done
void FileSys::tail(const char *name, unsigned int n)
{
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);
 
  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    // Check for name matches
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;

      // ERR_CHECK: file is a directory 
      if (is_directory(block_num))
      {
        cout << "File is a directory" << endl;
        return;
      }

      // Read matched i-node
      inode_t curr_inode;
      bfs.read_block(block_num, &curr_inode);

      if (n >= curr_inode.size)
      {
        cat(name);
        return;
      }

      // This is how many bytes are in use in the last data block
      short useddata = curr_inode.size % BLOCK_SIZE;
      // Find how many blocks are in use
      short usedblocks = ceil((double) curr_inode.size / BLOCK_SIZE);

      short left = n;
      short copied = 0;

      char result[n];

      datablock_t curr_data;
      
      bfs.read_block(curr_inode.blocks[usedblocks - 1], &curr_data);
      cout << "USEDDATA: " << useddata << endl; 
      for (int i = 0; i < n && i < useddata; i++)
      {
        result[n - 1 - copied] = curr_data.data[useddata - 1 - i];
        left--;
        copied++;
      }

      if (left != 0)
      {
        short fullblocks = floor((double) left / BLOCK_SIZE);

        for (int i = 0; i < fullblocks; i++)
        {
          bfs.read_block(curr_inode.blocks[usedblocks - i - 2], &curr_data);
          for (int j = 0; j < BLOCK_SIZE; j++)
          {
            result[n - 1 - copied] = curr_data.data[BLOCK_SIZE - 1 - j];
            left--;
            copied++;
          }
        }
        bfs.read_block(curr_inode.blocks[usedblocks - fullblocks - 2], &curr_data);
        for (int i = 0; i < left; i++)
        {
          result[n - 1 - copied] = curr_data.data[BLOCK_SIZE - 1 - i];
          copied++;
        }
      }
      
      for (int i = 0; i < n; i++)
      {
        cout << result[i];
      }
      cout << endl;
      return;
    }
  }
  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;
}

// delete a data file
// ERR_CHECK done
void FileSys::rm(const char *name)
{
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;

      // ERR_CHECK: file is a directory 
      if (is_directory(block_num))
      {
        cout << "File is a directory" << endl;
        return;
      }
      
      inode_t curr_inode;
      bfs.read_block(block_num, &curr_inode);
      for (int i = 0; i < MAX_DATA_BLOCKS; i++)
      {
        if (curr_inode.blocks[i] == 0)
          break;
        bfs.reclaim_block(curr_inode.blocks[i]);
        curr_inode.blocks[i] = 0;
      }
  
      curr.dir_entries[i].block_num = 0;
      curr.num_entries--;
      bfs.reclaim_block(block_num);
      bfs.write_block(curr_dir, &curr);
      return;
    }
  }

  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;

}

// display stats about file or directory
// ERR_CHECK done
void FileSys::stat(const char *name)
{
  dirblock_t curr;
  bfs.read_block(curr_dir, &curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; i++)
  {
    if (strcmp(curr.dir_entries[i].name, name) == 0)
    {
      short block_num = curr.dir_entries[i].block_num;
      bool isdir = is_directory(block_num);

      if (isdir)
      {
        cout << "Directory name: " << name << "/" << endl;
        cout << "Directory block: " << block_num << endl;
      }
      else
      {
        inode_t curr_inode;
        bfs.read_block(block_num, &curr_inode);
        short usedblocks = ceil((double) curr_inode.size / BLOCK_SIZE);
        cout << "Inode block: " << block_num << endl;
        cout << "Bytes in file: " << curr_inode.size << endl;
        cout << "Number of blocks: " << usedblocks + 1 << endl;
        cout << "First block: " << curr_inode.blocks[0] << endl;
      }
      return;
    }
  }

  // ERR_CHECK: File does not exist
  cout << "File does not exist" << endl;
}

// HELPER FUNCTIONS (optional)

bool FileSys::is_directory(const short block_num)
{
  dirblock_t dir;
  bfs.read_block(block_num, &dir);
  return (dir.magic == DIR_MAGIC_NUM) ? true : false;
}
