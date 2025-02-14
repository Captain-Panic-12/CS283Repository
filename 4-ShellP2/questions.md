1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  fork creates a new child processes, but exec replaces the child process with the new program. If we just used execvp we'd lose out on the ability to handle background processes

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork syscall fails, then no child process is created. In my implementation, it returns ERR_Memory, which is equivalent to -1 

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp is given either a program name or path to the program, it searches the PATH Variable for the program name, and will execute said program, if it's given the path to the program, it'll try to execute that file. 

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  wait() is called to help manage resources, and prevent zombie processes from accumulating. If wait wasn't called, it could result in the zombie processes accumulating, and potentially leaking system resources. 

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: It allows the parent processes to look at and handle the child processes' exit status. It lets us know if the child processes exited succesfully, or it it ran into an error. We can use this to better handle errors and improve the program's flow. 

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  It treats everything between the opening and closing of a single or double qoute as one arguement. Without this implementation, it would mean that the arguement "Hello World" would be parsed as two arguements instead of one. 

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  It now handles qouted arguments, it also has better error handling. The handling qouted arguements was an unexpected complication that took a bit of doing to get working. 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  To notify processes that an event has occured 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGINT: Used to interrupt a process , but give the process time to cleanup before it closes, SIGTERM does pretty much the same, but it's more commonly used by shutdown scripts or administrative tools. SIGKILL is forceful termination, unlike the other two it cannot be ignored. It will shutdown the process.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  It's immediately suspended. It remains this way until it recieves a SIGCONT signal to resume execution. It cannot be caught or ignored. SIGSTOP is designed to reliably pause processes, if it could be caught or ignored, then there's no point. 
