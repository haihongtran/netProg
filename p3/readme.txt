I. How to run the program
    1. Open terminal
    2. Enter 'p3/' directory
    3. Type 'make' and press Enter. The binary files 'web_server' and 'client' are created in the 'p3/bin/' directory
    4. Run './webserver'
    5. Run the client following this format './client [server_ip] [server_port] [threads_number] [requests_per_thread]'

II. Scenario
    1. Server starts. It initializes all things (thread pool, file descriptor pool) and waits for client requests
    2. Client starts. It creates a number of threads, each thread sends a number of HTTP requests to the server
    3. Server uses select function to handle the client requests with the help of the thread pool
    4. The active threads in the thread pool dequeue task from task queue and process the task (sending back HTTP response)
    5. After a thread at the client side sends all its requests, it will close the socket and exit
    6. After sending the responses, server continues to listen to new client requests

III. HTML file transmission
    1. Firstly, server thread sends the HTTP response header, following by data of the index.html file
    2. Client thread receives the data but currently do nothing to it (no requirement for processing)
    3. After sending the file, server thread sends an ending message to client to signal the completeness of file transmission
    4. Client thread receives the ending message and finish the transmission session

IV. Notes
    1. The index.html file is stored in the 'p3/res' directory
    2. In the task queue at the server side, besides client socket descriptor, each task also includes function pointer (to process task), client address (including client IP address and client port), and assigned thread ID for the task
