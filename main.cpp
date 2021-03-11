#include <iostream>
#include <termios.h>
#include <unistd.h>
using namespace std;

/*** memo ***/
// canonical mode - line by line
// noncanonical mode - byte by byte

/*** data ***/
struct termios orig_termios;

/*** terminal ***/
void die(const char *s){
    perror(s);
    exit(1);
}

void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode); // execute disableRawMode when program exits

    struct termios raw = orig_termios;
    // c_iflag - input modes
    // IXON - XON/XOFF flow controll on output
    // ICRNL - translate carriage return to newline on input
    // BREKINT - break condition cause a SIGINT signal to be sent to the program
    // INPCK - input parity checking
    // ISTRIP - strip off 8th bit
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
    // c_lflag - local modes
    // ISIG - turn off the signals ex. ctrl-c, ctrl-z
    // meaning - ctrl-c is read as 3, not terminated
    // ICANON - canonical mode
    // ECHO - echo input character
    // IEXTEN - turn off ctrl-v, ctrl-o
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    // c_oflag - output modes
    // OPOST - output processing
    raw.c_oflag &= ~(OPOST);
    // c_cflag - control modes
    // CS8 - character size to be 8 bits per byte
    raw.c_cflag |= (CS8);

    // minimum number of characters for noncanonical read
    // 0 meaning read() returns as soon as any input is read
    raw.c_cc[VMIN] = 0;
    // timeout in deciseconds for noncanonical read
    // 1 = 100 millseconds
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/// init ///
int main() {
    enableRawMode();

    while(1){
        char c = '\0';
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if(iscntrl(c)){
            printf("%d\r\n", c);
        }else{
            printf("%d ('%c')\r\n", c, c);
        }
        if(c == 'q') break;
    }
    return 0;
}
