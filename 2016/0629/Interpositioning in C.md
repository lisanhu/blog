# C语言打桩机制 "Interpositioning" in C
__By: Sanhu Li 李三乎__

***Tags: C, interposing, interpositioning***
---

## How does the idea come to my mind?

Recently, I'm reading a book called "Expert C Programming -- Deep C Secrets". It's written by __Peter Van Der Linden__ and it's really useful. I've learned a few C features from other ways of view. You will find it's a really old book but with great importance. Today, I'm writing about one feature I think we really need to take care of it. It's the "Interpositioning" feature in C (some people call it "interposing").

One thing I learn from the book is that when we are using some functions which are implemented in other libraries, especially when we are building C++ code and we are trying to use C libraries, we need to implicitly declare the functions with the key word 'extern "C"'. This is because C++ has different policy to translate from function name to assembly code label name. We must tell the C++ compiler that we want the label is translated into a C style name.

I've learned something from this feature that the translation of names. I'm thinking if I have two functions with the same name, how will the compiler translate the names? Will the two functions be translated into two different names? Or there will be collision during runtime?

After reading to Chapter 5 of the book, the "Interpositioning" feature of C, I think I find the answer. This feature might be created by the way the compiler translate function names to labels, if there's only one way to translate it, it's highly possible that you have two parts in your assembly file with the same label. I think the user built code (which are nearer to the main function) will be labeled at the top and when the instruction which wants to call some function name, it will go to the part we have written rather than the place it should go to. Also, since C++ has changed its way of naming the functions (to support function overloading feature), this problem may not happen in C++.

This blog shows the experiment of this feature under C compiler and C++ compiler with the same code to test the result.

---

## The design of the experiment

First, let's recall about this feature. "Interpositioning" means when you have a code with the same name with system functions (perhaps the same name with another function in some library written in C), and you are using a library which must use this system or library function, the library and your call cannot trigger the one from system or other library, you can only trigger the function by yourself. This might give some advantages when you are sure to implement another version of the function to be replaced that is much better (in performance, adding logs, etc.). But it's a disaster in most time.

To test this feature, we must have a function near to main function (let's declare and implement it in `main.c`), let's say the cos function (the original definition is: `double cos(double x)`, we may need to add "-lm" to link the math library). So in this step, we will implement our own version of cos in `main.c`.

Also, we want to test whether the functions in other library will call the replaced version instead of the one in math lib, so we need to write a library and use it in `main.c`, this library, called `my_lib.c`, will be a wrapper of the function cos.

Moreover, if my thinking is correct, the assembly code will not care about the arguments about the function, so we should be able to define a cos function with different signature, in my case, it's `void cos(void)`, this function will print out a message indicating it's in my own routine.

From all above, we will have `main.c` and a `my_lib.c`. In `main.c`, we will define a function `void cos(void)`, also, in main function, we will call the cos_container (the wrapper function of cos), we would like to see the result of it. We will also call our own function to make a comparison in `main.c`. In `my_lib.c`, there should be only one function which is the wrapper function.

Of course, we will have some header files.

## More thinking

I also would like to test the idea that C++ should not have this problem. Since we could have C program compiled within a C++ compiler, I just rename the files `main.c` to `main.cc`, `my_lib.c` to `my_lib.cc`. We will see the output to check whether we have a different output from the same code.

---

## The actual code

Below are the actual code I've written for this experiment.

`main.c`
```C
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
```
`my_lib.h`
```C
#ifndef MY_LIB_H
#define MY_LIB_H
double cos_container(double x);
#endif
```
`my_lib.c`
```C
#include <math.h>

double cos_container(double x) {
	return cos(x);
};
```

## Compile and Run result
```sh
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -c -g main.c -o main.c.o
main.c:4:6: warning: conflicting types for built-in function ‘cos’
 void cos() {
      ^
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -c -g my_lib.c -o my_lib.c.o
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -o main.c.out main.c.o my_lib.c.o -lm
lsh@lsh:~/mine/ws/C/interpositioning$ ./main.c.out
First, call my cos();
In my cos();

Then, call the cos_container function
In my cos();
0
lsh@lsh:~/mine/ws/C/interpositioning$
```
We can see in the C version, the gcc compiler report a warning about the interposing function and the cos function called in the wrapper is turned into our cos. Because we have void as return type, we have 0 as the value we get from the wrong function.

```sh
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -c -g main.cc -o main.cc.o
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -c -g my_lib.cc -o my_lib.cc.o
lsh@lsh:~/mine/ws/C/interpositioning$ gcc -o main.cc.out main.cc.o my_lib.cc.o -lm
lsh@lsh:~/mine/ws/C/interpositioning$ ./main.cc.out
First, call my cos();
In my cos();

Then, call the cos_container function
-0.416147
lsh@lsh:~/mine/ws/C/interpositioning$
```
We can see we do not have any warning this time and the output of the function is different. The result of the wrapper function called in main does not print out any message indicating using our own version of function and the result value is not 0 meaning it actually calls the cos function in math library.

---

## Conclusion
From the result of the experiment, we are sure about the behavior of interposing feature in C, and my thinking about this feature is caused by the naming rule from C compilers, and C++ does not have this problem.

## Future work
Even if the result seems fine, we are still having more things to check. For example, in our case, we are building the cos function with a different signature from the one in the math library, so the behavior in C++ compiler might be the result of the overriding feature in C++. We should check one more cos with exactly the same signature with the one in the math library and see which version it's using.

---
## Acknowledgement
I have to thank __Peter Van Der Linden__, the writer of the book, that you have written such a good book that makes me make progress on C programming. Without your book, I cannot learn about this feature and build this experiment. Thank you so much.

---
## Question

If you have any concerns and find any problem about this experiment, please contact [me](mailto:lisanhu2014@hotmail.com).
