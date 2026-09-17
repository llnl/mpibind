## Manipulating Files/Directories
### Copy
  - Copy a file or directory
    - `cp <source> <destination>`
    - Example
      - `cp fileA.txt fileB.txt`
  - Copy a directory and all of its files (recursively)
    - `cp -r <source> <destination>`
    - Example
      - `cp -r mySourceDir myDestinationDir`
### Move
  - Move a file or directory
    - `mv <source> <destination>`
    - Example
      - `mv fileA.txt fileA_new.txt`
      - (basically renaming a file)
    - If the destination is a directory, multiple sources can be listed
### Create
  - Create a file
    - `touch <target>`
    - Example
      - `touch fileA.txt`
  - Create a directory
    - `mkdir <target>`
    - Example
      - `mkdir -p make/directories/to/directoryA` - Creates "directoryA" and all parent directories that do not exist starting from the current directory.
### Delete
  - Delete a file
    - `rm <target>`
    - Example
      - `rm -rf haystack` - Deletes "haystack" and, if it's a directory, all of it's contents are deleted recursively.
      
## Filesystem Navigation
### Find current working directory (where you are)
  - `pwd`
### Change working directory
  - `cd /path/to/new/directory`
  - Example
    - `cd ~/experiment/lab`
### List current directory contents
  - `ls /path/to/target`
  - Usages
    - `ls` - List contents of current directory
    - `ls -alh` - List contents, including hidden files, long format, human-readable sizes

## Analyzing files
### Reading files
  - Example
    - `cat fileA.txt`
### Searching files
  - `grep <pattern> <target>`
  - Example
    - `grep -ir needle /path/to/haystack` - Searches for case in-sensitive "needle" in directory "haystack"
    - `grep "search1" fileA.txt | grep "search2"` - Searches for "search2" from the output of "search1"
## Survival Vim
### "Normal" mode
  - Press the `esc` key to enter "normal" mode from any mode.
  - Commands can be entered in "normal" mode.
    - `:wq` - Save (write) and quit
    - `:qa!` - Force quit all sessions
  - Navigate with `h,j,k,l` and arrow keys.
  - Search for text in your current "buffer" with `/` (add `\c` for case-insensitive). `/NEEDLE\c`
### "Insert" mode
  - Press the `i` key in "normal" mode to enter "insert" mode.
  - New text can be entered in "insert" mode - code, data, or notes!
  - Navigate with arrow keys.

## Special locations
- `.` - the current directory
- `..` - the current directory's parent
- `~` - your home directory

## Printable
<img width="1056" height="842" alt="image" src="https://github.com/user-attachments/assets/5d7c7ef1-6537-476a-a8ea-fd660e87957e" />
