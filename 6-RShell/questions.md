1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

It uses the null terminated bit at the end of each message to determine when the command has been fully recieved. You could loop through the output until you recieve the \0. 

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

You could again, use an end of message symbol to signal to that the message is complete. Or you could use something to mark the beggining or end of the code. If handled incorrectly, TCP could break the command into multiple packets, which could be processed seperately, breaking the command. Obviously this could lead to data loss and/or corruption

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols keep information about a clients previous interactions (or state) across multiple requests. 

Stateless protocols on the other hand, treat each interaction from a client as completely independent, not relying on previous requests. So each request must contain all the necessary parts to be processed correctly. 

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

While it may not be reliable, as in ensuring packets are sent in the correct order, or ensuring that the packets are delivered, UDP is very very fast and lightwieght. So for any applications that require q fast response time, but don't neccesarily require the connection to be relaible, UDP is ideal

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The operating system provides the Socket API, which allows programs to send and recieve the data over a network. 