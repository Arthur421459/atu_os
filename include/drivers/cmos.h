#ifndef CMOS_H
#define CMOS_H
#include "lib/main.h"
struct timestruct {
    uint8_t sec;
    uint8_t min;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    bool is_pm;
};
typedef uint32_t nixt;
nixt convert_to_nixt(struct timestruct time);
struct timestruct convert_to_timestruct(nixt nixtime, bool pm_enabled);
struct timestruct get_cmos_time();
void enable_nmi();
#endif