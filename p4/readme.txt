I. How to run the program
    The program is designed to run following the description in the assignment. The general flow is:
    1. Run 2 super nodes (exactly 2 super nodes)
    2. Create necessary directories and copy necessary files
    3. Run child nodes to connect to super nodes
    4. Enter command 'get [file_name] [destination_file_name]' to download file from peers

II. Description of messages
    The structure of header is given as follow. The number in round brackets describes size of the field in bytes.
        --------------------------------------------
        | Total length (4) | ID (4) | MSG Type (4) |
        --------------------------------------------
    All following packets has header, which includes Total Length, ID of the node, and MSG Type, as specified in description of the assignment.
    In the followings, only description of payload is given.

    1. HELLO_FROM_CHILD
        -------------------
        | Port number (4) |
        -------------------
        + Port number of child server (used to serve other peers)

    2. HELLO_SUPER_TO_SUPER
        -------------------
        | Port number (4) |
        -------------------
        + Port number of super server (used to serve child nodes and receive shared information)

    3. FILE_INFO
        --------------------------------------------------------------------------
        | File number (4) | Name (96) | Size (4) | ...... | Name (96) | Size (4) |
        --------------------------------------------------------------------------
        + Number of files (maximum 100)
        + An array of information of files including names and sizes (in bytes)

    4. FILE_INFO_RECV_SUCCESS
        ----------------------------------------------------
        | File number (4) | Name (96) | ...... | Name (96) |
        ----------------------------------------------------
        + Number of files (maximum 100)
        + An array of received file names

    5. SEARCH_QUERY
        -------------
        | Name (96) |
        -------------
        + File name to get info

    6. SEARCH_ANS_SUCCESS
        ---------------------------------
        | IP (20) | Port (4) | Size (4) |
        ---------------------------------
        + IP address of peer having the file
        + Port number of peer having the file
        + Size of requested file

    7. FILE_REQ
        -------------
        | Name (96) |
        -------------
        + Name of the requested file

    8. FILE_RES_SUCCESS
        ---------------------
        | FILE DATA (10000) |
        ---------------------
        + Data of the file

    9. FILE_INFO_SHARE
        --------------------------------------------------------------------------------------------------------------------
        | File number (4) | IP (20) | Port (4) | Name (96) | Size (4) | ...... | IP (20) | Port (4) | Name (96) | Size (4) |
        --------------------------------------------------------------------------------------------------------------------
        + File number
        + An array of information of files including IPs, port numbers, names and sizes

    10. FILE_INFO_SHARE_SUCCESS
        The same structure as FILE_INFO_SHARE.

    11. Others
        Other messages only have header, no payload.

III. Notes:
    1. File name in Linux has maximum length of 255 characters, but currently in the code the length of file name is limited to 95 characters.
    2. Data file from child node to child node will be sent in many small packets, each with 10000 bytes in size
