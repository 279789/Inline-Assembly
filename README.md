<!---
{
  "id": "6e50613d-60e2-40a8-91ae-609e0721f613",
  "depends_on": [
    "81f2e303-d35c-4857-9cb7-190e3c5372b0"
  ],
  "author": "Stephan Bökelmann",
  "first_used": "2025-06-05",
  "keywords": ["C Compiler", "Inline Assembly", "GCC Extended Asm", "Clobber List", "Inputs and Outputs"]
}
--->

# Inline Assembly in C: Inputs, Outputs, and Clobbers

> In this exercise you will learn how to embed inline assembly in C using GCC's extended asm syntax. Furthermore we will explore how to declare input operands, capture output values, and correctly use the clobber list to ensure reliable interaction between compiler and assembler.

## 1) Introduction

So far you have seen how a compiler can translate C code into machine instructions. But sometimes, you need to drop down one level closer to the hardware to write individual machine instructions yourself. This is where **inline assembly** comes into play.

Modern compilers like GCC allow you to embed assembly directly inside C code, using a feature called **extended inline assembly**. This lets you:

* access CPU registers directly,
* execute instructions that the C language cannot express,
* optimize certain critical code paths,
* interface with hardware or operating system features.

However, using inline assembly inside a C program is not without risk. You must carefully tell the compiler:

* which variables are inputs,
* which variables are outputs,
* which registers will be modified (clobbered).

GCC uses a fairly strict syntax to enforce this discipline:

```c
asm volatile (
    "assembly instructions"
    : output_operands
    : input_operands
    : clobber_list
);
```

Each of these sections is important:

* **Output operands:** variables that receive values from the assembly code.
* **Input operands:** variables whose values are passed into the assembly code.
* **Clobber list:** registers (or memory) that will be overwritten and must not be used by the compiler across the assembly block.

The keyword `volatile` ensures the compiler does not optimize away or reorder the assembly block, assuming it might have side effects that affect program correctness.

In this exercise, you will explore inline assembly step-by-step: starting with pure instructions, then introducing input and output operands, and finally working with clobbers to correctly inform the compiler.

### 1.1) Further Readings and Other Sources

* [GCC Inline Assembly HOWTO](https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html)
* [x86-64 System V ABI (PDF)](https://gitlab.com/x86-psABIs/x86-64-ABI/-/raw/master/x86-64-ABI.pdf)
* [Compiler Explorer - godbolt.org](https://godbolt.org/)
* [Intro to Inline Assembly (YouTube)](https://www.youtube.com/watch?v=4zOaVh1pZPI)

## 2) Tasks

### Task 1: Pure Assembly Without Operands

In this first task, we start with a simple assembly instruction that does not require any interaction with C variables.

Create the following C file:

```c
// file: nop.c

#include <stdio.h>

int main() {
    asm volatile ("nop");  // No operation, pure instruction

    printf("Executed a NOP instruction.\n");
    return 0;
}
```

Here, `nop` is a real x86 assembly instruction that literally means "do nothing." It occupies one CPU cycle but does not modify any registers or memory.

#### a) Compile and run:

* `gcc -Wall -o nop nop.c`

#### b) Inspect generated assembly:

* `objdump -d nop | less`
* Locate where the `nop` instruction appears.

#### c) Reflect:

* Why does the compiler allow this even without any operands? *This works, because nop(no operation) is an Assembler instruction that lets the cpu do nothing for one cycle, and because you need no further commands for doing nothing, this is also no problem.*
* What would happen if you omit `volatile`? *For me this works perfactly fine, but it some other cases it could happen that the compiler deletes some of your instructions after the optimisation process."volatile tells the processor to leave the instructions as they are and does not optimise them.*

---

### Task 2: Using Input and Output Operands

In this task, we introduce communication between C variables and the assembly code by passing values into and out of the assembly block.

Create the following C file:

```c
// file: add.c

#include <stdio.h>

int main() {
    int a = 5, b = 7, result;

    asm volatile (
        "addl %[input_b], %[input_a]"
        : [output_res] "=r" (result)
        : [input_a] "r" (a), [input_b] "r" (b)
    );

    printf("Result: %d\n", result);
    return 0;
}
```

#### Explanation:

* **"addl"**: adds two 32-bit integers.
* **Operands**:

  * `"=r" (result)` declares that `result` is an output operand. The `=` means it's written by the assembly.
  * `"r" (a)` and `"r" (b)` are input operands. The `r` constraint allows GCC to choose any general-purpose register.
* **Named placeholders** (`%[input_a]`) make the code more readable and less error-prone than numbered operands.

#### a) Compile:

* `gcc -Wall -o add add.c`

#### b) Inspect generated assembly:

* `objdump -d add`
* Try to identify which registers GCC assigned to which variables.

#### c) Analyze:

* Why is `result` declared as output? *Result is declared as an output, because it's the output of the assembler instruction(function), that is given back to main.*
* What if we accidentally declared both inputs as outputs? *The Compiler doesn't know which variables to use and uses some random numbers*In you do that, you tell the compiler that he ist allowed to manipulate and change these registers, which could cause problems and wrong results
* How does the compiler enforce register allocation using these constraints? The compiler does do that by following hte rules that you have set for each register. For example "r" means that gcc could use any general purpuse register.

---

### Task 3: Introducing Clobbers Safely

In this task, we explicitly inform the compiler that certain registers are being overwritten, even if they are not directly tied to variables.

Create this file:

```c
// file: clobber.c

#include <stdio.h>

int main() {
    int a = 42;

    asm volatile (
        "movl $100, %%eax\n"
        "addl %%eax, %[val]\n"
        : [val] "+r" (a)
        :
        : "%eax"
    );

    printf("Final value: %d\n", a);
    return 0;
}
```

#### Explanation:

* `movl $100, %eax` loads the constant `100` into `%eax`.
* `addl %eax, %[val]` adds `%eax` to the variable `a`.
* `+r` constraint for `[val]` means it is both read and written.
* The clobber list `"%eax"` tells the compiler that `%eax` will be modified, preventing conflicts with its own register allocation.

#### a) Compile:

* `gcc -Wall -o clobber clobber.c`

#### b) Inspect generated assembly:

* `objdump -d clobber`

#### c) Reflect:

* Why do we declare `%eax` as clobbered?
* What could happen if we omit the clobber list?
* Why is `+r` needed here instead of `=r`?

---

### Task 4: Using Numbered Operand References (%0, %1)

In this task, we introduce the alternative syntax where operands are referenced by position rather than names.

Create the following C file:

```c
// file: numbered.c

#include <stdio.h>

int main() {
    int x = 10, y = 20, sum;

    asm volatile (
        "movl %1, %0\n"
        "addl %2, %0\n"
        : "=r" (sum)
        : "r" (x), "r" (y)
    );

    printf("Sum is: %d\n", sum);
    return 0;
}
```

#### Explanation:

* `%0` refers to the first operand in the output list (`sum`).
* `%1` and `%2` refer to the first and second input operands (`x` and `y`).
* This form is more concise but slightly more error-prone because you must remember the correct ordering.
* No clobber list is needed since no fixed registers are touched explicitly.

#### a) Compile:

* `gcc -Wall -o numbered numbered.c`

#### b) Inspect generated assembly:

* `objdump -d numbered`

#### c) Reflect:

* How do `%0`, `%1`, `%2` map to the operands?
* What advantage do named operands (`%[name]`) provide compared to numbered references?
* When might you prefer one style over the other?

---

## 3) Questions

1. What does the `volatile` keyword mean in inline assembly?
2. Why is it important to distinguish between input and output operands?
3. How does GCC use constraints like `"r"`, `"=r"`, or `"+r"`?
4. What is the purpose of the clobber list?
5. What could happen if registers are clobbered but not declared?
6. Why is inline assembly a risky but sometimes necessary tool?
7. What is the difference between named operands (`%[name]`) and numbered operands (`%0`, `%1`)?

## 4) Advice

Always respect the compiler's optimizer: inline assembly interacts directly with register allocation and instruction scheduling. Declare your inputs, outputs, and clobbers honestly. Use `volatile` if your assembly performs side effects the compiler must not optimize away. Finally, always inspect your compiled binary with `objdump` to ensure your expectations match what the compiler actually emitted. Build confidence step by step before attempting more complex inline assembly involving system calls or register conventions.
