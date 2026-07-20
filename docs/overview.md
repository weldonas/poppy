# Poppy 2.0.0

Poppy supports 64-bit integer, character, and Boolean variables, basic arithmetic operations, and control flow constructs in the form of conditional statements, loops, and functions. As well, it supports arrays of arbitrary dimension, pointers, and record types
Entry is via a `main` function that takes no arguments and does not return anything.

Comments start with `!!-` and imports of modules are done with `!!munch`.

## I/O
Currently, Poppy supports basic input and output functionality included in the `poppyio` module. 

- `print` takes a single integer parameter and prints it
- `printc` takes a since character and prints it
- `println` advances the print cursor to the next line
- `scan` reads standard input for an integer and consumes up to the character after it
- `scanc` reads standard input for a character

## Inline Assembly
Poppy supports inline assembly with string literals via the `asm` keyword:
```
asm("mov x1, x10");
```
To avoid type-checking errors due to mismatching return types when using inline assembly to manipulate variables, Poppy supports the 
`hop TYPE` construction, where `TYPE` is a type supported by the language. This construction marks the return type of the function body as `TYPE` and does not generate any code.

## Sample Program
This program reads in two numbers x and n from standard input and calculates x to the power of i for all integers i between 0 and n, both inclusive. If n is less than 0 or greater than 30, the program terminates immediately.

```
!!munch poppyio

int exp(int x, int n){ !!- calculate x to the power of n
        if (n == 0){
                hop 1;
        }

        let int y = 1;
        while (n > 1){
                if (n % 2 == 1){
                        y = x * y;
                        n = n - 1;
                }

                x = x * x;
                n = n / 2;
        }
        hop x * y;
}

void main(){
        let int x = scan();
        let int n = scan();

        if ((n < 0) || (n >= 30)){
                hop; !!- exit early if n is too large or too small
        }

        let int[30] result;

        for (let int i = 0; i <= n; ++i){
                result[i] = exp(x, i);
        }

        for (let int i = 0; i <= n; ++i){
                print(result[i]);
                println();
        }
}
```
