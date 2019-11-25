# Report

## D

### format

Format direcly writes to the virtual disk, the required FAT basic structure and root directory.
It creates as many fatblocks as needed for the maxblocks and fatentry size ratio
Overwrites fatblock content with in memory FAT content when FAT content changes.
Sets currentDirIndex to rootDirIndex

## C

### myfopen

When files are opened a filedescriptor is populated with the most important info.
Filenames are searched, if they already exist in the directory.
If the file already exists and in readmode, return early with 0 to imitate fopen().
New files have basic direntry_t filled and ready for attaching to parentDir, block location
is the first unused block in the FAT.
FAT is updated.
filedescriptor also stores its parent directories dirIndex and its own entry index in
the partent directories entrylist, this is to make handling file closure easier.

### myfputc

handles returning character, saving buffer to virtual disk when full,
updating FAT, emptying the buffer.

### myfclose

update file metadata in the parent directories entrylist with entry length
and file size.

### myfgetc

Normally just increments position in buffer and returns the character stored
at the specific point in buffer.
Also empties buffer when it's been fully traversed with a new one.
When on the last block of a file, start reading the metadata stored in the
parent directory for the file size. Return EOF when full file length has been
traversed.

## B

### mymkdir

Choose if starting point of folder traversal is root or current dir, remove starting slash
for easier processing in case of root.
traverse path by tokenizing it.
Creata new directories when necessary and storing them in FAT and parent directories
entrylist. Done in the virtual disk.
all folders specified are created so behaviour tries to mimic `mkdir -p`

### mylistdir

tries to handle listing root directory, but fails. Unsure of reason. Time constrainsts.
has practically identical path handling to mymkdir, unfortunately as duplicate code.
Difference in loop content is in the search for the finish, where missing files just return
0 early. If final token in path is found, then copy it's directories entrylsit to
an array and return this for printing and freeing outside of the funciton.

### Missing

did not update handling of myfopen for paths. Handling of printing multiple files in a directory
unsure. Did not have time to test.
