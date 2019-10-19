#ifndef __MENU_H
#define __MENU_H

#include <stdbool.h>

#define	PSTR(s) ((const char*)(s)) //声明一个静态指针，该指针指向数据存储区的一串字符串常量s //P代表指针，STR代表字符串 

void menu_update(void);

#endif /* __MENU_H */

