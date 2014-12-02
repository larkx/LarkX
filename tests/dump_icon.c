/* 
 * File:   dump_icon.cpp
 * Author: peter
 *
 * Created on 2. Dezember 2014, 17:35
 */
#include <unistd.h>

    const unsigned char bitshares_icon_png[] = {
#include "../libraries/client/icon.c"
    };

/*
 * 
 */
int main(int argc, char** argv) {

    write(STDERR_FILENO, bitshares_icon_png, sizeof(bitshares_icon_png));
}
