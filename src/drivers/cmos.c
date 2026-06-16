#include "drivers/cmos.h"
#include "lib/bool.h"
#include "lib/io.h"
#include <stdint.h>
uint8_t monthdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
uint8_t statusregb = 0;

void enable_nmi() {
    outb(0x70, 0xD);
    // asm volatile ("sti");
}

#define disable_nmi 0x80
uint8_t read_cmos(uint8_t reg) {
    // asm volatile ("cli");
    outb(0x70, disable_nmi | reg);
    return inb(0x71);
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

struct timestruct get_cmos_time() {
    asm volatile ("cli"); // CMOS should not be interrupted >:(
    struct timestruct time = {0};

    if (!statusregb) {
        statusregb = read_cmos(0x0B);
    }
    if (statusregb & 4) {
        while (read_cmos(0x0A) & 0x80);
        time.sec = read_cmos(0);
        time.min = read_cmos(2);
        time.hours = read_cmos(4);
        if (!(statusregb & 2) && time.hours & 0x80) {
            time.is_pm = true;
            time.hours &= 0x7F;
        }

        time.day = read_cmos(7);
        time.month = read_cmos(8);
        time.year = read_cmos(9);
        if (time.year >= 70) {
            time.year += 1900;
        } else {
            time.year += 2000;
        }

    } else {
        while (read_cmos(0x0A) & 0x80);
        time.sec = bcd_to_bin(read_cmos(0));
        time.min = bcd_to_bin(read_cmos(2));
        uint8_t hours = read_cmos(4);
        if (!(statusregb & 2) && hours & 0x80) {
            time.is_pm = true;
            hours &= 0x7F;
        }
        time.hours = bcd_to_bin(hours);

        time.day = bcd_to_bin(read_cmos(7));
        time.month = bcd_to_bin(read_cmos(8));
        time.year = bcd_to_bin(read_cmos(9));
        if (time.year >= 70) {
            time.year += 1900;
        } else {
            time.year += 2000;
        }
    }

    enable_nmi(); // preventing system from die............
    asm volatile ("sti"); // other things can be interrupted >:D
    return time;
}
nixt convert_to_nixt(struct timestruct time) {
    nixt nixtime = time.sec; // set seconds
    nixtime += 60*time.min; // set minutes
    
    uint8_t hour = time.hours;

    if (time.is_pm && hour != 12)
        hour += 12;

    if (!time.is_pm && hour == 12)
        hour = 0;

    nixtime += hour * 3600;

    nixtime += 86400*(time.day-1); // set day of the month

    // set month start
    for (int month = 1; month < time.month;month++) {
        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                nixtime += 2678400;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                nixtime += 2592000;
            break;
            case 2:
                if ((time.year % 4 == 0 && time.year % 100 != 0) || (time.year % 400 == 0)) {
                    nixtime += 2505600;
                } else {
                    nixtime += 2419200;
                }
                break;
        }
    }
    // set month end

    // set year start
    for (int year = 1970; year < time.year;year++) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            nixtime += 31622400;
        } else {
            nixtime += 31536000;
            
        }
    }
    // set year end
    return nixtime;
}
struct timestruct convert_to_timestruct(nixt nixtime, bool pm_enabled) {
    struct timestruct time;

    time.sec = nixtime % 60;
    nixtime /= 60;

    time.min = time.sec % 60;
    nixtime /= 60;

    time.hours = time.min % 24;
    if (pm_enabled && time.hours > 12) {
        time.hours -= 12;
        time.is_pm = true;
    }
    nixtime /= 24;

    time.day = nixtime;

    time.year = 1970;
    while (true) {
        bool leap = (time.year % 4 == 0 && time.year % 100 != 0) || (time.year % 400 == 0);
        uint16_t days = leap ? 366 : 365;
        if (time.day >= days) {
            time.day -= days;
            time.year++;
        } else {break;}
    }

    bool leap = (time.year % 4 == 0 && time.year % 100 != 0) || (time.year % 400 == 0);
    time.month = 1;
    for (uint8_t i = 0; i < 12;i++) {
        uint16_t days = monthdays[i];
        if (leap && days == 28) {days++;}
        if (time.day >= days) {
            time.month++;
            time.day -= days;
        } else {
            break;
        }
    }

    return time;
}