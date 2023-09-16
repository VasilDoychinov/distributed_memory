// conversions.h:


#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <stdint.h>
#include <string>

#define PREFIX_LEN      16        // the max length of an IP address (\0 incl.)
#define MAX_MASK_L      32        // for IPv4 - 32 bits; mask in [0, 32]

// macros for bit operaations

#define IS_BIT_SET(n, pos)   (((n) & (1 << (pos))) != 0)
#define SET_BIT(n, pos)      (n |= (1 << (pos)))
#define UNSET_BIT(n, pos)    (n &= ~(1 << (pos)))
#define TOGGLE_BIT(n, pos)   (n ^= (1 << (pos)))


std::string get_broadcast_address(const uint8_t* ip_addr, uint8_t mask) ;

// ip to integer network format
uint32_t get_ip_integer_equivalent(const uint8_t* ip_addr) ;

std::string get_abcd_ip_format(uint32_t ip) ;

std::string get_network_id(const uint8_t* ip_addr, uint8_t mask) ;

uint32_t get_subnet_cardinality(uint8_t mask) ;

bool check_ip_subnet_membership(uint8_t* network_id, uint8_t mask, uint8_t* check_ip) ;

// helpers

uint32_t host_ip_to_ui(const uint8_t* ip_addr) ;
uint32_t mask_to_ui(uint8_t mask) ;  // as /24 to ui: no validity checks

uint8_t  digits_to_ui(const uint8_t**) ;
uint8_t* ui_to_digits(uint8_t, uint8_t*) ;


#endif // CONVERSIONS_H


// eof conversions.h
