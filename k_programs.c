
#include "k_programs.h"
#include "k_stdio.h"
#include "k_shell.h"

#include "keyboard.h"
#include "timer.h"
#include "isr.h"

#include "graphics.h"


extern cursor_x, cursor_y, globalFreq;

/*screen attributes*/
extern VGA_width, VGA_height;

/*To add a new program, add new case statement in function "runShellFunction" in k_programs.c
 * add the program name that the user will have to type in "char *programsList[]" in k_shell.c
 * make the function that will be run in the case stement below, nomenclature-> program_<command name>*/

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
  k_printf(arguements);
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


/*--------------------PONG--------------------------------------------*/

//~ typedef struct{
  //~ int x;
  //~ int y;
  //~ int width;
  //~ int height;
  //~ int color;
  //~ int priority;
  //~ 
//~ }objects;

//~ int objects[4][5]= //make array of the 5 attributes of the 4 objects
//~ {
  //~ {0, 0, 320, 200, 15}, //white background
  //~ {15, 70, 5, 30, 0}, //left pong
  //~ {305, 70, 5, 30, 0}, //right pong
  //~ {160, 100, 3, 3, 4}, //ball
//~ };
//~ objects arrayOfObjects[5]; //make array of the 5 attributes of the 4 objects
int numberOfObjects = 5;
objects arrayOfObjects[5]; //make array of the 5 attributes of the 4 objects

int upDown = 0;

//~ static double directionLR = 1; //Left right direction (1 is up, -1 is down)
//~ static double directionUD = 1; //up down direction (1 is left, -1 is right)
  //~ 
//~ static int xCoord = 150; //default x for ball
//~ static int yCoord = 100; //default y for ball
  
void moveLeftPong()
//~ void moveLeftPong(int movement)
{
  static int yCoord = 70;

  int speed = 2;

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

  refreshObjects(&arrayOfObjects[0], numberOfObjects, 1, 1, speed + 3); //refresh 1rd object, left paddle

  upDown = 0;

}

void moveRightPong()
{
  //COMPUTER AI -ish
  
  static int yCoord = 70;

  int tmpY;

  //~ yCoord = yCoord - 5 * movement;
  //~ yCoord = yCoord - 5 * upDown;
  tmpY = arrayOfObjects[3].y - 15;

  if(tmpY < 0)
  {
    yCoord = 0;
  }else if(tmpY + arrayOfObjects[2].height > VGA_height)
  {
    yCoord = VGA_height - arrayOfObjects[2].height;
  }else{
    yCoord = arrayOfObjects[3].y - 15;
  }
  
  putRect(arrayOfObjects[2].x, yCoord, arrayOfObjects[2].width, arrayOfObjects[2].height, arrayOfObjects[2].color);

  arrayOfObjects[2].y = yCoord;

  refreshObjects(&arrayOfObjects[0], numberOfObjects, 2, 1, 6); //refresh 2rd object, right paddle

  //~ upDown = 0;

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

void pongRestart()
{
  arrayOfObjects[1].x = 15; //Left Pong
  arrayOfObjects[1].y = 70;
  arrayOfObjects[2].x = 305; //Right Pong
  arrayOfObjects[2].y = 70;
  arrayOfObjects[3].x = 150; //Ball
  arrayOfObjects[3].y = 100;

  //~ directionLR = 1;
  //~ directionUD = 1;
  //~ xCoord = 1;
  //~ yCoord = 1;

  putRect(arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);
  putRect(arrayOfObjects[2].x, arrayOfObjects[2].y, arrayOfObjects[2].width, arrayOfObjects[2].height, arrayOfObjects[2].color);
  putRect(arrayOfObjects[3].x, arrayOfObjects[3].y, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);

  refreshScreen(&arrayOfObjects[0], numberOfObjects);
  //~ refreshObjects(&arrayOfObjects[0], numberOfObjects, 3, 0, 0);
  
}

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
  if(xCoord <= 0)
  {
    //~ pongRestart();
    //~ directionLR = 1;
    directionLR = -1;
  }else if(xCoord >= VGA_width - (arrayOfObjects[3].height + 2)) //height being the height of the ball
  {
    directionLR = 1;
  }

  /*If the ball hits the top or bottom of the screen*/
  if(yCoord <= 0)
  {
    directionUD = -1;
  }else if(yCoord >= VGA_height - (arrayOfObjects[3].width + 2)) //width being the width of the ball
  {
    directionUD = 1;
  }

  /*Collision detection segement*/
  if(objectsCollision(&arrayOfObjects[0], 1, 3, speed, speed) == TRUE) //if object 1 -left paddle and 3 - ball collide
  {
    directionLR = -1; //make ball go right
  }else if(objectsCollision(&arrayOfObjects[0], 2, 3, speed, speed) == TRUE) //if object 2 -right paddle and 3 - ball collide
  {
    directionLR = 1; //make ball go left
  }
  
  putRect(xCoord, yCoord, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);
  //~ putRect(xCoord, arrayOfObjects[3].y, arrayOfObjects[3].width, arrayOfObjects[3].height, arrayOfObjects[3].color);

  arrayOfObjects[3].x = xCoord;
  arrayOfObjects[3].y = yCoord;

  refreshObjects(&arrayOfObjects[0], numberOfObjects, 3, 6, 6);
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
  arrayOfObjects[0].color = 15;
  arrayOfObjects[0].priority = 0;
  
  arrayOfObjects[1].x = 15; //Left paddle
  arrayOfObjects[1].y = 70;
  arrayOfObjects[1].width = 5;
  arrayOfObjects[1].height = 30;
  arrayOfObjects[1].color = 0;
  //~ arrayOfObjects[1].priority = 2;
  arrayOfObjects[1].priority = 3;
  
  //~ arrayOfObjects[2].x = VGA_width - 15; //Right paddle
  arrayOfObjects[2].x = 305; //Right paddle
  arrayOfObjects[2].y = 70;
  arrayOfObjects[2].width = 5;
  arrayOfObjects[2].height = 30;
  arrayOfObjects[2].color = 0;
  //~ arrayOfObjects[2].priority = 2;
  arrayOfObjects[2].priority = 3;
  
  arrayOfObjects[3].x = 150; //Ball
  arrayOfObjects[3].y = 100;
  arrayOfObjects[3].width = 3;
  arrayOfObjects[3].height = 3;
  arrayOfObjects[3].color = 4;
  //~ arrayOfObjects[3].priority = 1;
  arrayOfObjects[3].priority = 2;
  
  arrayOfObjects[4].x = 160; //Line in the middle
  arrayOfObjects[4].y = 0;
  arrayOfObjects[4].width = 0;
  //~ arrayOfObjects[4].width = 1;
  arrayOfObjects[4].height = 200;
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

    refreshObjects(&arrayOfObjects[0], numberOfObjects, 4, 2, 0); //refresh 4th object, center line if ball passes over it

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

    //long long int time, secondTime, runTime;

    while(1)
    {
      keyboardInput_handler();
      
      movePongBall();
      moveLeftPong();

 
      mSleep(20);
      
    }

  }
  
}
