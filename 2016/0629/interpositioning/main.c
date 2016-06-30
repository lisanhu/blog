#include <stdio.h>
#include "my_lib.h"

void cos() {
	printf("In my cos();\n");
}

int main(void) {
	printf("First, call my cos();\n");
	cos();
	printf("\n");
	printf("Then, call the cos_container function\n");
	printf("%g\n", cos_container(2));
	return 0;
}
