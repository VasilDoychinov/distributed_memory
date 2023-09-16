// conversions.cpp: miscellaneous like IP address from text to int, etc


#include "conversions.h"


// private helpers
#define SWAP_UC(uc1, uc2)   ((uc1) ^= (uc2), (uc2) ^= (uc1), (uc1) ^= (uc2))
static uint32_t reorder_ui(uint32_t ui) ;


// public interface

uint32_t                                                    // as /24 to ui
mask_to_ui(uint8_t mask)                                       // no validity
{
    uint32_t   wm = 0xFFFFFFFF ;
    for (int i = 0 ; i < MAX_MASK_L - mask ; ++i)   UNSET_BIT(wm, i) ;
    return wm ;
}

std::string                                                 //
get_broadcast_address(const uint8_t* ip_addr, uint8_t mask)
{
    uint32_t   wuip = get_ip_integer_equivalent(ip_addr) ;
    wuip |= ~(mask_to_ui(mask)) ;

    return get_abcd_ip_format(wuip) ;
}


uint32_t                                                    // @return:
get_ip_integer_equivalent(const uint8_t* ip_addr)              // network UI
{
    uint32_t   wui = host_ip_to_ui(ip_addr) ;
    return reorder_ui(wui) ; // htonl(wui) ;
}

std::string                                                        // from network UI
get_abcd_ip_format(uint32_t ip)
{
    char w_buff[sizeof(ip) * 6 + 1] ;   // ?? must suffice
    uint8_t* output_buffer = (uint8_t *)w_buff ;

    void*          pn = &ip ;
    uint8_t* ptr = static_cast<uint8_t*>(pn) ; ptr += sizeof(ip) - 1 ;

    for (uint32_t i = 0 ; i < sizeof(ip) ; ++i, --ptr) {
        output_buffer = ui_to_digits(*((uint8_t*)ptr), output_buffer) ;
        *output_buffer++ = '.' ;
    }
    *(--output_buffer) = 0 ;

    return std::string{w_buff} ;
}

std::string                                                        //
get_network_id(const uint8_t* ip_addr, uint8_t mask)
{
    uint32_t   wuip = get_ip_integer_equivalent(ip_addr) ;
    wuip &= mask_to_ui(mask) ;

    return get_abcd_ip_format(wuip) ;
}

uint32_t                                                    //
get_subnet_cardinality(uint8_t mask)
{  // max # = (2 ^ (32 - mask)) - 2
    return (1 << (32 - (uint32_t)mask)) - 2 ;
}

bool                                                         //
check_ip_subnet_membership(const uint8_t* network_id, uint8_t mask, const uint8_t* check_ip)
{
    uint32_t   check_u = get_ip_integer_equivalent(check_ip) ;
    check_u &= mask_to_ui(mask) ;

    uint32_t   net_u= get_ip_integer_equivalent(network_id) ;

    return net_u == check_u ? 0 : -1 ;
}


// (private) helpers

uint32_t                                                    // ip to Host UI
host_ip_to_ui(const uint8_t* ip_addr)
{
    uint32_t   wui = 0 ; void*  pn = &wui ;
    uint8_t*   ptr = static_cast<uint8_t *>(pn) ;
    for (uint32_t i = 0 ; i < sizeof(wui) ; ++i, ++ptr) {
        *((uint8_t *)ptr) = digits_to_ui(&ip_addr) ;
        ++ip_addr  ; // was over the delimiter - move it
    }
    return wui ;
}

uint8_t
digits_to_ui(const uint8_t** digs)                                   // seq of Digits to uchar
{
    const uint8_t*     d = *digs ;
    uint8_t            num = 0 ;
    for ( ; *d >= '0' && *d <= '9' ; ++d) {
        num = num * 10 + (*d - '0') ;
    }
    *digs = d ;
    return num ;
}

uint8_t*                                              // @return - end()
ui_to_digits(uint8_t num, uint8_t* digs)                 // UChar to digits
{
    char   txt[4] = { 0, 0, 0, 0 } ;
    char*  p = txt ;
    do {
        *p++ = '0' + num % 10, num /= 10 ;
    } while (num != 0) ;

    for (--p ; p >= txt ; --p)   *digs++ = *p ;
    return digs ;
}

static uint32_t reorder_ui(uint32_t ui) { // as htonl(uint32_t)
    void *ptr = &ui ;
    uint8_t* pb = static_cast<uint8_t *>(ptr) ;
    uint8_t* pe = pb + sizeof(ui) - 1 ;

    SWAP_UC(*pb, *pe), SWAP_UC(*(pb + 1), *(pe - 1)) ;
    return ui ;
}


// eof conversions.cpp
