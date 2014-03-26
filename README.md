## rscanf

Reads repeated input from file or standard input.

### Usage
It can be used as a simple scanf. It will return an error, if the non-format characters (everything except %d) in the format string, are not the same as the characters read.

EXAMPLE:
```c
scanf("%d = %d", &x, &y);
```

INPUT:
```
20 = 30
```

OUTPUT:
```
x = 20
y = 30
```

If there is repeated input, it can read it with a single call. The format string needs to have the following format:

"(NUMBER_OF_REPEATED_VALUES,SEPERATOR_STRING){FORMAT_STRING}"

EXAMPLE:
```c
int x[5];

rscanf("(5,\n){%d}", x);
```

INPUT:
```
1
2
3
4
5
```

OUTPUT:
```
x = {1, 2, 3, 4, 5}
```

This can also be nested:

EXAMPLE:
```c
int line[3], value[12];

rscanf("(3,\n){%d: (4, ){%d}}", line, value);
```

INPUT:
```
1: 1 2 3 4
2: 5 6 7 8
3: 9 10 11 12
```

OUTPUT:
```
line = {1, 2, 3, 4, 5}
value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
```

Sometimes you don't know how many times something repeats, but if it's written in the file, you can still use rscanf. For that, you can tell rscanf, that it should look at the value given by a pointer in the argument. Use 0 as the number of repeated values.

If you write a '*' after the 0, you tell rscanf to look at that pointer as an array. Both ways, it will use malloc to allocate the appropriate amount of memory. So don't forget to use free() afterwards!

EXAMPLE:
```c
int number_of_lines, *line_length, value*

rscanf("%d\n(0,\n){%d: (0*, ){%d}}", &number_of_lines, &number_of_lines, &line_length, &line_length, &value);

free(line_length);
free(value);
```

INPUT:
```
3
4: 1 2 3 4
3: 5 6 7
5: 9 10 11 12 13
```

OUTPUT:
```
number_of_lines = 3
line_length = {4, 3, 5}
value = {1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13}
```
