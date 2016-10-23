Execution
===
type `make` to produce the executables.
`./bidding_system [host_num] [player_num]` to run.

Description
===
I use the `fork` and `pipe` and `mkfifo` to do most of my work.
For the bonus, I always bid all my money.
But I'm afraid that other people may use the same approach, so I always bid `money + rand() % 8` to add some randomness.

Self-Examination
===
I've done all the grading criteria.
1. I use `pipe` & `dup` & `fork` & `exec` to do the trick.
2. I use `select` to do I/O multiplexing.
3. I implement inter-process communication to do the job.
4. I use `mkfifo` & `fork` & `exec` to do the trick.
5. I implement the algorithm that the spec requested.
6. I write code.
7. I write makefile.

