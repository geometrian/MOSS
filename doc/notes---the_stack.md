# The Stack

Each core on the processor has a concept called the stack, which is a region of memory used for
storing scratch space for the nested functions called.

On x86, it grows *downward* as function calls nest deeper and deeper inside each other.
Intuitively, you can think of starting at a 'high' address like 0xffff'ffff and growing toward 'low'
addresses (toward zero, 0x0000'0000).  For kernel development, we can actually start at 0x0000'0000
and push downward to -1 (0xffff'ffff) to kick the whole thing off; this makes sense if you think
about it as signed.  Also, everything is actually 64-bit.



## Example Stack

An example stack (32-bit, "cdecl" calling convention, near `call` and `ret`) might look like this:

	╭──────────╮
	│ Figure 1 │
	╰──────────╯     call info                                           NULL
	                  (dummy)  ╲                                          ^
	0x00105004 ━━━━━━━━━━━━━━━━━━━>━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓   |
	               ROUTINE 0    │   ┃ Return to routine -1 EBP value  ╂ ─ ┘
	0x00105000    STACK FRAME   ╞═> ┠─────────────────────────────────┨ <━┯━ Routine 0 EBP
	                            │   ┃ Routine 1 argument 1            ┃   |
	0x00104FFC            call ─┤   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                      info  │   ┃ Return to routine 0 EIP address ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━┯━ Routine 1 EBP
	                                ┃ Local variable                  ┃   |
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨   |
	                      call ─┤   ┃ Routine 2 argument 2            ┃   |
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Routine 2 argument 1            ┃   |
	0x00104FE8                  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Return to routine 1 EIP address ┃   |
	0x00104FE4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 1 ESP
	               ROUTINE 2    │   ┃ Return to routine 1 EBP value   ╂ ─ ┘
	0x00104FE0    STACK FRAME   └─> ┠─────────────────────────────────┨ <━┯━ Routine 2 EBP
	                                ┃ Local variable                  ┃   |
	0x00104FDC                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                                ┃ Local variable                  ┃   |
	0x00104FD8                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                                ┃ Local variable                  ┃   |
	0x00104FD4                  ┌─> ┠─────────────────────────────────┨   |
	                      call ─┤   ┃ Routine 3 argument 1            ┃   |
	0x00104FD0            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Return to routine 2 EIP address ┃   |
	0x00104FCC ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 2 ESP
	               ROUTINE 3    │   ┃ Return to routine 2 EBP value   ╂ ─ ┘
	0x00104FC8    STACK FRAME   ╞═> ┠─────────────────────────────────┨ <━┯━ Routine 3 EBP
	                            │   ┃ Return to routine 3 EIP address ┃   |
	0x00104FC4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 3 ESP
	               ROUTINE 4    │   ┃ Return to routine 3 EBP value   ╂ ─ ┘
	0x00104FC0    STACK FRAME   ├─> ┠─────────────────────────────────┨ <━━━ Routine 4 EBP
	                           ╱    ┃ Local variable                  ┃      (Current EBP)
	0x00104FBC              call    ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                        info    ┃ Local variable                  ┃
	0x00104FB8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 4 ESP
	                                ┃                                 ┃      (Current ESP)
	0x00104FB4                      ┃        (dragon habitat)         ┃
	                                ┃                                 ┃
	                                ┃  \/ \/ \/ \/ \/ \/ \/ \/ \/ \/  ┃

In this example, routine 0 (the entry point of the thread, which is a little special) called routine
1, which called routine 2, which called routine 3, which called routine 4.  Routine 4 is where the
core is currently executing.

## Function Call Example

Let's see how the stack evolves from the perspective of routine 1 (the 'caller') and routine 2 (the
'callee').

We start in the caller's frame:

	╭──────────╮
	│ Figure 2 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫   |
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘  Routine 1 EBP
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ (Current EBP)
	                                ┃ Local variable                  ┃
	0x00104FF0 ━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━━━ Routine 1 ESP
	                                                                         (Current ESP)

The routine's actual data starts at the frame's EBP (the 'frame pointer') which, since this is the
current frame, is also the value of the EBP register.  The frame's ESP (the 'stack pointer') marks
the end of the stack.  We have 4 bytes of local variables, so ESP is just 4 bytes lower.  Again,
since this is the current frame, it is also the current value of the ESP register.

### Caller: push arguments

The caller now wants to call routine 2.  Routine 2 takes two arguments.  The first thing the caller
does is [`push`](https://www.felixcloutier.com/x86/push) the two arguments with code like:
```x86asm
; caller code
push   222   ; the value of the second argument
push   111   ; the value of the first argument
```
This has the effect of beginning to build a new frame for the callee:

	╭──────────╮
	│ Figure 3 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ Routine 1 EBP
	                                ┃ Local variable                  ┃      (Current EBP)
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨
	                      call ─┤   ┃ Routine 2 argument 2            ┃
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                            │   ┃ Routine 2 argument 1            ┃
	0x00104FE8 ━━━━━━━━━━━━━━━━━━━>━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━━━ Routine 1 ESP
	                                                                         (Current ESP)

It might seem slightly strange to push the arguments in reverse order, but notice that it makes the
arguments show up in order in memory (e.g. argument 1 is at a lower address than argument 2).

### Caller: call (pushes return address and jumps)

The caller now executes the [`call`](https://www.felixcloutier.com/x86/call) instruction:
```x86asm
; caller code
call   routine2   ; pushes return address and then jumps
```
This pushes the return address onto the stack and then jumps to the appropriate code.  The return
address is basically the EIP register, which holds the instruction pointer for the instruction
following the `call`.  (Remember that the *code* memory and the *stack-data* memory are completely
separate!)

	╭──────────╮
	│ Figure 4 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ Routine 1 EBP
	                                ┃ Local variable                  ┃      (Current EBP)
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨
	                      call ─┤   ┃ Routine 2 argument 2            ┃
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                            │   ┃ Routine 2 argument 1            ┃
	0x00104FE8                  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                            │   ┃ Return to routine 1 EIP address ┃   ┏━ Routine 1 ESP
	0x00104FE4 ━━━━━━━━━━━━━━━━━━━>━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━┫
	               ROUTINE 2       ╱                                      ┗━ Routine 2 ESP
	              STACK FRAME     ╱                                          (Current ESP)
	           ━━━━━━━━━━━━━━━━━━━

### Callee: push (save) and then setup frame pointer

The callee is now executing code.  The first thing the callee must do is save the caller's frame
pointer by `push`ing EBP.  Then routine 2 sets up its EBP by moving it to the current frame pointer:
```x86asm
; callee code
push   ebp        ; save EBP of caller
mov    ebp, esp   ; set EBP for callee
```

	╭──────────╮
	│ Figure 5 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━┯━ Routine 1 EBP
	                                ┃ Local variable                  ┃   |
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨   |
	                      call ─┤   ┃ Routine 2 argument 2            ┃   |
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Routine 2 argument 1            ┃   |
	0x00104FE8                  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Return to routine 1 EIP address ┃   |
	0x00104FE4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 1 ESP
	               ROUTINE 2    │   ┃ Return to routine 1 EBP value   ╂ ─ ┘
	              STACK FRAME   └─>┏┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━┳━ Routine 2 EBP
	0x00104FE0 ━━━━━━━━━━━━━━━━━━━━┛                                      ┃  (Current EBP)
	                                                                      ┃
	                                                                      ┗━ Routine 2 ESP
	                                                                         (Current ESP)

### Callee: function operation and alloc/free local variables

Routine 2 is now free do whatever it wants.  For example, it might allocate 12 bytes of local
variables by `push`ing three values.  It could also reserve it all at once by decrementing the stack
pointer with `sub`:
```x86asm
; callee code
sub   esp, 12   ; allocate space for some local variable(s)
```
Which way is (slightly) better [depends on the situation](https://stackoverflow.com/a/62442626/).

	╭──────────╮
	│ Figure 6 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━┯━ Routine 1 EBP
	                                ┃ Local variable                  ┃   |
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨   |
	                      call ─┤   ┃ Routine 2 argument 2            ┃   |
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Routine 2 argument 1            ┃   |
	0x00104FE8                  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
	                            │   ┃ Return to routine 1 EIP address ┃   |
	0x00104FE4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 1 ESP
	               ROUTINE 2    │   ┃ Return to routine 1 EBP value   ╂ ─ ┘
	0x00104FE0    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ Routine 2 EBP
	                                ┃ Local variable                  ┃      (Current EBP)
	0x00104FDC                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                                ┃ Local variable                  ┃
	0x00104FD8                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                                ┃ Local variable                  ┃
	0x00104FD4 ━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━━━ Routine 2 ESP
	                                                                         (Current ESP)

Routine 2 calls routine 3, which calls routine 4, and so on, but eventually it all returns back and
we're in routine 2 again.  We're ready to return back into routine 1.

First, we have to remove all the local variables by moving ESP back up to BSP.  In our example, we
can [`pop`](https://www.felixcloutier.com/x86/pop) three times, or we can just:
```x86asm
; callee code
mov   esp, ebp   ; forget about our local variables
```
to do that directly.  We're now back to where we were above in Figure 5, before allocating the
variables.

	╭────────────────────────────────────────────╮
	│ Figure 5 (partially reproduced from above) │
	╰────────────────────────────────────────────╯

	                      call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FE4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 1 ESP
	               ROUTINE 2    │   ┃ Return to routine 1 EBP value   ╂ ─ ┘
	              STACK FRAME   └─>┏┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━┳━ Routine 2 EBP
	0x00104FE0 ━━━━━━━━━━━━━━━━━━━━┛                                      ┃  (Current EBP)
	                                                                      ┃
	                                                                      ┗━ Routine 2 ESP
	                                                                         (Current ESP)

### Callee: pop (restore) caller frame pointer and return (pops return address and jumps)

We now `pop` the stack into EBP, thus loading routine 1's EBP value into register EBP.  This brings
us back to Figure 4.

Then we execute [`ret`](https://www.felixcloutier.com/x86/ret), which pops the stack into EIP, thus
moving the saved return address into the instruction pointer and transferring execution back to the
caller.
```x86asm
; callee code
pop   ebp   ; load the EBP of the caller
ret         ; pop the return address, then jump to it
```
The (integer) return value of the function is in EAX.

	╭──────────╮
	│ Figure 7 │
	╰──────────╯          call  │                                         ^
	                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
	0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
	               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
	0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ Routine 1 EBP
	                                ┃ Local variable                  ┃      (Current EBP)
	0x00104FF0                  ┌─> ┠─────────────────────────────────┨
	                      call ─┤   ┃ Old routine 2 argument 2        ┃
	0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
	                            │   ┃ Old routine 2 argument 1        ┃
	0x00104FE8 ━━━━━━━━━━━━━━━━━━━>━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━━━ Routine 1 ESP
	                                                                         (Current ESP)

The caller has to decide what to do with the old arguments.  It can `pop` them (or increment ESP),
thus returning to Figure 2.  This can be deferred to when routine 1 itself returns.  Or routine 1
might treat the arguments as local variables and use their values.
