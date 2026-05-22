# AI Usage Documentation — All Phases

## Tool used

Throughout all three phases I used ChatGPT and Claude as AI assistants. I used them in different ways depending on what I needed: sometimes to generate specific functions, sometimes to understand concepts better, and sometimes to debug issues I was stuck on.

---

## PHASE 1 — File Systems

### Context

In Phase 1 I had to build the foundation of the project: storing reports in binary files, managing permissions, symbolic links, and implementing a filter command.

### Filter functions — AI assisted

The spec explicitly allowed using AI for two functions in the filter command.

My report struct:

```c
typedef struct {
    int report_id;
    char inspector_name[50];
    double latitude;
    double longitude;
    char category[30];
    int severity;
    time_t timestamp;
    char description[100];
} Report;
```

**First function — `parse_condition`**

I asked the AI to generate a function that splits a string like `severity:>=:2` into three parts: field, operator, and value.

The AI used `strchr()` to find the `:` characters and `strncpy()` / `strcpy()` to extract each part.

What I changed after reviewing it:
- added checks for missing `:` characters
- added checks for empty field or operator
- added a length check to avoid buffer overflow

**Second function — `match_condition`**

I asked the AI to generate a function that checks whether a report satisfies a condition.

The AI generated comparison logic based on field type — integers for severity and timestamp, strings for category and inspector.

What I changed:
- split the comparison logic into two separate helper functions: `compare_int()` and `compare_string()`
- used `atol()` for integer conversion instead of `atoi()` to handle timestamps correctly

**Filter logic — written by me**

I wrote the actual filter loop myself: opening `reports.dat`, reading records one by one with `read()`, calling `parse_condition()` and `match_condition()` for each condition, and printing only reports where all conditions returned 1.

### Other AI usage in Phase 1

I also used the AI to better understand some concepts I was less familiar with:

- how `lstat()` differs from `stat()` and why it matters for symlinks
- how `ftruncate()` works after shifting records in `remove_report`
- how to extract permission bits from `st_mode` using macros like `S_IRUSR`, `S_IWGRP`

In all cases I read the generated explanations, understood the logic, and wrote the actual code myself.

### What I learned

- how binary file I/O works with fixed-size structs
- how Unix permission bits are structured and how to check them
- that AI-generated code always needs to be reviewed — it sometimes skips edge cases

---

## PHASE 2 — Processes and Signals

### remove_district

I used AI to understand how to properly implement process creation for running `rm -rf`. The discussions helped me understand:

- how `fork()` creates a child process and what each process does after the split
- how `execlp()` replaces the child process with an external command
- how `waitpid()` makes the parent wait for the child to finish
- how `unlink()` removes the symbolic link after the directory is deleted

After understanding the flow I wrote the function myself and integrated it into my existing code structure.

### monitor_reports

I used AI to understand signal handling since it was a new topic for me. The discussions helped me understand:

- why `.monitor_pid` is needed and how to write a PID to a file using `write()`
- how `sigaction()` works and why the spec says not to use `signal()`
- how `SIGUSR1` and `SIGINT` are different and what each is used for
- why signal handlers must use `write()` instead of `printf()` — async-signal-safety
- how `pause()` blocks the process until a signal arrives

I then implemented the program myself based on what I understood.

### Debugging help

I also used AI during debugging when things were not working as expected. For example, when the monitor was not responding to signals I used AI to understand that the issue was related to how I was sending the signal — I was using the wrong PID. The AI helped me understand what `.monitor_pid` contains and how to read it correctly.

### What I learned

- how parent and child processes behave after `fork()`
- how signal handlers work and their limitations
- how processes communicate using signals with `kill()`
- how to keep a process alive waiting for events using `pause()`

---

## PHASE 3 — Pipes and Redirects

### city_hub — start_monitor

Phase 3 introduced pipes and `dup2()`, which I had not used much before. I used AI to understand the overall architecture required by the spec.

The main thing I needed to understand was the two-level process structure: `city_hub` forks `hub_mon`, and `hub_mon` forks `monitor_reports`. I used AI to understand:

- why two separate forks are needed instead of one
- how `dup2()` redirects stdout to the write end of a pipe
- why `city_hub` should not wait for `hub_mon` — so it stays responsive
- how `hub_mon` reads from the pipe and relays messages to the terminal

I also had a bug where `close(fd[0])` and `waitpid()` were inside the while loop instead of after it. I used AI to understand why that was wrong — closing the read end of the pipe on the first iteration meant all subsequent `read()` calls would fail.

After understanding the structure I wrote the code myself, including the pipe setup, the dup2 redirections, and the message detection logic.

### city_hub — calculate_scores

For `calculate_scores` I used AI mostly to confirm my approach was correct. The logic is similar to `start_monitor` — fork a child per district, redirect its stdout through a pipe, read the output in the parent. I understood the pattern from the previous function and applied it myself.

### scorer.c

I wrote `scorer.c` mostly on my own since it was straightforward binary file reading. I used AI briefly to double-check the struct layout matched `city_manager.c` exactly, since mismatched struct sizes would cause the binary reads to produce wrong data.

### Debugging help in Phase 3

I ran into an issue where `hub_mon` was not displaying any messages even though the monitor started. The AI helped me identify two problems:

- `printf()` output was buffered and not being sent through the pipe until the process exited — fixed with `fflush(stdout)`
- the string length passed to `write()` in the signal handler was off by one, causing the newline to be cut — fixed by counting the characters correctly

Both were small mistakes but hard to spot without understanding how pipe buffering works.

### What I learned

- how pipes work at the file descriptor level
- how `dup2()` redirects I/O between processes
- that `printf` buffers output and `fflush` is necessary when writing through a pipe
- how to build a small multi-process system where each component has a clear role
- that debugging inter-process communication requires understanding what each process sees at each moment