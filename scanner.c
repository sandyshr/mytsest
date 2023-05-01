#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <math.h>
#include "scanner.h"


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
        target->status = DOWN; //we will call id down for now
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




void ping_check(TargetList *target_list){
    
}





