
#ifndef PROGRAMS
#define PROGRAMS

/*function that gets input index and runs corresponding function*/
void runShellFunction(int runFunction, char *arguements);

/*program list starts here*/
void program_ascii(char *arguements); //0 index --prints ascii animals

void program_echo(char *arguements); //1 index --prints the arg put in

void program_tinytext(char *arguements); //2 index --very simple text editor

//PONG//
void program_GUI_pong(char *arguements); //3 index --play pong
void movePongBall();
void pongRestart();
//PONG//

void program_song(char *arguements); //4 index --plays some small tunes

void program_JS_viewer(char *arguements); //5 index --a simple image viewer

void program_start(char *arguements); //6 index --starts programs (like start X)

#endif

