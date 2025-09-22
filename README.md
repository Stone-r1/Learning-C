# Learning C

---

## Reason
I always wanted to improve my C knowledge, so I decided to start this series where I solve simple tasks in C programming language.

## Problem 1
Write a C program `repeat` writing a message in a loop to standard output. The command syntax:

### Syntax

```bash
./repeat -n <count> -t <text>
```

---

## Problem 2
Write a C program called `bench` that executes a command repeatedly in order to measure its execution time. The implementation of `bench` should implement a command line option `-d` to set the number of seconds that `bench` should execute the command in a loop.The option `-w` can be used to set the number of warmup runs. `bench` program should produce summary information indicating the time of the **shortest run**, the time of the **longest run**, the **average** runtime, the **total time** that has elapsed and the **total number** of command executions and the number of **failed** command executions.

### What is allowed to use (recommended)
`fork()`, `execvp()`, `waitpid()`, `clock_gettime()`.

### Restrictions
`system()`.

### Syntax

```bash
./bench -w <count> -d <count> [command to execute]
```

### Expected Result (example)
```bash
Min: 1.003625 seconds       Warmups: 2
Avg: 1.007656 seconds       Runs: 4
Max: 1.013614 seconds       Fails: 0
Total: 4.030625 seconds
```
