# Welcome to the Nest documentation!

## What is Nest and the philosophy behind it
Nest is a silly programming language, mostly for the syntax. It is a project of mine to experiment with the concept of using exclusively punctuation to code, removing all those keywords that make it apparently legible. This is why Nest code looks incredibly cryptic at first, but with a trained eye, I believe that the programming experience is not that different from the one you would have with any other language such as Python.

For example let's take `truth_machine.nest` in `examples/`:

```text
<<<'' '1' == ? ?.. >>> 1 []
```

This is certainly quite poorly formatted and hard to read to first things firts let's make it more legible:

```text
<<<'' '1' == ? [
    ?.. >>> 1 [
    ]
]
```

OK, now it is a little more clear and you can probably already understand which is the if statement (or actually expression) and which is the while loop, but if it is not obvious, using some variables should help.

```text
<<<'' = user_input
user_input '1' == ? [
    ?.. true [
        >>> 1
    ]
]
```

Now, if you are already familiar with other programming languages, you can see that `<<<''` simply asks some input from the user, then it is checked for equality against `'1'` and if that comes out to be true, the program enters an infinite loop that prints ones. 
In practice no one would write such odd code but in the examples it highlights the beauty of having no keywords. Take for example the equivalent C code, stripped of the preprocessor directives and the main function:

```c
int n; scanf("%d", &n); if (n == 1) while (printf("1"));
```

It just doesn't look as good, that is why the example is written the way it is: to show how beautifully misterious Nest looks and feels.