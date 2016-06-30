# C语言打桩机制 "Interpositioning" in C
__Written and Translated by: Sanhu Li 李三乎__

***Tags: C, interposing, interpositioning***
---

## How does the idea come to my mind? 这篇博客的由来

Recently, I'm reading a book called "Expert C Programming -- Deep C Secrets". It's written by __Peter Van Der Linden__ and it's really useful. I've learned a few C features from the book. You will find it's a really old book but with great importance. Today, I'm writing about one feature I think we really need to take care of it. It's the "Interpositioning" feature in C (some people call it "interposing").

最近，我在读《C专家编程》一书，作者是 __Peter Van Der Linden__ ，这本书于我而言十分有用，我从中已经学习到了很多的C语言特性。这是一本有些过时但是非常重要的书。今天，我要写关于一项需要我们仔细对待的特性——即C语言的打桩特性。

One thing I learn from the book is that when we are using some functions which are implemented in other libraries, especially when we are building C++ code and we are trying to use C libraries, we need to implicitly declare the functions with the key word 'extern "C"'. This is because C++ has different policy to translate from function name to assembly code label name. We must tell the C++ compiler that we want the label is translated into a C style name.

我从书中学到的一点是：当我们在函数中调用其它函数库中的函数的时候，尤其是我们在C++代码中使用C语言库的时候，我们需要特别使用'extern "C"'关键字对函数定义进行显示声明。我们这么做的原因是C++对于函数名称转换成为二进制标签的过程采用了与C语言有着不同的策略，我们必须让C++编译器知道我们希望把这个调用函数名转换成C风格的标签名。

I've learned something from this feature that the translation of names. I'm thinking if I have two functions with the same name, how will the compiler translate the names? Will the two functions be translated into two different names? Will there be collision during runtime? Or will we be able to compile it at all?

从对命名的转换中我想到了一些东西。我想如果我们有两个函数具有相同的函数名，编译器会如何转换呢？这两个函数名会被转换成为不同的函数名吗？它们在运行时会产生冲突？还是编译器完全不会进行编译？

After reading to Chapter 5 of the book, the "Interpositioning" feature of C, I think I find the answer. This feature might be created by the way the compiler translate function names to labels, if there's only one way to translate it, it's highly possible that you have two parts in your assembly file with the same label. I think the user code (which are nearer to the main function) will be labeled at the top and when the instruction which wants to call some function name, it will go to the part we have written rather than the place it should go to. Also, since C++ has changed its way of naming the functions (to support function overloading feature), this problem may not happen in C++.

在读到这本书的第五章——C语言的打桩机制这一章节的时候，我认为我找到了答案。这个机制的产生可能是由C编译器名称转换机制造成的，如果名称和转换结果是一一对应的，在你的二进制文件中极有可能有两个函数具有相同的标签。我认为编程用户的代码（由于更加贴近于main函数）会被标记在上边，当我们执行到调用某个函数名的指令的时候，它会跳转到我们写的那个函数，而不是原函数库中的位置。并且C++更改了名称转换机制（从而支持函数重载），这个问题可能在C++中并不存在。

This blog shows the experiment of this feature under C compiler and C++ compiler with the same code to test the result.
本篇博客展示了在C和C++编译器中使用相同代码对这一特性进行测试的过程。

---

## The design of the experiment 实验设计

First, let's recall about this feature. "Interpositioning" means when you have a code with the same name with system functions (perhaps the same name with another function in some library written in C), and you are using a library which must use this system or library function, the library and your call cannot trigger the one from system or other library, you can only trigger the function by yourself. This might give some advantages when you are sure to implement another version of the function to be replaced that is much better (in performance, adding logs, etc.). But it's a disaster in most time.

首先，让我们回忆一下这个特性的内容。打桩机制是指当你有函数名与系统函数名（可能其它C语言函数库的名字）相同时，你同时使用了一个函数库调用了这个名字的系统函数，你和这个库对这个函数名的调用就无法调用到系统的函数了，只能调用自己写的函数。这个特性可能会有一些好处，尤其是你确信函数实现的没有问题并且你实现的函数有更好的特点（性能更快，添加了日志特性等等）。但是通常情况下这是一个灾难性的特性。

To test this feature, we must have a function near to main function (let's declare and implement it in `main.c`), let's say the cos function (the original definition is: `double cos(double x)`, we may need to add "-lm" to link the math library). So in this step, we will implement our own version of cos in `main.c`.

为了测试这一特性，我们必须有一个距离main函数很近的函数（所以让我们在`main.c`这一文件中声明和实现它），这个函数我们就选择cos函数（原函数声音是：`double cos(double x)`，我们可能需要在编译的末尾加上`-lm`来链接到系统数学库）。所以在这一步中，我们要在`main.c`中实现自己的cos函数。

Also, we want to test whether the functions in other library will call the replaced version instead of the one in math lib, so we need to write a library and use it in `main.c`, this library, called `my_lib.c`, will be a wrapper of the function cos.

而且，我们希望测试其它函数库中对cos函数调用是否用了我们的替代品而不是数学库的函数，我们必须写一个库，并在`main.c`函数中使用它，这个函数库——名为`my_lib.c`，会是一个对于cos函数的包装。

Moreover, if my thinking is correct, the assembly code will not care about the arguments about the function, so we should be able to define a cos function with different signature, in my case, it's `void cos(void)`, this function will print out a message indicating it's in my own routine.

并且，如果我的想法是正确的，二进制代码不会关心函数的参数，所以我们能定义一个函数声明并不相同的函数，在本次实验环境中是`void cos(void)`，这个函数会打印出消息表明使用的是我们自己的过程（不是系统的）。

From all above, we will have `main.c` and a `my_lib.c`. In `main.c`, we will define a function `void cos(void)`, also, in main function, we will call the cos_container (the wrapper function of cos), we would like to see the result of it. We will also call our own function to make a comparison in `main.c`. In `my_lib.c`, there should be only one function which is the wrapper function.

总之，我们会有`main.c`和`my_lib.c`。在`main.c`中，我们会定义一个函数`void cos(void)`，并且，在main函数中，我们会调用cos_container函数（cos函数的包装函数），我们希望看到函数运行的结果。我们也会调用自己版本的函数从而进行比较。在`my_lib.c`中，应该只会有一个函数，即是那个包装函数。

Of course, we will have some header files.

当然，我们会有相应的头文件。

## More thinking 更进一步

I also would like to test the idea that C++ should not have this problem. Since we could have C program compiled within a C++ compiler, I just rename the files `main.c` to `main.cc`, `my_lib.c` to `my_lib.cc`. We will see the output to check whether we have a different output from the same code.

我也想测试一下C++没有这个问题的想法是否正确。既然我们可以让C程序直接在C++编译器中编译，我仅仅把`main.c`修改为`main.cc`，`my_lib.c`修改为`my_lib.cc`。我们会检查输出，判断是否得到了不同的输出结果。

---

## The actual code 实际代码

Below are the actual code I've written for this experiment.

下列代码是在实验过程中使用的代码。

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

## Compile and Run result 编译及运行结果
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
We can see in the C version, the gcc compiler report a warning about the interposing function and the cos function called in the wrapper is turned into our cos. Because we have void as return type, we have 0 as the value we get from the wrong function call.

在C版本中，我们发现GCC编译器报告了一个警告，并且在包装函数中对cos的调用已经变成我们自己的函数了。因为我们的函数不返回值，所以在这个错误的调用中我们得到0作为结果。

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

我们发现我们这回没有得到警告，函数输出也并不相同。main函数中被调用的包装函数的结果并没有打印出指示使用的我们的函数的消息，并且运算的结果也不是0说明它的确调用了数学库中的函数。

---

## Conclusion 结论
From the result of the experiment, we are sure about the behavior of interposing feature in C, and my thinking that this feature is caused by the naming rule from C compilers, and C++ does not have this problem should be correct.

根据实验结果，我们可以确信C语言的打桩机制的运行结果，并且我对于这一特性是由于C语言名称转换规则造成的，而C++中并无此问题的想法应该是正确的。

## Future work 后续的工作
Even if the result seems fine, we are still having more things to check. For example, in our case, we are building the cos function with a different signature from the one in the math library, so the behavior in C++ compiler might be the result of the overriding feature in C++ (meaning that we may get some different result if we have exactly the same signature). We should check one more cos with exactly the same signature with the one in the math library and see which version it's using. We could also try the key word 'extern "C"' to test the naming and calling procedures.

即使结果看起来不错，我们还有很多东西需要检查。比如，在我们的实验中，我们使用了与系统库函数不同的函数声明，所以C++中的行为可能是由编译器实现的函数重载的结果（即：如果函数签名相同我们可能也会有打桩机制的产生，暂不确定结果如何）。我们应该再测试一个具有相同函数签名的cos函数，并观察程序用了哪个版本的函数。我们也可以尝试使用'extern "C"'关键字来测试命名和调用过程。

Moreover, we can find that the signature of our function does not need to be the same with the library one, so it's possible we could do something on the running stack (like need more args, we may be able to track the return address and other args, etc.). Also, can we call other functions using more or less parameters to "hack" the library? All these applications could be studied carefully.

而且，我们可以发现函数的签名并不需要与原函数相同，所以我们可以在运行栈上做些文章（比如我们的签名需要更多的参数，我们可能从而跟踪到返回地址和其它参数等等）。并且，我们是否可以用更多或者更少的参数调用其它函数，从而“黑”进函数库呢？所有这些应用需要我们仔细的研究。

We can also test whether this works for not only system functions but other libraries, I think these experiment will make us think more about programming in C.

我们还能测试这个是否不仅仅适用于系统函数，也适用于其它函数库，我想这些实验会让我们对C语言编程理解更加深刻。

---
## Acknowledgement 感谢
I have to thank __Peter Van Der Linden__, the writer of the book, that you have written such a good book that makes me make progress on C programming. Without your book, I cannot learn about this feature and build this experiment. Thank you so much.

我感谢 __Peter Van Der Linden__，这本书的作者，感谢你写了如此好的一本书让我在C语言编程的取得进步。没有你的书，我无法学到这一特性并设置本个实验，十分感谢。

I also want to thank my parents. They have offered me a stable and relaxed environment to do the experiment.

我还要感谢父母，他们为了提供了稳定舒适的环境做实验。

---
## Question 问题

If you have any concerns and find any problem about this experiment, please contact [me](mailto:lisanhu2014@hotmail.com).

如果你对这个实验有任何想法和问题，请联系[我](mailto:lisanhu2014@hotmail.com)。
