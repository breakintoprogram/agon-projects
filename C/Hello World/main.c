/*
 * Title:			Hello World - C example
 * Author:			Dean Belfield
 * Created:			22/06/2022
 * Last Updated:	22/11/2022
 *
 * Modinfo:
 */
 
#include <stdio.h>

// Parameters:
// - argc: Argument count - currently always 1
// - argv: Pointer to the argument string - zero terminated, parameters separated by spaces
//
// TODO: Need to make this C compliant at some point
//
int main(int argc, char * argv) {
	printf("Hello World\n\r");
	printf("Arguments:\n\r");
	printf("- argc: %d\n\r", argc);
	printf("- argv: %s\n\r", argv);
	return 0;
}