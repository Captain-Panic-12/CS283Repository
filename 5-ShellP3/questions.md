1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

It creates an array, named pids[], the size of the number of max elements for a command. When it comes time to execute the child processes, it iterates through this array and calls waitpid() on each one. 

If you forgott to call waitpid, those child processes would become zombies, taking system resources without being properly terminated. 

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

It mitigates potential resource leaks by closing unused  file descriptors, and the the reader end might keep waiting for data thats never going to be written

Open pipes lead to resource leaks or conflicts if multiple processes have access to the pipes read or write end

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

external commands run in a child process, and cd would only change the child processes working directory, built in commands on the other hand work in the actual shells process, so allowing cd to be a built in command allows to change the shells working directory. 

If CD were an external command, it wouldnt be able to modify the parent enviornment, and which then would require a lot of workaround to get an approximation going. 


4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

_answer here_
