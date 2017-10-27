I. Additional commands
    1. PKT_RECEIVED (0x0006)
        Each time server receives a DATA_DELIVERY packet from client, it will send back a PKT_RECEIVED packet to notify the client and tell the client to send the next packet. Server also generates a sequence number at the beginning of the communication (with the SERVER_HELLO packet), and it will increment this value for the following packets.
    2. FILE_STORED (0x0007)
        When server receives DATA_STORE packet from client, it will write the data stored in buffer into a file. After writing the file successfully, it will send a FILE_STORED packet back to notify client of the storing success.
    3. STORED_ERROR (0x0008)
        In case server cannot write data from buffer into a file, it will send a STORED_ERROR packet back to notify client of the storing failure. This command is different from ERROR command in terms of the time error happens. The ERROR command is used when an error occurs in the transfering time.

II. How to run the program
    1. Open terminal
    2. Enter 'p2/' directory
    3. Type 'make' and press Enter. The binary files 'select_server', 'multi_server' and 'client' are created in the 'p2/bin/' directory
    4. Run either 'select_server' or 'multi_server'
    5. Run the client following this format './client [server_ip] [number_of_requests]'

III. Scenario
    A. Overall scenario
        1. Run the server
        2. Run the client
        3. Client generates a number of threads to communicate with servers
        4. If server is 'select_server', it will process client request using one process in the order of first come, first served
           If server is 'multi_server', it will generate one process to handle one client request

    B. Scenario for each client request
        1. After the TCP connection is set up, client sends CLIENT_HELLO message to server, then server replies with SERVER_HELLO message
        2. Client uses DATA_DELIVERY message to send a string to server
        3. Server replies to client with a PKT_RECEIVED message
        4. Client sends DATA_STORE message with the file name
        5. After storing file successfully, server sends a FILE_STORED message back to client

IV. Other things
    To clean the binary files, do step 1, 2 in II, then type 'make clean' and press Enter
