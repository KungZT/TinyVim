#include <unistd.h>
#include "enableRawMode.h"
#include "disableRawMode.h"

int main(){
    enableRawMode();
    char c;
    while (read(STDIN_FILENO , &c, 1)==1 && c != 'q');
    return 0;
}