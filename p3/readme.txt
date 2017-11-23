I. How to run the program
    1. Open terminal
    2. Enter 'p3/' directory
    3. Type 'make' and press Enter. The binary files 'web_server' and 'client' are created in the 'p3/bin/' directory
    4. Run './webserver'
    5. Run the client following this format './client [server_ip] [server_port] [threads_number] [requests_per_thread]'

II. Scenario
    1. Server runs first. It initializes all things (thread pool, file descriptor pool) and waits for client requests
    2. Client runs. It creates a number of threads, each thread sends a number of HTTP requests to the server
    3. Server uses select function to handle the client requests with the help of the thread pool
    4. The active threads in the thread pool dequeue task from task queue and process the task (sending back HTTP response)
    5. After a thread at the client side send all its requests, it will close the socket and exit
    6. After sending the responses, server continues to listen to new client requests

III. Notes
- In the task queue at the server side, besides client socket descriptor, each task also includes other things like function pointer and client address (including client address and client port)
- The task queue implemented inside the thread pool
- The index.html file is stored in the 'p3/res' directory
- Assume client can parse the HTTP response to get the HTML file size. Implement HTTP parser is not the purpose of this assignment.
