NOTE:
- File name in Linux has maximum length of 255 characters, but currently in the code the length of file name is limited to 95 characters.
- Only the second super node sends HELLO_SUPER_TO_SUPER message to the first super node. Currently the first super node does not reply to this message.
- When second super node sends HELLO_SUPER_TO_SUPER message, it includes its server port number.
