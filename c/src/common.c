#include <stdio.h>

// Pause and then exit the program.
int quit(int status){
	printf("\n\nPress ENTER to exit");
	getchar();
	return status;
}

// Exit with an error message.
int error(char msg[]){
	printf(msg);
	return quit(1);
}

// Test a CSV string "value" for matches to int "key".
int match(int key, const char value[]){
	long *val;
	char **num;
	char s1[50];
	strcpy(s1, value);
	char *t1 = strtok(s1, ",");
	while (t1 != NULL){
		val = strtol(t1, &num, 0);
		if (val == key){
			return 1;
		}
		t1 = strtok(NULL, ",");
	}
	return 0;
}