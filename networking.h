#ifndef _NETWORKING_H_
#define _NETWORKING_H_

int connectToServer();
int writeToServer(char *message, int size);
int readFromServer(char *message, int size);
int pullUpdates255(void *message);

#endif
