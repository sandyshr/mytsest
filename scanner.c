#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <math.h>
#include <errno.h>
#include <netdb.h>

#include "scanner.h"

extern TargetList *my_target_list;
//Steps:
/*
1. create a function that will accepts single ip address, cidr notation, or csv ip addresses and print them. 
 
*/
//helper function to print a TargetList
void print_target_list(TargetList *target_list) {
    printf("Request Type: %d\n", target_list->ip_req_type);    
    printf("Target Count: %d\n", target_list->target_count);
    for (int i = 0; i < target_list->target_count; i++) {
        printf("Target %d: IP = %s, Status = %s\n", i+1,
        inet_ntoa(target_list->targets[i].ip_addr),
        target_list->targets[i].status==0 ? "Up!" : "Down!");
    }
}


TargetList *get_target_list(char * ip_input){
    TargetList * target_list;
    //first finds out if its csv, range or single ip
    if (strchr(ip_input, ',') != NULL) { //i.e , is found
        target_list = parse_csv_ip_addrs(ip_input);
    } else if (strchr(ip_input, '/') != NULL) {
        target_list = parse_cidr_ip_addrs(ip_input);
    } else {
        target_list = parse_single_ip_addr(ip_input);
    }
    return target_list;
}


//initialize the struct TargetList
//returns a pointer to the new_target_list
TargetList *init_target_list(){
    TargetList *new_target_list = malloc(sizeof(TargetList));
    new_target_list->ip_req_type = -1;
    new_target_list->target_count=0;
    new_target_list->targets = NULL;
    return new_target_list;
}


Target *init_target() {
    Target *new_target = malloc(sizeof(Target));
    new_target->ip_addr.s_addr = 0;
    new_target->port_count = 0;
    new_target->port_list = NULL;
    new_target->status = UNKNOWN;
    return new_target;
}






//

TargetList *parse_cidr_ip_addrs(char *ip_input){
    TargetList *target_list = init_target_list();
    target_list->ip_req_type = CIDR;

    char *network_bit_str = strtok(ip_input, "/");
    char *host_bit_str = strtok(NULL, "/");

    // Convert IP address string to binary representation
    struct in_addr net_addr;
    if (inet_pton(AF_INET, network_bit_str, &net_addr) != 1) {//network portion
        printf("Error: invalid IP address '%s'\n", network_bit_str);
        return NULL;
    }

    // Convert prefix length string to integer
    int prefix_len = atoi(host_bit_str);
    if (prefix_len < 0 || prefix_len > 32) {
        printf("Error: invalid prefix length '%s'\n", host_bit_str);
        return NULL;
    }

    // Calculate number of IP addresses in range
    int num_addrs = pow(2, 32 - prefix_len);
    target_list->target_count = num_addrs;

    // Allocate memory for target list
    target_list->targets = malloc(num_addrs * sizeof(Target));
    if (target_list->targets == NULL) {
        printf("Error: unable to allocate memory for target list\n");
        return NULL;
    }

    // Initialize target list with IP addresses
    for (int i = 0; i < num_addrs; i++) {
        struct in_addr target_ip_addr;
        target_ip_addr.s_addr = net_addr.s_addr + htonl(i);
        Target *target = &target_list->targets[i];
        target->ip_addr = target_ip_addr;
        target->port_list = NULL; //we dont know yet
        target->status = UNKNOWN; //we will call id down for now
    }

    return target_list;
}


//
TargetList *parse_csv_ip_addrs(char * ip_input){
    //TODO

    return NULL;
}

TargetList *parse_single_ip_addr(char* ip_input){
    //TODO
    return NULL;
}


////following function deals with port parsing
Port_Struct *get_port_list(char * port_input) {
    Port_Struct * new_port_struct_ptr = {0};
    //first finds out if its csv, range or single ip
    if (strchr(port_input, ',') != NULL) { //i.e , is found
        new_port_struct_ptr = parse_csv_ports(port_input);
    } else if (strchr(port_input, '-') != NULL) {
        new_port_struct_ptr = parse_range_ports(port_input);
    } else {
        new_port_struct_ptr = parse_single_port(port_input);
    }
    return new_port_struct_ptr;
    //loo
}

Port_Struct* parse_range_ports(char* port_input) {
    Port_Struct* port_list = NULL;
    char* tok;
    int start_port, end_port, total_ports;

    tok = strtok(port_input, "-");
    if (tok != NULL) {
        start_port = strtol(tok, NULL, 10);
        if (errno != 0) {
            perror("strtol");
            return NULL;
        }

        tok = strtok(NULL, "-");
        if (tok != NULL) {
            end_port = strtol(tok, NULL, 10);
            if (errno != 0) {
                perror("strtol");
                return NULL;
            }
        } else {
            end_port = start_port;
        }
    } else {
        printf("Invalid port range!\n");
        return NULL;
    }

    if (end_port < start_port) {
        printf("Invalid port range!\n");
        return NULL;
    }

    total_ports = end_port - start_port + 1;
    port_list = malloc(sizeof(Port_Struct) * total_ports);
    if (port_list == NULL) {
        perror("malloc");
        return NULL;
    }

    for (int i = 0; i < total_ports; i++) {
        port_list[i].port_num = start_port;
        port_list[i].port_state = UNK;

        port_list[i].port_info = malloc(sizeof(struct servent)); 
        struct servent * se = getservbyport(htons(start_port), NULL);
        if (se != NULL) {
            port_list[i].port_info->s_aliases = se->s_aliases;
            port_list[i].port_info->s_name = strdup( se->s_name);
            port_list[i].port_info->s_port = se->s_port;
            port_list[i].port_info->s_proto= strdup(se->s_proto);
        } else { //fill some info
            port_list[i].port_info->s_aliases = NULL;
            port_list[i].port_info->s_name = strdup("UNK");
            port_list[i].port_info->s_port = ntohs(start_port);
            port_list[i].port_info->s_proto= strdup("UNK");
        }

        start_port++;
    }
    my_target_list->targets->port_count = total_ports;
    return port_list;
}


Port_Struct *parse_csv_ports(char *port_input){
    return NULL;
}
Port_Struct *parse_single_port(char *port_input){
    return NULL;
}




//copying it in main.c
// void ping_check(void * arg) {
//     TargetList *target_list = (TargetList *)arg;
//     for (int i = 0; i < target_list->target_count; i++) {
//         Target *target = &target_list->targets[i];
//         char cmd[256];
//         sprintf(cmd, "ping -c 1%s > /dev/null", inet_ntoa(target->ip_addr));
//         int ret = system(cmd);
//         if (ret == 0) {
//             target->status = UP;
//             printf("%s is UP\n", inet_ntoa(target->ip_addr));
//         } else {
//             target->status = DOWN;
//             printf("%s is DOWN\n", inet_ntoa(target->ip_addr));
//         }
//     }
// }






