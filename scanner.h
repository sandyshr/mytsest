#ifndef SCANNER_H
#define SCANNER_H
#include <netinet/in.h> //needed for struct in_addr



//Constant Definition
#define MAX_IP 256
#define BUF_SIZE 1024

typedef enum {SINGLE, CSV, CIDR} Request_Type;

typedef enum {OPEN, CLOSED, FILTERED, UNK} Port_State;

typedef enum {UP, DOWN, UNKNOWN} IP_Status;
//Header file for scanner 

typedef struct port_struct {
    int port_num;
    struct servent * port_info;
    Port_State port_state;
} Port_Struct;

typedef struct target {
    struct in_addr ip_addr;
    Port_Struct * port_list; //list of ports
    int port_count;
    IP_Status status;
} Target;


//List of all target
typedef struct target_list {
    struct target *targets;
    int target_count;
    Request_Type ip_req_type; //single ip, range of ip, or csv ip
} TargetList;


//this function will parse the ip_address inputs
//will call other helper functions depending on if the ip address is cider notation, single ip address or csv ip address. 

TargetList *get_target_list(char * ip_input);
Port_Struct *get_port_list(char * port_input);
TargetList *init_target_list();
void ping_check(void * arg);



/*these are the helper function*/
TargetList *parse_csv_ip_addrs(char * ip_input);
TargetList *parse_cidr_ip_addrs(char *ip_input);
TargetList *parse_single_ip_addr(char *ip_input);

//ports functions
Port_Struct *parse_csv_ports(char *port_input);
Port_Struct *parse_single_port(char *port_input);
Port_Struct *parse_range_ports(char *port_input);



void print_target_list(TargetList *target_list);


#endif