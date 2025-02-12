1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  It reads a line from input, and then stores it as a buffer, which we have expeirence with accessing and using from past assignments. 

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  For future proofing, we're going to be using this shell in the future, and the actual size of the command may vary. Better to have and not need, then need and not have. 


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  The commands need to be very precise, even a little extra whitespace could throw the entire implementation off. 

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  > directs output of command to file, instead of screen, < directs input for a command from a file, and >> appends the output of the command to a specified file. 

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirecting involves taking from and adding to files, while piping takes the output from one command, and directs it as the input for the next command. 

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  STDOUT is used for our outputs, that's how we see the results of our program. STDERR is used for error messages. We have to keep them seperate, otherwise it might get hard to tell what's an error, and what is the expected output. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  Our shell should catch the erorrs, and display them away from the standard output. 