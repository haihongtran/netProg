I. How to build the program
    Enter 'p4/' directory, type 'make' and press Enter.
    Binary files 'super' and 'child' are created in 'p4/bin/' directory.

II. How to run the program
    The program follows the description in the assignment. The general flow is:
    1. Create n directories for n child nodes
    2. Copy 'child' binary file to each of the created directories
    3. In each of the n created directory, create two directories named 'data' and 'download'
    4. Copy different data files into different 'data' directories
    5. Enter /bin directory, run 2 super nodes following the format:
        + Super node 1: ./super -p [port]
        + Super node 2: ./super -p [port] --s_ip [IP_super_node_1] --s_port [port_super_node_1]
    6. Enter created directories for child nodes. In each of the directory, run command:
        ./child -p [port] --s_ip [super_node_ip] --s_port [super_node_port]
    7. In each terminal for each child node, enter following command to download file from peer:
        get [file_name] [destination_file_name]

II. Scenario (assuming no error happens)
    1. Super node 1 starts running
    2. Super node 2 starts running and exchanges HELLO_SUPER_TO_SUPER messages with super node 1
    3. Child node starts running
        a. It first exchanges HELLO messages with its specified super node (HELLO_FROM_CHILD and HELLO_FROM_SUPER)
        b. It scans local storage (./data/) to get file information (name and size) and sends these information to its corresponding super node via FILE_INFO packet
    4. The corresponding super node receives information, replies to the child a FILE_INFO_RECV_SUCCESS message, and shares the information to the other super node (FILE_INFO_SHARE and FILE_INFO_SHARE_SUCCESS packet)
    5. Steps 3 and 4 are repeated as the other child nodes run, until all child nodes have connected to their specified super node and delivered their file information
    6. User enters command via STDIN to download a file. Child asks its super node for the file information via SEARCH_QUERY packet.
    7. Super node replies to the child a SEARCH_ANS_SUCCESS packet containing necessary file information (address of peer having the file and file size)
    8. The child node sends a FILE_REQ packet to the peer having the file
    9. Peer replies FILE_RES_SUCCESS packet(s) with file contents
    10. The child node receives and saves the file into its './download/' directory

III. Description of messages
    The structure of header is given as follow. The number in round brackets describes size of the field in bytes.
        --------------------------------------------
        | Total length (4) | ID (4) | MSG Type (4) |
        --------------------------------------------

    Please note that all the packets of the protocol starts with the described header above.
    In the following, only description of payload is given.

    1. HELLO_FROM_CHILD
        + Port number of child server (used to serve other peers)
            -------------------
            | Port number (4) |
            -------------------

    2. HELLO_SUPER_TO_SUPER
        + Port number of super server (used to serve child nodes and receive shared information)
            -------------------
            | Port number (4) |
            -------------------

    3. FILE_INFO
        + Number of files (maximum 100)
        + An array of information of files including names and sizes (in bytes)
            --------------------------------------------------------------------------
            | File number (4) | Name (96) | Size (4) | ...... | Name (96) | Size (4) |
            --------------------------------------------------------------------------

    4. FILE_INFO_RECV_SUCCESS
        + Number of files (maximum 100)
        + An array of received file names
            ----------------------------------------------------
            | File number (4) | Name (96) | ...... | Name (96) |
            ----------------------------------------------------

    5. SEARCH_QUERY
        + Name of the file that child wants to download
            -------------
            | Name (96) |
            -------------

    6. SEARCH_ANS_SUCCESS
        + IP address of peer having the file
        + Port number of peer having the file
        + Size of requested file
            ---------------------------------
            | IP (20) | Port (4) | Size (4) |
            ---------------------------------

    7. FILE_REQ
        + Name of the requested file
            -------------
            | Name (96) |
            -------------

    8. FILE_RES_SUCCESS
        + Data of the file
            ----------------------------------------------
            |              FILE DATA (10000)             |
            ----------------------------------------------

    9. FILE_INFO_SHARE
        + File number
        + An array of information of files including IPs, port numbers, names and sizes
            ----------------------------------------------------------------------
            | File number (4) | IP (20) | Port (4) | Name (96) | Size (4) | ....
            ----------------------------------------------------------------------
                ----------------------------------------------------
                  .... | IP (20) | Port (4) | Name (96) | Size (4) |
                ----------------------------------------------------

    10. FILE_INFO_SHARE_SUCCESS
        This packet has the same structure as FILE_INFO_SHARE's.

    11. Others
        Other packets only have header, no payload.

IV. Notes
    1. File name in Linux has maximum length of 255 characters, but currently in the code, length of file name is limited to 95 characters.
    2. Data file from child node to child node is sent in many small packets, each with 10000 bytes in size
