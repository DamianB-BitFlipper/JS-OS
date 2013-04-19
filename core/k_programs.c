/*
 * k_programs.c
 * 
 * Copyright 2013 JS <js@duck-squirell>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <system.h>

//The currently running task.
extern volatile task_t *current_task;

extern cursor_x, cursor_y, globalFreq;

/*screen attributes*/
extern VGA_width, VGA_height;
extern char *path; //the char array that contains the path from the root to the current directory

extern u32int fs_location;
extern u32int nroot_nodes;

/**To add a new program, add new case statement in function "runShellFunction" in k_programs.c
 * add the program name that the user will have to type in "char *programsList[]" in k_shell.c
 * add initial run function of that program into k_programs.h
 * make the function that will be run in the case stement below, nomenclature-> program_<command name> **/

void runShellFunction(int runFunction, char *arguements, u32int priority, u32int burst_time)
{

  switch(runFunction)
  {
    case 0:
      //~ start_task(priority, program_ascii, arguements);
      program_ascii(arguements);
      break;
    case 1:
      //~ start_task(priority, burst_time, program_echo, arguements, "echo");

      //turn the shell indent off so it does not print an indent before echo prints
      //~ turnShellIndentOnOff(OFF);
      start_task(priority, burst_time, program_echo, arguements, "echo");
      //~ program_echo(arguements);
      break;
    case 2:
      program_tinytext(arguements);
      break;
    case 3:
      program_GUI_pong(arguements);
      break;
    case 4:
      //multitasking enabled program
      start_task(priority, burst_time, program_song, arguements, "song");
      break;
    case 5:
      program_JS_viewer(arguements);
      break;
    case 6:
      program_start(arguements);
      //~ start_task(priority, burst_time, program_start, arguements, "start");
      
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
    case 17:
      program_find(arguements);
      break;
  }

}

void program_ascii(char *arguements)
{

  if(k_strcmp(arguements, "-h") == 0 || k_strcmp(arguements, "-help") == 0 || k_strlen(arguements) == 0)
  {
    k_printf("Usage: ascii -<animal>\nType ascii -list to see list of animals");

  }else if(k_strcmp(arguements, "-list") == 0)
  {
    /*The list of animals*/
    k_printf("Animals:\n\tcow\n\tduck\n\tsquirrel or skverl\n\tlittle_skverl");

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
    /*The skverl*/
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

  }else if(!k_strcmp(arguements, "-little_skverl"))
  {
    /*The little_skverl*/
   
    k_printf("\t  .  _ |  |  _\n");
    k_printf("\t  )\\|';| @| //\n");
    k_printf("\t / (/ \\=  |//\n");
    k_printf("\t \\__:_.=   /\n");
    k_printf("\t       |  |    ");
  }else{
    k_printf("Arguement: %s not found\n", arguements);
    k_printf("Usage: ascii -<animal>\nType ascii -list to see list of animals");
  }

  //~ k_printf("\n%d\n", k_strcmp(arguements, "-duck"));
}

void program_echo(char *arguements)
{

  //if there is an arg tied to the current_task, set it to arguements
  asm volatile("cli");
  if(current_task->thread_flags)
  {
    arguements = (char*)current_task->thread_flags;
  }
  asm volatile("sti");

  k_printf("%s\n", arguements);

  //delete the process from the multitasking
  //~ exit();  
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

  //if there is an arg tied to the current_task, set it to arguements
  asm volatile("cli");
  if(current_task->thread_flags)
  {

    arguements = (char*)current_task->thread_flags;
  }
  asm volatile("sti");
  
  if(k_strcmp(arguements, "-pacman") == 0)
  {
    song_pacman();
  }

  //delete the process from the multitasking queue
  exit();
}

///**************************LS function*****************************///

struct ls_file_header
{
  char *name;
  u32int inode;
};

struct ls_file_header *ls_files;

static int ls_nfiles; //length of block that *files points to
static int ls_files_indexed; //the number of files to print

static int all_files; //for -a flag
static int really_all_files; //for -A flag
static int ls_print_inode;

enum format
{
  long_format,		/* -l and other options that imply -l */
  one_per_line,		/* -1 */
  many_per_line,	/* -C */
  horizontal,			/* -x */
  with_commas			/* -m */
};

static enum format format;

enum sort_type
{
  sort_none,			 /* -U */
  sort_name,			 /* default */
  sort_extension,	 /* -X */
  sort_time,			 /* -t */
  sort_size,			 /* -S */
  sort_version		 /* -v */
};

static enum sort_type sort_type;

/*returns TRUE (1) if file should be listed*/
static int ls_file_interesting(struct dirent *entry)
{

  if(really_all_files == TRUE || entry->name[0] != '.' ||
  (all_files == TRUE && entry->name[1] != 0 && (entry->name[1] != '.' || entry->name[2] != 0)))
  {
    return TRUE;
  }

  return FALSE;
}

void ls_extra_print(u32int inode)
{
  //test if user wants to print the inode
  if(ls_print_inode == TRUE)
  {
    k_printf("%d ", inode);
  }
}

void ls_print_permissions(u32int inode)
{
  int p, binPer = 0b100000000;
  
  for(p = 0; p < 9; p++) //loop 9 times to do rwx for user, group, and other
  {
    switch(p % 3) //case for every three, itterate between r, w, and, x
    {
      case 0:
        if((root_nodes[inode].mask & binPer) != 0) //there is a 1 (TRUE) for the specific location
        {
          k_printf("r");
        }else{
          k_printf("-"); //if the location is 0 (FALSE), just have a slash
        }
        break;
      case 1:
        if((root_nodes[inode].mask & binPer) != 0) //there is a 1 (TRUE) for the specific location
        {
          k_printf("w");
        }else{
          k_printf("-"); //if the location is 0 (FALSE), just have a slash
        }
        break;        
      case 2:
        if((root_nodes[inode].mask & binPer) != 0) //there is a 1 (TRUE) for the specific location
        {
          k_printf("x");
        }else{
          k_printf("-"); //if the location is 0 (FALSE), just have a slash
        }
        break;
    }

    binPer /= 2;
  }

  k_printf(" ");
}

void ls_sort_name()
{
  //TODO make a sort function for the file names to print


  //int i = 0, a;
  //u32int address;
  //u8int one, two;
  ////~ for(i = 0; i < ls_files_indexed - 1; i++)
  ////~ {
    //for(a = 0; a < (ls_files_indexed - 1) * (ls_files_indexed - 1); a++)
    //{
      //one = *(ls_files[a].name + i);
      //two = *(ls_files[a + 1].name + i);

      //if(one > two)
      //{
        //address = ls_files[a + 1].name
        //array[a] = two;
        //array[a + 1] = one;
      //}
    //}
  ////~ }
}

void ls_sort_files()
{

  switch(sort_type)
  {
    case sort_none:
      //do not sort, do nothing and exit
      return;
    case sort_name:
      ls_sort_name();
      break;
    default:
      //error!
      return;
  }

}

void ls_print_one_per_line()
{
  int i;
  for(i = 0; i < ls_files_indexed; i++)
  {

    if ((root_nodes[ls_files[i].inode].flags & 0b111) == FS_DIRECTORY)
    {
      ls_extra_print(ls_files[i].inode);
      k_printf(LS_DIR_COLOR); //set the k_printf color to the LS_DIR_COLOR
      k_printf(ls_files[i].name);
      k_printf(" (dir)\n");
      k_printf(LS_DEFAULT_COLOR); //revert the k_printf color back to default

    }else{
      ls_extra_print(ls_files[i].inode);      
      k_printf(ls_files[i].name);
      k_printf("\n");
    }

  }
}

void ls_print_with_commas()
{
  int i;

  //print all files, but the last
  for(i = 0; i < ls_files_indexed - 1; i++)
  {

    if ((root_nodes[ls_files[i].inode].flags & 0b111) == FS_DIRECTORY)
    {
      ls_extra_print(ls_files[i].inode);      
      k_printf(LS_DIR_COLOR); //set the k_printf color to the LS_DIR_COLOR
      k_printf(ls_files[i].name);
      k_printf(" (dir), ");
      k_printf(LS_DEFAULT_COLOR); //revert the k_printf color back to default

    }else{
      ls_extra_print(ls_files[i].inode);      
      k_printf(ls_files[i].name);
      k_printf(", ");
    }

  }

  //still, if the last file is a directory, print (dir) at the end
  if ((root_nodes[ls_files[i].inode].flags & 0b111) == FS_DIRECTORY)
  {
    ls_extra_print(ls_files[i].inode);
        
    k_printf(LS_DIR_COLOR); //set the k_printf color to the LS_DIR_COLOR
    //print the last without a comma and space
    k_printf(ls_files[i].name);
    k_printf(" (dir)");
    k_printf(LS_DEFAULT_COLOR); //revert the k_printf color back to default

  }else{
    ls_extra_print(ls_files[i].inode);      
    
    //print the last without a comma and space
    k_printf(ls_files[i].name);
  }
}

void ls_long_format()
{
  int i;

  //print all files, but the last
  for(i = 0; i < ls_files_indexed; i++)
  {

    if ((root_nodes[ls_files[i].inode].flags & 0b111) == FS_DIRECTORY)
    {
      ls_extra_print(ls_files[i].inode);

      k_printf("d"); //print "d" to indicate in permisions that it is a directory

      ls_print_permissions(ls_files[i].inode);
      
      k_printf(LS_DIR_COLOR); //set the k_printf color to the LS_DIR_COLOR
      k_printf(ls_files[i].name);
      k_printf(" (dir)\n");
      k_printf(LS_DEFAULT_COLOR); //revert the k_printf color back to default

    }else{
      ls_extra_print(ls_files[i].inode);

      k_printf("-"); //print "-" to indicate in permisions that it is not a directory

      ls_print_permissions(ls_files[i].inode);
          
      k_printf(ls_files[i].name);
      k_printf("\n");
    }

  }
}

void ls_print_format()
{

  switch(format)
  {
    case one_per_line:
      ls_print_one_per_line();
      break;
    case with_commas:
      ls_print_with_commas();
      break;
    case long_format:
      ls_long_format();
      break;
    default:
      //error!
      return;
  }

}

/*adds a file to the *ls_files block so that file can be printed*/
void ls_gobble_file(struct dirent *entry)
{
  if(ls_files_indexed == ls_nfiles) //we need more space in *ls_files!
  {
    ls_nfiles *= 2;

    //TODO: use realloc instead of this makeshift reallocation
    int newLoc = kmalloc(ls_nfiles * sizeof(struct ls_file_header));

    memcpy(newLoc, ls_files, ls_files_indexed * sizeof(struct ls_file_header));

    kfree(ls_files);
    ls_files = (struct ls_file_header*)newLoc;
  }

  //copy file name into ls_files block entry
  ls_files[ls_files_indexed].name = (char*)kmalloc(entry->name_len + 1); //+1 is for \000 at the end of the name
  memcpy(ls_files[ls_files_indexed].name, entry->name, entry->name_len);

  *(ls_files[ls_files_indexed].name + entry->name_len) = 0; //add \000 to the end

  //add the inode of the file to the entry
  ls_files[ls_files_indexed].inode = entry->ino;

  ls_files_indexed++;
}

static int ls_decode_flags(int nArgs, char **args)
{
  ///* PROCESS THE ARGUMENT FLAGS *///

  int flagRep = 0, i;
  while((i = getopt(flagRep, nArgs, args, "ailmAU")) != -1)
  {
    switch(i)
    {
      case 0:
        //do nothing
        break;
      case 'a': //flag 'a' all
        all_files = TRUE;
        really_all_files = TRUE;
        break;
      case 'i': //flag 'a' all
        ls_print_inode = TRUE;
        break;
      case 'l': //flag 'l' lengthy
        format = long_format;
        break;
      case 'm': //flag 'l' lengthy
        format = with_commas;
        break;
      case 'A': //flag 'A' almost all
        all_files = TRUE;
        really_all_files = FALSE;
        break;
      case 'U': //flag 'A' almost all
        sort_type = sort_none;
        break;
      default: //a legal flag was not found, so print and error and exit
        //error!
        return 1;
    }

    flagRep++;
  }
  ///* PROCESS THE ARGUMENT FLAGS *///

  //success!
  return 0;
}

void program_ls(char *arguements)
{
  //*reset all flag characteristics*//
  all_files = FALSE;
  really_all_files = FALSE;
  ls_print_inode = FALSE;
  format = one_per_line;
  sort_type = sort_name;

  //set up block information that stores the names of the files in the directroy to ls
  ls_nfiles = 100; //default size
  ls_files_indexed = 0; //currently there are 0 files to print, we have not parced anything yet
  ls_files = (struct ls_file_header*)kmalloc(ls_nfiles * sizeof(struct ls_file_header));

  //get the current dir Inode before we cd into a different directory
  int currentIno = currentDir_inode;

  //gets the number of args in the char *arguements
  int nArgs = countArgs(arguements);

  //assigns values from the char *arguements to char *arguements[nArgs]
  char *args[nArgs];
  getArgs(arguements, args);

  //gets the arg number in char *arguements[nArgs] that points to the path to ls to
  int dirPathArg;
  for(dirPathArg = 0; dirPathArg < nArgs; dirPathArg++)
  {
    /*if the first char in the arg is not a slash (not a flag but the dirPath itself)
     * or, if the first arg is a slash and there is nothing after it, meaning it could be a directory name */
    if(*(args[dirPathArg]) != '-' || *(args[dirPathArg] + 1) == ' ' || *(args[dirPathArg] + 1) == 0)
    {
      break;
    }
  }

  int work;

  if(dirPathArg == nArgs) //if there was no arg that had no "-" in front of it
  {
    //we do not need to cd and that stuff (there was no directory path input), so keep it simple

    //set work to pass since there was no cd'ing, we assume ls to current dir
    work = 0;
  }else{
    //Gets the dirPath and filePath in the args and cd's to that directory
    int dirCount, fileCount;

    dirFilePathCount(args[dirPathArg], &dirCount, &fileCount);

    char *dirPath, *filePath;

    dirPath = (char*)kmalloc(dirCount + 1);
    filePath = (char*)kmalloc(fileCount + 1);

    //Gets the dirPath and filePath in the args and cd's to that directory
    work = cdFormatArgs(args[dirPathArg], dirPath, filePath);

    //free these two paths as we have no use for them
    kfree(dirPath);
    kfree(filePath);
  }

  ///*PROCESS THE ARGUMENT FLAGS*///
  ls_decode_flags(nArgs, args);

  //there was no error in the cd'ing process
  if(work == 0)
  {
    fs_node_t *fsnode;

    fsnode = &root_nodes[currentDir_inode];
    struct dirent *entry = 0;
    int i = 0;

    while ( (entry = readdir_fs(fsnode, i)) != 0)
    {
      //see if the file name is of any interest
      if(ls_file_interesting(entry) == TRUE)
      {
        ls_gobble_file(entry);
      }

      i++;
    }

    /***Print the files***/
    ls_print_format();
    /***Print the files***/

  }else if(work == 1)
  {
    k_printf("In \"%s\", \"%s\" is not a directory\n", arguements, arguements);
  }

  //free char **args that holds our individual arguments
  int c;
  for(c = 0; c < nArgs; c++)
  {
    kfree(args[c]);
  }

  setCurrentDir(&root_nodes[currentIno]);

  //also free all of the name locations
  for(c = 0; c < ls_files_indexed; c++)
  {
    kfree(ls_files[c].name);
  }
  kfree(ls_files); //frees the block that contained the file names to print

}

///**************************LS function*****************************///

int program_cd(char *arguements)
{
  int argLen = strlen(arguements);
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
      if(*(arguements + i) == '/')
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

    memcpy(cd, (arguements + originalI), (i - originalI));
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
      if(*(cd) != 0) //if the string cd has contents, usually if the char *arguements is "//" for some reason, cd will have no content (begins with a \000)
      {
        k_printf("In \"%s\", \"%s\" is not a directory\n", arguements, cd);
      }
      //~ }else{
        //~ k_printf("In \"%s\", \"%s\" is not a directory\n", arguements, arguements);

      //~ }
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
  int currentIno = currentDir_inode;

  int nArgs = countArgs(arguments);

  char *args[nArgs];
  getArgs(arguments, args);

  int dirCount, fileCount;

  dirFilePathCount(args[0], &dirCount, &fileCount);

  char *dirPath, *filePath;

  dirPath = (char*)kmalloc(dirCount + 1);
  filePath = (char*)kmalloc(fileCount + 1);

  cdFormatArgs(args[0], dirPath, filePath);

  fs_node_t *src;

  src = finddir_fs(&root_nodes[currentDir_inode], filePath); //gets the structure of the first argument input

  if(src != 0) //if there is a file with the name of the first argument
  {
    setCurrentDir(&root_nodes[currentIno]); //sets the current dir back to the original
    
    s32int i = 0, length = strlen(args[1]), count = -1;

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

    char destPath[count + 2]; //the extra +1 is for the \000 and the other 1 is because i, and thus count start from 0 on the first element
    memcpy(destPath, args[1], count + 1);
    *(destPath + count + 1) = 0; //add \000 to the end

    char restPath[length - count];
    memcpy(restPath, args[1] + count + 1, length - count - 1);
    *(restPath + length - count - 1) = 0; //add the \000 to the end

    //By default, we have not cd'd yet, so u32int work should be set to fail (1)
    u32int work = 1;
    
    //if there is something in the destPath
    if(*(destPath) != 0)
    {
      work = program_cd(destPath);
    }

    u32int b;

    fs_node_t *restSrc;
    restSrc = finddir_fs(&root_nodes[currentDir_inode], restPath);

    /*if the rest of the user text after the final "/" is a directory,
     * go to that directory and then create the file */
    if(restSrc != 0 && restSrc->flags == FS_DIRECTORY)
    {
      program_cd(restPath);

      u32int src_block, copied_block;
      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], src->name, src->length);

      for(b = 0; b < (u32int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        src_block = (u32int)block_of_set(src, b);
        copied_block = (u32int)block_of_set(copiedFile, b);

        memcpy(*(u32int*)copied_block, *(u32int*)src_block, BLOCK_SIZE);
      }

    }else if(*restPath != 0) //if there is contents after the last "/", make the file with that name
    {
      u32int src_block, copied_block;
      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], restPath, src->length);

      for(b = 0; b < (u32int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        
        src_block = (u32int)block_of_set(src, b);
        copied_block = (u32int)block_of_set(copiedFile, b);

        memcpy(*(u32int*)copied_block, *(u32int*)src_block, BLOCK_SIZE);
      }


    }else if(work == 0) //if the cd function above did not fail, i.e., we should still copy the file, but with its original name
    {
      u32int src_block, copied_block;      
      fs_node_t *copiedFile;
      copiedFile = createFile(&root_nodes[currentDir_inode], src->name, src->length);

      for(b = 0; b < (u32int)((src->length - 1) / BLOCK_SIZE) + 1; b++)
      {
        src_block = (u32int)block_of_set(src, b);
        copied_block = (u32int)block_of_set(copiedFile, b);

        memcpy(*(u32int*)copied_block, *(u32int*)src_block, BLOCK_SIZE);
      }

    }

    setCurrentDir(&root_nodes[currentIno]); //sets the current dir back to the original

  }

  u32int i;
  for(i = 0; i < nArgs; i++)
  {
    kfree(args[i]);
  }

  kfree(dirPath);
  kfree(filePath);

}

void program_mv(char *arguements)
{
  u32int nArgs = countArgs(arguements);

  char *args[nArgs];
  getArgs(arguements, args);

  program_cp(arguements); //copy the source to the dest
  program_rm(args[0]);
}

void program_find(char *arguments)
{
  u32int initDir = currentDir_inode;

  u32int dirCount, fileCount;

  dirFilePathCount(arguments, &dirCount, &fileCount);

  char *dirPath, *filePath;

  dirPath = (char*)kmalloc(dirCount + 1);
  filePath = (char*)kmalloc(fileCount + 1);

  u32int work = cdFormatArgs(arguments, dirPath, filePath);

  u32int i, argLen = strlen(arguments);
  for(i = argLen - 1; i >= strlen(arguments) - fileCount; i--)
  {
    if(*(arguments + i) == '*') //there is an asterisc, so cd would have not found the file, but still let it pass
    {
      work = 0; //correct error
      /*becuase the arguements file has a "*", in it, most of the time,
       * our cdFormatArgs() function will return 1 (error) since for
       * the most part, there will be no file named (ex: if there is a file
       * name "test.txt" and the user will search "t*", cd will literally
       * search for "t*", not find it and complain, thus the char *filePath
       * will be empty and nothing will be saved to it, we just copy the contents
       * of our file name in the arguments to filePath manually */
      memcpy(filePath, arguments + argLen - fileCount, fileCount);
      break;
    }
  }

  if(work == 0) //there was no error
  {
    fs_node_t *fsnode;

    fsnode = &root_nodes[currentDir_inode];
    struct dirent *node = 0;
    i = 0;

    //~ k_printf("\nFILEPATH %s, %d\n", filePath, fileCount);

    while ( (node = readdir_fs(fsnode, i)) != 0)
    {
      //if the files names are the same (includeing using "*" to represent and characters)
      if(compareFileName(filePath, node->name) == 0)
      {
        k_printf("%s\n", node->name);
      }
      i++;
    }

  }else{
    k_printf("%s: not a proper file path\n", arguments);
  }

  setCurrentDir(&root_nodes[initDir]);

  kfree(dirPath);
  kfree(filePath);
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

    /*the purpose of the following section is to one address with a max
     * size of 1KB (to conserve space) to display any size file
     * We do this by dividing the file into blocks and printing the
     * file's data one block at a time instead of all at once */
    u32int j, a, next_size, kmalloc_size = blockSizeAtIndex(file->length, 0, 0);
    char *buf = (char*)kmalloc(kmalloc_size);

    /* ((u32int)(root_nodes[i].length / BLOCK_SIZE) + 1) calculates the
     * number of blocks this file will take up at a given length */
    for(a = 0; a < ((u32int)(file->length / BLOCK_SIZE) + 1); a++)
    {
      //~ u32int sz = read_fs(file, 0, file->length, buf); //assign the content of file to char *buf
      u32int sz = read_fs(file, a * BLOCK_SIZE, kmalloc_size, buf); //assign the content of file to char *buf
      for (j = 0; j < sz; j++)
      {
        k_putChar(buf[j]);
      }
  
      //if there is a block after this one (we are not at the last block)
      if(a != (u32int)(file->length / BLOCK_SIZE))
      {

        //calculate the size to kmalloc for the next block
        next_size = blockSizeAtIndex(file->length, a + 1, 0);
        
        /*if our current kmalloc'd space is not the same as the next block's,
         * then we have to free the current buf and kmalloc it with the
         * new size, else we do not need to touch as it is the same size */
        if(kmalloc_size != next_size)
        {
          kfree(buf);
          kmalloc_size = next_size;
          buf = (char*)kmalloc(kmalloc_size);
        }
      }else{
        //free the buffer
        kfree(buf);
      }
    }
    
  }else if(file->flags == FS_DIRECTORY)
  {
    k_printf("%s: Is a directory\n", arguments);
  }else{
    k_printf("%s: No such file\n", arguments);
  }

  setCurrentDir(&root_nodes[initDir]);

  kfree(dirPath);
  kfree(filePath);

}

void program_rm(char *arguments)
{
  u32int initialDir = currentDir_inode, length = strlen(arguments);
  s32int a;

  //goes from the back, when it breaks, a == the position of the very last "/"
  for(a = length - 1; a >= 0; a--)
  {
    if(*(arguments + a) == '/')
    {
      break;
    }
  }

  if(a < 0) //if the char *arguments had no "/"
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
      a = -1; //set s32int a to be just right for those kmallocs below
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

    u32int i = 0, b = 0, dir_block;
    fs_node_t *dirNode;
    dirNode = &root_nodes[currentDir_inode];

    //calculate the first dir_block
    dir_block = (u32int)block_of_set(dirNode, b);

    /*this section looks through the direcetory and finds the location
     * of the dirent of the file that we want to remove */
    do
    {
      
      //if we found the file in the directory's contents
      if(*(u32int*)(*(u32int*)dir_block + i) == file->inode)
      {

        dirent2->ino = *(u32int*)(*(u32int*)dir_block + i);
        dirent2->rec_len = *(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino));

        break;

      }else{

        //if the next rec_length is 0, then we are currently at the last dirent, else increase the offset (i)
        if(*(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino)) != 0)
        {
          //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
          i = i + *(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino));
        }else{ //this is the last direct, add 1 to block and reset the offset (i)
          i = 0;
          b++;
          //recalculate the dir_block
          dir_block = (u32int)block_of_set(dirNode, b);
        }

      }

      if(b > (u32int)((dirNode->length - 1) / BLOCK_SIZE))
      {
        //error, block exceds the number of blocks dirNode has
        return;
      }

    }
    while(1);

    /*shifts the dirent data in the directory
     * the "-1 *" is used to show shift to the left */
    shiftData((u32int*)(*(u32int*)dir_block + i + dirent2->rec_len), -1 * dirent2->rec_len, BLOCK_SIZE - i - dirent2->rec_len);

    //Delete the actual file content
    u32int c, block;
    for(c = 0; c < (u32int)((file->length - 1) / BLOCK_SIZE) + 1; c++)
    {
      block = (u32int)block_of_set(file, c);
      
      memset(*(u32int*)block, 0, BLOCK_SIZE);
      kfree((void*)(*(u32int*)block));
      //~ memset(file->blocks[c], 0, BLOCK_SIZE);
      //~ kfree(file->blocks[c]);
    }

    u32int nameLength = strlen(root_nodes[dirent2->ino].name);

    // Delete the file node.
    memset(root_nodes[dirent2->ino].name, 0, nameLength); //delete the name
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
    k_printf("Cannot remove '%s': Is a directory\n", filePath);
  }else{
    k_printf("%s: No such file\n", arguments);
  }

  kfree(dirPath);
  kfree(filePath);

  setCurrentDir(&root_nodes[initialDir]);

}

void program_mkdir(char *arguments)
{
  u32int initDir = currentDir_inode, length = strlen(arguments);

  u32int i;

  /*s32int count would equal the number of characters before and
   * including the last '/' in the char *arguments, if it equals
   * -1 then that means there were no '/' in the char *arguments */
  s32int count = -1;
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
    /* +1 being the \000 at the end, and the other +1 is due to
     * that we start counting from 0, 1, 2, etc. so if count was to equal
     * 0, that would mean 1 character, so we boost count up by one to equal
     * the number of characters */
    dirString = (char*)kmalloc(count + 2);

    // +1 for same reason as above, count starts from 0 for the first character
    memcpy(dirString, arguments, count + 1);
    *(dirString + count + 1) = 0; // add the \000 at the end of the string "dirString"

    u32int work = program_cd(dirString);


    if(work == 0) //if the program_cd() did not return an error
    {
      char *restString;
      restString = (char*)kmalloc(length - count);

      //get the contents of char *arguments after the final '/'
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
  k_printf("\n\tls - lists the contents in the current directory [ls]");
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
  //if there is an arg tied to the current_task, set it to arguements
  //asm volatile("cli");
  //if(current_task->thread_flags)
  //{

    //arguements = (char*)current_task->thread_flags;
  //}
  //asm volatile("sti");
  
  if(k_strcmp(arguements, "x") == 0)
  {
    //~ asm volatile("sti");
    //~ init_mouse(); // Initialise mouse

    xServer();
  }

  //delete the process from the multitasking
  //exit();
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
