
enum stonith_cmd {
    STON_ECHO_REQUEST = 1,
    STON_ECHO_REPLY = 2,
    STON_REBOOT = 3,
    STON_POWEROFF = 4,
};

struct stonith_message {
    enum stonith_cmd command;
};
