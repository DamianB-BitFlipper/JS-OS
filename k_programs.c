
#include "k_programs.h"
#include "k_stdio.h"
#include "k_shell.h"

#include "sound.h"
#include "keyboard.h"
#include "mouse.h"

#include "timer.h"
#include "isr.h"

#include "fs.h"
#include "initrd.h"

#include "graphics.h"

#include "x_server.h"


extern cursor_x, cursor_y, globalFreq;

/*screen attributes*/
extern VGA_width, VGA_height;
extern char *path; //the char array that contains the path from the root to the current directory

extern u32int fs_location;
extern u32int nroot_nodes;
extern u32int greatestFS_location;
/**To add a new program, add new case statement in function "runShellFunction" in k_programs.c
 * add the program name that the user will have to type in "char *programsList[]" in k_shell.c
 * add initial run function of that program into k_programs.h
 * make the function that will be run in the case stement below, nomenclature-> program_<command name> **/

void runShellFunction(int runFunction, char *arguements)
{

  switch(runFunction)
  {
    case 0:
      program_ascii(arguements);
      break;
    case 1:
      program_echo(arguements);
      break;
    case 2:
      program_tinytext(arguements);
      break;
    case 3:
      program_GUI_pong(arguements);
      break;
    case 4:
      program_song(arguements);
      break;
    case 5:
      program_JS_viewer(arguements);
      break;
    case 6:
      program_start(arguements);
      break;
    case 7:
      program_ls(arguements);
      break;
    case 8:
      program_cd(arguements);
      break;
    case 9:
      program_now(arguements);
      break;
    case 10:
      program_mkdir(arguements);
      break;
    case 11:
      program_cp(arguements);
      break;
    case 12:
      program_cat(arguements);
      break;
    case 13:
      program_rm(arguements);
      break;
    case 14:
      program_pwd(arguements);
      break;
    case 15:
      program_help(arguements);
      break;
    case 16:
      program_mv(arguements);
      break;

    ////~ case that-value:
      ////~ Code to execute if <variable> == that-value
      ////~ break;
    ////~ ...
    ////~ default:
      ////~ Code to execute if <variable> does not equal the value following any of the cases
      ////~ break;
  }

}

void program_ascii(char *arguements)
{

  if(k_strcmp(arguements, "-h") == 0 || k_strcmp(arguements, "-help") == 0 || k_strlen(arguements) == 0)
  {
    k_printf("Usage: ascii -<animal>\nType ascii -list to see list of animals");

  }else if(k_strcmp(arguements, "-list") == 0)
  {
    /*The cow saying "Moo" */
    k_printf("Animals:\n\tcow\n\tduck\n\tsquirrel or skverl");

  }else if(k_strcmp(arguements, "-cow") == 0)
  {
    /*The cow saying "Moo" */
    k_printf("             ^__^\n");
    k_printf("    _____    (OO)\\_______\n");
    k_printf("   | MOO | --(__)\\       )\\/\\\n");
    k_printf("    -----       ||--WWW |\n");
    k_printf("                ||     ||\n");
  }else if(k_strcmp(arguements, "-duck") == 0)
  {
    /*The duck*/
    k_printf("                         _____\n");
    k_printf("                     _-~~     ~~-_//\n");
    k_printf("                   /~             ~\\\n");
    k_printf("                  |              _  |_\n");
    k_printf("                 |         _--~~~ )~~ )___\n");
    k_printf("                \\|        /   ___   _-~   ~-_\n");
    k_printf("                \\          _-~   ~-_         \\\n");
    k_printf("                |         /         \\         |\n");
    k_printf("                |        |           |     (O  |\n");
    k_printf("                 |      |             |        |\n");
    k_printf("                 |      |   O)        |       |\n");
    k_printf("                 /|      |           |       /\n");
    k_printf("                 / \\ _--_ \\         /-_   _-~)\n");
    k_printf("                   /~    \\ ~-_   _-~   ~~~__/\n");
    k_printf("                  |   |\\  ~-_ ~~~ _-~~---~  \\\n");
    k_printf("                  |   | |    ~--~~  / \\      ~-_\n");
    k_printf("                   |   \\ |                      ~-_\n");
    k_printf("                    \\   ~-|                        ~~--__ _-~~-,\n");
    k_printf("                     ~-_   |                             /     |\n");
    k_printf("                        ~~--|                                 /\n");
    k_printf("                          |  |                               /\n");
    k_printf("                          |   |              _            _-~\n");
    k_printf("                          |  /~~--_   __---~~          _-~\n");
    k_printf("                          |  \\                   __--~~\n");
    k_printf("                          |  |~~--__     ___---~~\n");
    k_printf("                          |  |\n");
  }else if(k_strcmp(arguements, "-squirrel") == 0 || k_strcmp(arguements, "-skverl") == 0)
  {
    /*The duck*/
    k_printf("                              _\n");
    k_printf("                          .-'` `}\n");
    k_printf("                  _./)   /       }\n");
    k_printf("                .'o   \\ |       }\n");
    k_printf("                '.___.'`.\\    {`\n");
    k_printf("                /`\\_/  , `.    }\n");
    k_printf("                \\=' .-'   _`\\  {\n");
    k_printf("                 `'`;/      `,  }\n");
    k_printf("                    _\\       ;  }\n");
    k_printf("                   /__`;-...'--'\n");

  }else{
    k_printf("Arguement: %s not found\n", arguements);
    k_printf("Usage: ascii -<animal>\nType ascii -list to see list of animals");
  }

  //~ k_printf("\n%d\n", k_strcmp(arguements, "-duck"));
}

void program_echo(char *arguements)
{
  k_printf("%s\n", arguements);
}

void program_tinytext(char *arguements)
{
  k_save(); //save screen to char array
  k_clear(); //clear screen so text editor can enter
  turnShellIndentOnOff(OFF); //turn shell indents off

  int yHolder = cursor_y; //store y position

  /*Add program name and title*/
  //cursor_y = 0; //set y position to top so message can be printed there
  //~ k_printf("%Cw%cbk  JS - tinytext 0.0.1%Cbk%cw");
  //~ k_printf("%Cw%cbk  JS - tinytext 0.0.1                New Buffer                                 %Cbk%cw ");
  k_setprintf(0, 0, "%Cw%cbk  JS - tinytext 0.0.1                New Buffer                                 %Cbk%cw ");
  //~ k_printf("  JS - tinytext 0.0.1                New Buffer                                 ");
  //~ k_printf("POOP");
  //cursor_y = yHolder; //restore cursor_y position
  //cursor_x = 0;

  setScreenYMinMax(1, 22); //reserve 3 rows at the bottom of the screen for mesages and 1 row at the top for program name
  turnShellInputOnOff(OFF); //turn shell indents off

  arrowKeyFunction("write", "left", &normalHCursor); //sets left and write to use a different function
  arrowKeyFunction("write", "up", &normalVCursor); //sets left and write to use a different function

  //~ k_printf("writing");
}

void program_song(char *arguements)
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  if(k_strcmp(arguements, "-pacman") == 0)
  {
    song_pacman();
  }
}

void program_ls(char *arguements)
{

  //if(k_strcmp(arguements, "x") == 0)
  //{
    ////~ asm volatile("sti");
    ////~ init_mouse(); // Initialise mouse

    //xServer();
  //}

  //int i = 0;
  //struct dirent *node = 0;
  //while ( (node = readdir_fs(fs_root, i)) != 0)
  //{
    //k_printf("Found file ");
    //k_printf(node->name);
    //fs_node_t *fsnode = finddir_fs(fs_root, node->name);

    //if ((fsnode->flags&0x7) != FS_DIRECTORY)
    //{

      //k_printf("\n\t contents: \"");
      //unsigned char buf[256];
      //u32int sz = read_fs(fsnode, 0, 256, buf);
      //int j;
      //for (j = 0; j < sz; j++)
      //{
        ////~ k_putChar(buf[j]);
        //k_printf(" %h", buf[j]);
      //}

      //k_printf("\"\n");
    //}
    //i++;
  //}
  static fs_node_t *fsnode;

  fsnode = &root_nodes[currentDir_inode];
  static struct dirent *node = 0;
  int i = 0;

  while ( (node = readdir_fs(fsnode, i)) != 0)
  {
    k_printf(node->name);
    fs_node_t *tmpfsnode = finddir_fs(fsnode, node->name);

    if ((tmpfsnode->flags&0x7) == FS_DIRECTORY)
    {
      k_printf(" (dir)\n");
    }else{
      k_printf("\n");
    }

    i++;
  }

}

int program_cd(char *arguments)
{
  int argLen = strlen(arguments);
  int i = 0, originalI = 0;

  int isInitDirRoot = FALSE, initDir = currentDir_inode;

  char *cd;

  fs_node_t *dir, *initial_dir;

  do
  {
    originalI = i;

    /*Parcing the arguments*/
    for(i; i < argLen; i++)
    {
      if(*(arguments + i) == '/')
      {
        if(i != 0) //if the first character is "/" it is ok to not break and continue
        {
          break;
        }else{
          //~ initial_dir = fs_root;
          //~ setCurrentDir(initial_dir);
          isInitDirRoot = TRUE;
          i++;
          break;
        }
      }
    }

    /*(i - originalI) is the length of the portion after the portion
     * we just processed. IE: arguments = "./poop", when we process
     * the "." i == 2 and originalI == 2. Continuing on, for "poop",
     * i will equal 6 and originalI equals 2. 6 - 2 is 4, which is
     * how long poop is */
    cd = (char*)kmalloc((i - originalI) + 1); //+1 being the \000 at the end of a string

    memcpy(cd, (arguments + originalI), (i - originalI));
    *(cd + (i - originalI)) = 0; //adds the \000 at the end

    if(i == 1 && isInitDirRoot == TRUE)
    { //the directory is "/" so we are starting in the root instead of currentDir
      dir = fs_root;
      isInitDirRoot = FALSE;
    }else{
      dir = finddir_fs(&root_nodes[currentDir_inode], cd);
      i++;
    }

    if(dir != 0 && dir->flags == FS_DIRECTORY) //if dir exists and is a directory
    {
      //if(dir == fs_root) //if the dir is root, increment originalI by strlen so next dir doesn't end up "/dir" istead of "dir"
      //{
        ////~ i++;
        ////~ originalI = originalI + strlen(dir->name);
      //}else{
        //originalI = i;

      //}

      setCurrentDir(dir);
      kfree(cd);

    }else{
      if(*(cd) != 0) //if the string cd has contents, usually if the char *arguments is "//" for some reason, cd will have no content (begins with a \000)
      {
        k_printf("In \"%s\", \"%s\" is not a directory\n", arguments, cd);
      }else{
        k_printf("In \"%s\", \"%s\" is not a directory\n", arguments, arguments);

      }
      kfree(cd);

      /*if we have an initial (starting point) dir, set the current dir
       * to that dir as it was before the user typed an incorrect path name */
      setCurrentDir(&root_nodes[initDir]);

      break;

      //failure!
      return 1;
    }
  }
  while(i < argLen);

  //success!
  return 0;

}

void program_cp(char *arguments)
{

  int nArgs = countArgs(arguments);

  char *args[nArgs];
  getArgs(arguments, args);

  int currentIno = currentDir_inode;

  int dirCount, fileCount;

  dirFilePathCount(args[0], &dirCount, &fileCount);

  char *dirPath, *filePath;

  dirPath = (char*)kmalloc(dirCount + 1);
  filePath = (char*)kmalloc(fileCount + 1);

  cdFormatArgs(args[0], dirPath, filePath);
  
  fs_node_t *src;
  
  //~ src = finddir_fs(&root_nodes[currentDir_inode], args[0]); //gets the structure of the first argument input
  src = finddir_fs(&root_nodes[currentDir_inode], filePath); //gets the structure of the first argument input

  if(src != 0) //if there is a file with the name of the first argument
  {
    int i = 0, length = strlen(args[1]), count = -1;

    for(i; i < length; i++)
    {
      /* using the following, after it is done executing, the integer
       * count will be equeal to the number of character before and
       * including the last "/" in the destination string of input */
      if(*(args[1] + i) == '/') //for every "/" in the dest of input, increment count to i
      {
        count = i;
      }
    }

    if(count == -1)
    {
      count = i - 1;
    }

    char destPath[count + 2]; //the extra +1 is for the \000 and the other one is because i, and thus count start from 0 on the first element
    memcpy(destPath, args[1], count + 1);
    *(destPath + count + 1) = 0; //add \000 to the end

    char restPath[length - count];
    memcpy(restPath, args[1] + count + 1, length - count - 1);
    *(restPath + length - count - 1) = 0; //add the \000 to the end

    int work = program_cd(destPath);
    int b;

    fs_node_t *restSrc;
    restSrc = finddir_fs(&root_nodes[currentDir_inode], restPath);

    /*if the rest to the user text after the final "/" is a file and
     * a directory, go to that directory and then create the file */
    if(restSrc != 0 && restSrc->flags == FS_DIRECTORY)
    {
      program_cd(restPath);

      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], src->name, src->length);
      //~ memcpy((u32int*)(file_headers[copiedFile->inode].offset), (u32int*)(file_headers[src->inode].offset), src->length);

      for(b = 0; b < (int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        memcpy((u32int*)(root_nodes[copiedFile->inode].blocks[b]), (u32int*)(root_nodes[src->inode].blocks[b]), BLOCK_SIZE);
      }

    }else if(*restPath != 0) //if there is contents after the last "/"
    {
      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], restPath, src->length);
      //~ memcpy((u32int*)(file_headers[copiedFile->inode].offset), (u32int*)(file_headers[src->inode].offset), src->length);

      for(b = 0; b < (int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        memcpy((u32int*)(root_nodes[copiedFile->inode].blocks[b]), (u32int*)(root_nodes[src->inode].blocks[b]), BLOCK_SIZE);
      }

    }else if(work == 0) //if the cd function above did not fail, ie: we should still copy the file
    {
      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], src->name, src->length);
      //~ memcpy((u32int*)(file_headers[copiedFile->inode].offset), (u32int*)(file_headers[src->inode].offset), src->length);

      for(b = 0; b < (int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        memcpy((u32int*)(root_nodes[copiedFile->inode].blocks[b]), (u32int*)(root_nodes[src->inode].blocks[b]), BLOCK_SIZE);
      }

    }

    setCurrentDir(&root_nodes[currentIno]); //sets the current dir back to the original

  }

  int i;
  for(i = 0; i < nArgs; i++)
  {
    kfree(args[i]);
  }

}

void program_mv(char *arguments)
{
  int nArgs = countArgs(arguments);

  char *args[nArgs];
  getArgs(arguments, args);
  
  program_cp(arguments); //copy the source to the dest
  program_rm(args[0]);
}

void program_cat(char *arguments)
{
  int initDir = currentDir_inode;

  int dirCount, fileCount;

  dirFilePathCount(arguments, &dirCount, &fileCount);

  char *dirPath, *filePath;

  dirPath = (char*)kmalloc(dirCount + 1);
  filePath = (char*)kmalloc(fileCount + 1);

  cdFormatArgs(arguments, dirPath, filePath);
  
  fs_node_t *file;

  file = finddir_fs(&root_nodes[currentDir_inode], filePath);

  if(file != 0 && file->flags == FS_FILE) //if the file exists
  {

    unsigned char buf[file->length];

    u32int sz = read_fs(file, 0, file->length, buf); //assign the content of file to buf[]
    int j;
    for (j = 0; j < sz; j++)
    {
      k_putChar(buf[j]);
    }

    k_printf("\n");

  }else if(file->flags == FS_DIRECTORY)
  {
    k_printf("%s: Is a directory\n", arguments);
  }
  else{
    k_printf("%s: No such file\n", arguments);
  }

  setCurrentDir(&root_nodes[initDir]);
  
}

void program_rm(char *arguments)
{
  int initialDir = currentDir_inode;
  int a, length = strlen(arguments);

  //goes from the back, when it breaks, a == the position of the very last "/"
  for(a = length - 1; a >= 0; a--)
  {
    if(*(arguments + a) == '/')
    {
      break;
    }
  }

  if(a == 0) //if the char *arguments had no "/"
  {
    fs_node_t *isDir;

    isDir = finddir_fs(&root_nodes[currentDir_inode], arguments);

    if(isDir == 0) //if there is no such file
    {
      k_printf("%s: No such file\n", arguments);

      return;
    }else if(isDir != 0 && isDir->flags == FS_DIRECTORY) //if the input is just a directory, return error
    {
      k_printf("cannot remove '%s': Is a directory\n", arguments);

      return;
    }else if(isDir != 0 && isDir->flags == FS_FILE) //if the input is just an existing filename (which is perfectly legal)
    {
      a = -1; //set a to be just right for those kmallocs below
    }
  }else if(a == length - 1) //if char *arguments ends in "/", therefore no file has been specified
  {
    k_printf("%s: No such file\n", arguments);

    return;
  }

  char *dirPath, *filePath;

  dirPath = (char*)kmalloc(a + 2);
  filePath = (char*)kmalloc(length - a);

  cdFormatArgs(arguments, dirPath, filePath);

  fs_node_t *file;

  file = finddir_fs(&root_nodes[currentDir_inode], filePath);

  if(file != 0 && file->flags == FS_FILE) //if the file exists and is a file
  {
    struct dirent *dirent2;
    dirent2 = (struct dirent*)kmalloc(sizeof(struct dirent));

    //~ dirent2 = readdir_fs(&root_nodes[currentDir_inode], file->inode);

    int i = 0, b = 0;
    fs_node_t *dirNode;
    dirNode = &root_nodes[currentDir_inode];

    do
    {
      if(*(u32int*)(dirNode->blocks[b] + i) == file->inode)
      {

        dirent2->ino = *(u32int*)(dirNode->blocks[b] + i);
        dirent2->rec_len = *(u16int*)(dirNode->blocks[b] + i + sizeof(dirent2->ino));


        break;

      }else{
        //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
        //i = i + *(u16int*)(dirNode->blocks[b] + i + sizeof(dirent2->ino));

        //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
        if(*(u16int*)(dirNode->blocks[b] + i + sizeof(dirent2->ino)) != 0)
        {
          //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
          i = i + *(u16int*)(dirNode->blocks[b] + i + sizeof(dirent2->ino));
        }else{ //this is the last direct, add 1 to block and reset the offset (i)
          i = 0;
          b++;
        }

      }

      if(b > (int)((dirNode->length - 1) / BLOCK_SIZE))
      {
        //error, block exceds the number of blocks dirNode has
        return;
      }

    }
    while(1);

    /*shifts the dirent data in the directory*/
    shiftData((u32int*)(dirNode->blocks[b] + i + dirent2->rec_len), -1 * dirent2->rec_len, BLOCK_SIZE - i - dirent2->rec_len); //the "-1 *" is used to show shift to the left

    //~ k_printf("\nBefore %s and size is: %d address %h\n", file_headers[dirent2->ino].offset, file->length, (file_headers[dirent2->ino].offset));

    /* shifts the offset of all files after this file we are deleting,
     * by the size of the file we are deleting */
    //int n;
    //for(n = 0; n < nroot_nodes; n++)
    //{
      //if(file_headers[n].offset > file_headers[dirent2->ino].offset)
      //{
        //file_headers[n].offset = file_headers[n].offset - file->length;
      //}
    //}

    //Delete the actual file content
    //~ memset(&(file_headers[dirent2->ino].offset), 0, file->length);
    //~ shiftData((u32int*)(file_headers[dirent2->ino].offset + file_headers[dirent2->ino].length), -1 * file_headers[dirent2->ino].length, greatestFS_location - (u32int)(file_headers[dirent2->ino].offset + file_headers[dirent2->ino].length)); //the "-1 *" is used to show shift to the left
    int c;
    for(c = 0; c < (int)((file->length - 1) / BLOCK_SIZE) + 1; c++)
    {
      memset(file->blocks[c], 0, BLOCK_SIZE);
      kfree(file->blocks[c]);
    }
    //~ k_printf("\nAfter %s\n", file_headers[dirent2->ino].offset);

    // Edit the file's header - currently it holds the file offset
    // relative to the start of the ramdisk. We want it relative to the start
    // of memory.
    //memset(file_headers[dirent2->ino].name, 0, dirent2->name_len); //delete the name
    //file_headers[dirent2->ino].offset = 0;
    //file_headers[dirent2->ino].length = 0;

    // Delete the file node.
    memset(root_nodes[dirent2->ino].name, 0, dirent2->name_len); //delete the name
    root_nodes[dirent2->ino].mask = root_nodes[dirent2->ino].uid = root_nodes[dirent2->ino].gid = 0;
    root_nodes[dirent2->ino].length = 0;
    root_nodes[dirent2->ino].inode = 0;
    root_nodes[dirent2->ino].flags = 0;
    root_nodes[dirent2->ino].read = 0;
    root_nodes[dirent2->ino].write = 0;
    root_nodes[dirent2->ino].readdir = 0;
    root_nodes[dirent2->ino].finddir = 0;
    root_nodes[dirent2->ino].open = 0;
    root_nodes[dirent2->ino].close = 0;
    root_nodes[dirent2->ino].impl = 0;
    root_nodes[dirent2->ino].ptr = 0;

    kfree(dirent2);

  }else if(file != 0 && file->flags == FS_DIRECTORY)
  {
    k_printf("cannot remove '%s': Is a directory\n", filePath);
  }else{
    k_printf("%s: No such file\n", arguments);
  }

  kfree(dirPath);
  kfree(filePath);

  setCurrentDir(&root_nodes[initialDir]);

}

void program_mkdir(char *arguments)
{
  int initDir = currentDir_inode;

  int length = strlen(arguments);

  int i, count = -1;
  for(i = 0; i < length; i++)
  {
    if(*(arguments + i) == '/')
    {
      count = i;
    }
  }

  if(count != -1) //if count was changed, ie: in the char *arguments there was atleast one "/"
  {
    char *dirString;
    /* +1 being the \000 at the end, and the other plus one is due to
     * that we start counting from 0, 1, 2, etc. so if count was to equal
     * 0, that would mean 1 character, so we boost count up by one to equal
     * the number of characters */
    dirString = (char*)kmalloc(count + 2);

    memcpy(dirString, arguments, count + 1); // +1 for same reason as above, count starts from 0 for the first character
    *(dirString + count + 1) = 0; // add the \000 at the end of the string "dirString"

    int work = program_cd(dirString);


    if(work == 0) //if the program_cd() did not return an error
    {
      char *restString;
      restString = (char*)kmalloc(length - count);

      memcpy(restString, arguments + count + 1, length - count - 1);
      *(restString + length - count - 1) = 0; //add the \000 at the end

      fs_node_t *dir;

      dir = createDirectory(&root_nodes[currentDir_inode], restString);

      setCurrentDir(&root_nodes[initDir]);

      kfree(restString);
    }

    kfree(dirString);
  }else{
    fs_node_t *dir;

    dir = createDirectory(&root_nodes[currentDir_inode], arguments);
  }
}

void program_pwd(char *arguments)
{
  k_printf("%s\n", path);
}

void program_now(char *arguments)
{
  datetime_t time;

  time = getDatetime();

  k_printf("%d:%d:%d %d/%d/%d\n", time.hour, time.min, time.sec, time.month, time.day, time.year);
}

void program_help(char *arguements)
{
  k_printf("Commands list:");
  k_printf("\n\tascii - prints ascii animals [ascii -squirrel]");
  k_printf("\n\tcat - prints the contents of a file [cat test.txt]");
  k_printf("\n\tcd - changes directories [cd /test]");
  k_printf("\n\tcp - copies a file to a destination [cp test.txt /test/dir]");
  k_printf("\n\techo - prints text [echo hello]");
  k_printf("\n\thelp - prints the command list [help]");
  k_printf("\n\tls - lists the contest in the current directory [ls]");
  k_printf("\n\tmkdir - creates a directory [mkdir /test]");
  k_printf("\n\tmv - moves (renames) a file to a destination [mv /test/source.txt /test/renamed.txt]");
  k_printf("\n\tnow - prints the date/time [now]");
  k_printf("\n\tpong - starts a game of pong [pong -gui]");
  k_printf("\n\tpwd - prints the working (current) directory [pwd]");
  k_printf("\n\trm - deletes a file [rm /test/file.txt]");
  k_printf("\n\tsong - plays a tune [song -pacman]");
  k_printf("\n\tstart - starts a program [start x]");
  k_printf("\n\ttinytext - a very simple text editor [tinytext]");
  k_printf("\n\tviewer - a very simple image viewer [viewer]\n");
}

void program_start(char *arguements)
{

  if(k_strcmp(arguements, "x") == 0)
  {
    //~ asm volatile("sti");
    //~ init_mouse(); // Initialise mouse

    xServer();
  }

}

//~ objects arrayOfObjects[5]; //make array of the 5 attributes of the 2 objects
static int images = 2;
objects arrayOfImages[2]; //make array of the 5 attributes of the 2 objects

void program_JS_viewer(char *arguements)
{
  //~ k_printf("\n\nPOOP\n");
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  /*initialize the objects attributes*/
  arrayOfImages[0].x = 0; //Background
  arrayOfImages[0].y = 0;
  //~ arrayOfImages[0].width = VGA_width;
  arrayOfImages[0].width = 740;
  //~ arrayOfImages[0].height = VGA_height;
  arrayOfImages[0].height = 480;
  arrayOfImages[0].color = 4;
  arrayOfImages[0].priority = 0;

  //~ VGA_init(320, 200, 256); //initialize the gui
  VGA_init(1024, 768, 24); //initialize the gui

  int x, y = 0;

  //~ for(x = 0; x < 300; x++)
  //~ {
    //~ for(y = 0; y < 300; y++)
    //~ {

      putRect(0, 0, 400, 400, 0xbf2bc4);

      mSleep(10);
      //~ y++;
    //~ }
  //~ }

  //~ VGA_init(640, 480, 16); //initialize the gui

  //~ VGA_clear_screen();

  //~ putPixel(100, 100, 0xc812e7);
  //~ putRect(100, 100, 100, 75, 0xc812e7);
  //~ putRect(arrayOfImages[0].x, arrayOfImages[0].y, arrayOfImages[0].width, arrayOfImages[0].height, arrayOfImages[0].color);

}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*------------------------------PONG----------------------------------*/
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////



//~ objects arrayOfObjects[5]; //make array of the 5 attributes of the 5 objects
static int numberOfObjects = 5;
objects arrayOfObjects[5]; //make array of the 5 attributes of the 5 objects

int upDown = 0, pongPlay = ON;

//~ static double directionLR = 1; //Left right direction (1 is up, -1 is down)
//~ static double directionUD = 1; //up down direction (1 is left, -1 is right)
  //~
//~ static int xCoord = 150; //default x for ball
//~ static int yCoord = 100; //default y for ball

void moveLeftPong()
//~ void moveLeftPong(int movement)
{

  static int yCoord = 70;

  //~ int speed = 7;
  int speed = 2;

  if(pongPlay == ON)
  {
    //~ yCoord = yCoord - 5 * movement;
    yCoord = yCoord - speed * upDown;

    if(yCoord < 0)
    {
      yCoord = 0;
    }else if(yCoord + arrayOfObjects[1].height > VGA_height)
    {
      yCoord = VGA_height - arrayOfObjects[1].height;
    }

    putRect(arrayOfObjects[1].x, yCoord, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);

    arrayOfObjects[1].y = yCoord;

    refreshObjects(&arrayOfObjects[0], numberOfObjects, 1, 1, speed + 3, 0); //refresh 1rd object, left paddle

    upDown = 0;

  }else if(pongPlay == OFF) //signals that the ball has hit the left or right edge
  {
    /*restore defaults*/
    yCoord = 70;
    arrayOfObjects[1].y = 70;

    /*resets the upDown and pongPlay*/
    upDown = 0;
    pongPlay = ON;

    /*refreshes screen*/
    refreshScreen_std(&arrayOfObjects[0], numberOfObjects);

    /*sleeps 1 second (1000 milliseconds) so player can get ready*/
    mSleep(1000);

  }

}

void moveRightPong()
{
  //COMPUTER AI -ish

  static int yCoord = 70;

  int tmpY;

  if(pongPlay == ON)
  {
    //~ yCoord = yCoord - 5 * movement;
    //~ yCoord = yCoord - 5 * upDown;
    //~ tmpY = arrayOfObjects[3].y - 15; //follows the y coordinate of the ball
    tmpY = arrayOfObjects[3].y - (arrayOfObjects[2].height / 2); //follows the y coordinate of the ball

    if(tmpY < 0)
    {
      yCoord = 0;
    }else if(tmpY + arrayOfObjects[2].height > VGA_height)
    {
      yCoord = VGA_height - arrayOfObjects[2].height;
    }else{
      yCoord = arrayOfObjects[3].y - (arrayOfObjects[2].height / 2);
    }

    putRect(arrayOfObjects[2].x, yCoord, arrayOfObjects[2].width, arrayOfObjects[2].height, arrayOfObjects[2].color);
    //~ putRect(arrayOfObjects[1].x, yCoord, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);

    arrayOfObjects[2].y = yCoord;
    //~ arrayOfObjects[1].y = yCoord;

    refreshObjects(&arrayOfObjects[0], numberOfObjects, 2, 1, 6, 0); //refresh 2rd object, right paddle
    //~ refreshObjects(&arrayOfObjects[0], numberOfObjects, 1, 1, 6); //refresh 2rd object, right paddle

    //~ upDown = 0;
  }else if(pongPlay == OFF) //if movePongBall has signalled that the ball has hit the left/right edge
  {
    /*retores defaults*/
    yCoord = 70;
    arrayOfObjects[2].y = 70;

    /*sets upDown to != 0 so when loop goes, it will call moveLeftPong also since there is a feature that
     * skips calling that function if the user doesn't hit an arrow, ie: upDown = 0*/
    upDown = 1;

  }

}

void pongMove(int movement)
{

  if(movement == 1) //up arrow
  {
    moveLeftPong(movement);

  }else if(movement == -1) //down arrow
  {
    moveLeftPong(movement);

  }

}

void pongRun()
{
  int firstTime, secondTime, totalTime;

  while(pongPlay < 2)
  //~ while(1)
  {
    //~ c++;
    //~
    //~ if(c % 5 == 0 && c!= 0)
    //~ {
    firstTime = getSystemUpTime();

    keyboardInput_handler();
      //~ c = 0;
    //~ }


    // /* Read from the keyboard's data buffer */
    //scancode = inb(0x60);

    //speciaKeyNumber = isSpecialKey(scancode);

    //if(speciaKeyNumber == 3) //up
    //{
      //upDown++;
    //}else if(speciaKeyNumber == 4) //down
    //{
      //upDown--;
    //}

    movePongBall();

    if(upDown != 0)
    {
      moveLeftPong();
    }

    secondTime = getSystemUpTime();
    totalTime = secondTime - firstTime;


    mSleep(20 - totalTime);

  }
}

//void pongRestart()
//{
  //pongPlay = 0;
  //pongRun();

  //arrayOfObjects[1].x = 15; //Left Pong
  //arrayOfObjects[1].y = 70;
  //arrayOfObjects[2].x = 305; //Right Pong
  //arrayOfObjects[2].y = 70;
  //arrayOfObjects[3].x = 150; //Ball
  //arrayOfObjects[3].y = 100;

  ////~ directionLR = 1;
  ////~ directionUD = 1;
  ////~ xCoord = 1;
  ////~ yCoord = 1;

  //putRect(arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);
  //putRect(arrayOfObjects[2].x, arrayOfObjects[2].y, arrayOfObjects[2].width, arrayOfObjects[2].height, arrayOfObjects[2].color);
  //putRect(arrayOfObjects[3].x, arrayOfObjects[3].y, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);

  //refreshScreen(&arrayOfObjects[0], numberOfObjects);
  ////~ refreshObjects(&arrayOfObjects[0], numberOfObjects, 3, 0, 0);

  //pongPlay = 1;
  //pongRun();
//}

void movePongBall()
{
  static double directionLR = 1; //Left right direction (1 is up, -1 is down)
  static double directionUD = 1; //up down direction (1 is left, -1 is right)

  static int xCoord = 150; //default x for ball
  static int yCoord = 100; //default y for ball

  //~ xCoord = xCoord - 5 * directionLR;
  //~ yCoord = yCoord - 5 * directionUD;
  int speed = 3;

  xCoord = xCoord - speed * directionLR;
  yCoord = yCoord - speed * directionUD;

  /*If the ball hits the left or right of the screen*/
  if(xCoord <= 0) //if the ball hits the left side
  {
    //~ pongRestart();

    /*restore defaults*/
    directionLR = 1;
    directionUD = 1;

    xCoord = 150; //default x for ball
    yCoord = 100; //default y for ball
//~
    //~ refreshScreen(&arrayOfObjects[0], numberOfObjects);

    int repeat;
    for(repeat = 0; repeat < 11; repeat++)
    {
    playNote("D4", 10);
    //~ mSleep(10);
    playNote("E4", 10);

    }

    pongPlay = OFF; /** sets the pongPlay to OFF which is used to signal that the ball has hit the left edge*/

    //~ directionLR = -1;
  }else if(xCoord >= VGA_width - (arrayOfObjects[3].height + 2)) //height being the height of the ball
  {
    directionLR = 1;

  }

  /*If the ball hits the top or bottom of the screen*/
  if(yCoord <= 0)
  {
    directionUD = -1;
    playNote("D4", 10);

  }else if(yCoord >= VGA_height - (arrayOfObjects[3].width + 2)) //width being the width of the ball
  {
    directionUD = 1;
    playNote("D4", 10);

  }

  /**Collision detection segement*/
  if(objectsCollision(&arrayOfObjects[0], 1, 3, speed, speed) == TRUE) //if object 1 -left paddle and 3 - ball collide
  {
    directionLR = -1; //make ball go right
    playNote("C5", 10);
  }else if(objectsCollision(&arrayOfObjects[0], 2, 3, speed, speed) == TRUE) //if object 2 -right paddle and 3 - ball collide
  {
    directionLR = 1; //make ball go left
    playNote("C5", 10);

  }

  putRect(xCoord, yCoord, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);
  //~ putRect(xCoord, arrayOfObjects[3].y, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);

  arrayOfObjects[3].x = xCoord;
  arrayOfObjects[3].y = yCoord;

  refreshObjects(&arrayOfObjects[0], numberOfObjects, 3, 6, 6, 0);
  //~ refreshScreen(&arrayOfObjects[0], 4);

  //if(objectsCollision(&arrayOfObjects[0], 4, 3, speed, speed) == TRUE) //if object 4 -center line and 3 - ball collide
  //{
    //refreshObjects(&arrayOfObjects[0], numberOfObjects, 4, speed, 0); //refresh 4th object, center line if ball passes over it
    ////~ refreshObjects(&arrayOfObjects[0], numberOfObjects, 3, speed, speed); //refresh 4th object, center line if ball passes over it
  //}

  /*Computer AI -ish*/
  moveRightPong();
}

void flagUpDown(int movement)
{
  upDown = upDown + movement;
  //~ k_printf("\nKEY\n");

  moveLeftPong();

}

void program_GUI_pong(char *arguements)
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  /*initialize the objects attributes*/
  arrayOfObjects[0].x = 0; //Background
  arrayOfObjects[0].y = 0;
  //~ arrayOfObjects[0].width = VGA_width;
  arrayOfObjects[0].width = 320;
  //~ arrayOfObjects[0].height = VGA_height;
  arrayOfObjects[0].height = 200;
  //~ arrayOfObjects[0].color = 0xffffff;
  arrayOfObjects[0].color = 15;
  arrayOfObjects[0].priority = 0;

  arrayOfObjects[1].x = 15; //Left paddle
  arrayOfObjects[1].y = 70;
  //~ arrayOfObjects[1].width = 5;
  arrayOfObjects[1].width = 2;
  //~ arrayOfObjects[1].height = 30;
  arrayOfObjects[1].height = 15;
  //~ arrayOfObjects[1].color = 0x000000;
  arrayOfObjects[1].color = 0;
  //~ arrayOfObjects[1].priority = 2;
  arrayOfObjects[1].priority = 3;

  //~ arrayOfObjects[2].x = VGA_width - 15; //Right paddle
  arrayOfObjects[2].x = 305; //Right paddle
  arrayOfObjects[2].y = 70;
  //~ arrayOfObjects[2].width = 5;
  arrayOfObjects[2].width = 2;
  //~ arrayOfObjects[2].height = 30;
  arrayOfObjects[2].height = 15;
  //~ arrayOfObjects[2].color = 0x000000;
  arrayOfObjects[2].color = 0;
  //~ arrayOfObjects[2].priority = 2;
  arrayOfObjects[2].priority = 3;

  arrayOfObjects[3].x = 150; //Ball
  arrayOfObjects[3].y = 100;
  arrayOfObjects[3].width = 3;
  arrayOfObjects[3].height = 3;
  //~ arrayOfObjects[3].color = 0xff0000;
  arrayOfObjects[3].color = 4;
  //~ arrayOfObjects[3].priority = 1;
  arrayOfObjects[3].priority = 2;

  arrayOfObjects[4].x = 160; //Line in the middle
  arrayOfObjects[4].y = 0;
  arrayOfObjects[4].width = 1;
  //~ arrayOfObjects[4].width = 1;
  arrayOfObjects[4].height = 200;
  //~ arrayOfObjects[4].color = 0xffff55;
  arrayOfObjects[4].color = 14;
  arrayOfObjects[4].priority = 1;

  if(k_strcmp(arguements, "-gui") == 0)
  {
    VGA_init(320, 200, 256); //initialize the gui

    //~ int rightPongY = 70, leftPongY = 70, ballX, ballY;

    //Nomenclature, {top left x, top left y, width, height, color}
    //~ int objects[4][5]= //make array of the 5 attributes of the 4 objects
    //~ {
      //~ {0, 0, 320, 200, 15}, //white background
      //~ {15, leftPongY, 5, 30, 0}, //left pong
      //~ {305, rightPongY, 5, 30, 0}, //right pong
      //~ {ballX, ballY, 3, 3, 4}, //ball
    //~ };

    //~ putRect(
    putRect(arrayOfObjects[0].x, arrayOfObjects[0].y, arrayOfObjects[0].width, arrayOfObjects[0].height, arrayOfObjects[0].color);
    putRect(arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);
    putRect(arrayOfObjects[2].x, arrayOfObjects[2].y, arrayOfObjects[2].width, arrayOfObjects[2].height, arrayOfObjects[2].color);
    putRect(arrayOfObjects[3].x, arrayOfObjects[3].y, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);
    putRect(arrayOfObjects[4].x, arrayOfObjects[4].y, arrayOfObjects[4].width, arrayOfObjects[4].height, arrayOfObjects[4].color);

    //~ refreshObjects(&arrayOfObjects[0], numberOfObjects, 0, 0, 0, 0); //refresh 4th object, center line if ball passes over it

    //~ while(1);
    //~ refreshScreen(&arrayOfObjects[0], numberOfObjects);
    //~ int i;
    //~
    //~ for(i = 0; i < VGA_height + 1; i++)
    //~ {
      //~ putPixel(VGA_width / 2, i, 14);
    //~ }
    //~ putRect(arrayOfObjects[1][0], arrayOfObjects[1][1], arrayOfObjects[1][2], arrayOfObjects[1][3], arrayOfObjects[1][4]);
    //~ putRect(arrayOfObjects[2][0], arrayOfObjects[2][1], arrayOfObjects[2][2], arrayOfObjects[2][3], arrayOfObjects[2][4]);

    //~ mSleep(5000);
    //~ k_printf("\nRUNNING\n");

    //~ arrowKeyFunction("write", "up", &pongMove); //sets left and write to use a different function
    arrowKeyFunction("write", "up", &flagUpDown); //sets left and write to use a different function


    /*sleeps 1 second (1000 milliseconds) so player can get ready*/
    mSleep(1000);

    /*starts pong Main loop*/
    pongRun();


  }

}
