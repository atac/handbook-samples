#include <stdio.h>
#include <string.h>

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
	char **ctx = NULL;
	const char s1[100];
	strcpy_s(s1, 100, value);
	char *t1 = strtok_s(s1, ",", &ctx);
	while (t1 != NULL){
		val = strtol(t1, &num, 0);
		if (val == key){
			return 1;
		}
		t1 = strtok_s(NULL, ",", &ctx);
	}
	return 0;
}