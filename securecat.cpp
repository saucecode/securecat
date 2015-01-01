#include <ncurses.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <sstream>

#include "rc4.h"
#include "dyad.h"

char* chatLog[32];
//WINDOW* catWindow;
WINDOW* chatWindow;
dyad_Stream* client;
char* username;
char* password;

short strtoshort(char* x);
WINDOW* create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void onData(dyad_Event* event);

void printLog(){
	wclear(chatWindow);
	wmove(chatWindow,1,2);
	for(int i=31; i>=0; i--){
		wprintw(chatWindow, chatLog[i]);
		wprintw(chatWindow, "\n");
	}
	
	wrefresh(chatWindow);
}

void writeToScreen(char* x){
	for(int i=31; i>0; i--){
		memset(chatLog[i], '\0', 256);
		memcpy(chatLog[i], chatLog[i-1], strlen(chatLog[i-1]));
	}
	memset(chatLog[0], '\0', 256);
	memcpy(chatLog[0], x, strlen(x));
	//memset(x, '\0', strlen(x));
}

void sendToServer(char* x){
	char* message = (char*) calloc(512,1);
	memset(message, '<', 1);
	memcpy(message+1, username, strlen(username));
	memset(message+strlen(username)+1, '>', 1);
	memset(message+strlen(username)+2, ' ', 1);
	memcpy(message+strlen(username)+3, x, strlen(x));
	dyad_write(client, (char*) rc4((void*) message, strlen(message), password, strlen(password)), strlen(message));
	memset(x, '\0', strlen(x));
}

bool connectToServer(char* serverIP, char* serverPort){
	dyad_init();
	client = dyad_newStream();
	dyad_setUpdateTimeout(0);
	dyad_addListener(client, DYAD_EVENT_DATA, onData, NULL);
	int err = dyad_connect(client, serverIP, strtoshort(serverPort));
	return err == 0;
}

void onData(dyad_Event* event){
	//char* data = (char*) calloc(256, 1);
	char* data = (char*) rc4((void*) event->data, event->size, password, strlen(password));
	writeToScreen(data);
}

int main(){
	char* currentLine = (char*) calloc(256,1);
	username = (char*) calloc(256, 1);
	char* serverIP = (char*) calloc(16, 1);
	char* serverPort = (char*) calloc(16, 1);
	password = (char*) calloc(512, 1);
	
	char* inputBuffer = (char*) calloc(256, 1);
	for(int i=0; i<32; i++) chatLog[i] = (char*) calloc(256, 1);
	
	initscr();
	printw("Enter username: ");
	scanw("%s", username);
	printw("Enter server: ");
	scanw("%s", serverIP);
	printw("Enter port: ");
	scanw("%s", serverPort);
	
	
	printw("Enter password: ");
	noecho();
	getstr(password);
	clear();
	printw("Connecting to server at ");
	printw(serverIP);
	printw(":");
	printw(serverPort);
	printw("...\n");
	bool connected = connectToServer(serverIP, serverPort);
	if(!connected){
		printw("\nERROR: Failed to connect.\n(Press any key to close program.)");
		refresh();
		getch();
		endwin();
		return 1;
	}else{
		endwin();
	}
	
	//printf("%s\n", password);
	
	//std::thread networkingThread(start_networking, username, serverIP, serverPort, password);
	//std::thread testThread(test);
	
	initscr();
	clear();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	refresh();
	
	WINDOW* topWindow = create_newwin(3,COLS,0,0);
	//catWindow = create_newwin(LINES-6,COLS,3,0);
	chatWindow = create_newwin(LINES-6,COLS,3,0);
	WINDOW* inputOuter = create_newwin(3,COLS,LINES-3,0);
	WINDOW* inputWindow = subwin(inputOuter, 1, COLS-2, LINES-2, 1);
	scrollok(chatWindow, TRUE);
	scrollok(inputWindow, TRUE);
	
	wmove(topWindow, 1,1);
	wprintw(topWindow, serverIP);
	wprintw(topWindow, ":");
	wprintw(topWindow, serverPort);
	wprintw(topWindow, " -- secure cat client -- ");
	wprintw(topWindow, username);
	wrefresh(topWindow);
	
	while(1){
		int ch = getch();
		mvwprintw(inputWindow, 0, 0, currentLine);
		wclrtoeol(inputWindow);

		if(ch != ERR){
			if((ch == 127 || ch == 8) && strlen(currentLine) != 0){
				memset(currentLine + strlen(currentLine)-1, '\0', 1);
			}else if(ch == '\n'){
				memcpy(inputBuffer, currentLine, strlen(currentLine));
				memset(currentLine, '\0', 256);
			}else if(strlen(currentLine) <= 255){
				memset(currentLine + strlen(currentLine), ch, 1);
			}
		}
		printLog();
		wrefresh(inputWindow);
		//refresh();
		
		if(strlen(inputBuffer) != 0){
			//writeToScreen(inputBuffer);
			sendToServer(inputBuffer);
		}
		
		dyad_update();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	getch();
	endwin();
	dyad_shutdown();
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}

short strtoshort(char* x){
	short y;
	std::stringstream ss(x);
	ss >> y;
	return y;
}
