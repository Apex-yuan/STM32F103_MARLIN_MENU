#include "menu.h"
#include "bsp_lcd12864.h"
#include "bsp_key.h"

#include "bsp_tim.h"

#define NULL 0

#define LCD_WIDTH 16
#define LCD_HEIGHT 4

/* Custom characters defined in font font_6x10_marlin.c */
#define LCD_STR_BEDTEMP     "\xFE"
#define LCD_STR_DEGREE      "\xB0"
#define LCD_STR_THERMOMETER "\xFF"
#define LCD_STR_UPLEVEL     "\xFB"
#define LCD_STR_REFRESH     "\xF8"
#define LCD_STR_FOLDER      "\xF9"
#define LCD_STR_FEEDRATE    "\xFD"
#define LCD_STR_CLOCK       "\xFC"
#define LCD_STR_ARROW_RIGHT "\xFA"

#define lcd_implementation_drawmenu_back_selected(row, pstr, data) lcd_implementation_drawmenu_generic(row, pstr, LCD_STR_UPLEVEL[0], LCD_STR_UPLEVEL[0])
#define lcd_implementation_drawmenu_back(row, pstr, data) lcd_implementation_drawmenu_generic(row, pstr, ' ', LCD_STR_UPLEVEL[0])
#define lcd_implementation_drawmenu_submenu_selected(row, pstr, data) lcd_implementation_drawmenu_generic(row, pstr, '>', LCD_STR_ARROW_RIGHT[0])
#define lcd_implementation_drawmenu_submenu(row, pstr, data) lcd_implementation_drawmenu_generic(row, pstr, ' ', LCD_STR_ARROW_RIGHT[0])

//Function pointer to menu functions.//指向菜单函数的函数指针
typedef void (*menuFunc_t)();

static void lcd_main_menu(void);
static void lcd_tune_menu(void);
void lcd_quick_feedback(void);
static void menu_action_back(menuFunc_t data);
static void menu_action_submenu(menuFunc_t data);


/* Helper macros for menus */
#define START_MENU() do { \
    if (encoderPosition > 0x8000) encoderPosition = 0; \
    if (encoderPosition / ENCODER_STEPS_PER_MENU_ITEM < currentMenuViewOffset) currentMenuViewOffset = encoderPosition / ENCODER_STEPS_PER_MENU_ITEM;\
    uint8_t _lineNr = currentMenuViewOffset, _menuItemNr; \
    bool wasClicked = LCD_CLICKED;\
    for(uint8_t _drawLineNr = 0; _drawLineNr < LCD_HEIGHT; _drawLineNr++, _lineNr++) { \
        _menuItemNr = 0;
#define MENU_ITEM(type, label, args...) do { \
    if (_menuItemNr == _lineNr) { \
        if (lcdDrawUpdate) { \
            const char* _label_pstr = PSTR(label); \
            if ((encoderPosition / ENCODER_STEPS_PER_MENU_ITEM) == _menuItemNr) { \
                lcd_implementation_drawmenu_ ## type ## _selected (_drawLineNr, _label_pstr , ## args ); \
            }else{\
                lcd_implementation_drawmenu_ ## type (_drawLineNr, _label_pstr , ## args ); \
            }\
        }\
        if (wasClicked && (encoderPosition / ENCODER_STEPS_PER_MENU_ITEM) == _menuItemNr) {\
            lcd_quick_feedback(); \
            menu_action_ ## type ( args ); \
            return;\
        }\
    }\
    _menuItemNr++;\
} while(0)
#define MENU_ITEM_DUMMY() do { _menuItemNr++; } while(0)
#define MENU_ITEM_EDIT(type, label, args...) MENU_ITEM(setting_edit_ ## type, label, PSTR(label) , ## args )
#define MENU_ITEM_EDIT_CALLBACK(type, label, args...) MENU_ITEM(setting_edit_callback_ ## type, label, PSTR(label) , ## args )
#define END_MENU() \
    if (encoderPosition / ENCODER_STEPS_PER_MENU_ITEM >= _menuItemNr) encoderPosition = _menuItemNr * ENCODER_STEPS_PER_MENU_ITEM - 1; \
    if ((uint8_t)(encoderPosition / ENCODER_STEPS_PER_MENU_ITEM) >= currentMenuViewOffset + LCD_HEIGHT) { currentMenuViewOffset = (encoderPosition / ENCODER_STEPS_PER_MENU_ITEM) - LCD_HEIGHT + 1; lcdDrawUpdate = 1; _lineNr = currentMenuViewOffset - 1; _drawLineNr = -1; } \
    } } while(0)

#define ENCODER_STEPS_PER_MENU_ITEM 1
#define LCD_CLICKED (keyPressed == 5)

volatile uint8_t buttons;//Contains the bits of the currently pressed buttons.??????????μ?????λ
uint8_t currentMenuViewOffset;              /* scroll offset in the current menu */ //?????????????????
uint32_t blocking_enc;
uint8_t lastEncoderBits;
uint32_t encoderPosition;
    
    
menuFunc_t currentMenu = lcd_main_menu; //lcd_status_screen; /* function pointer to the currently active menu */
uint32_t lcd_next_update_millis;
uint8_t lcd_status_update_delay;
uint8_t lcdDrawUpdate = 2;                  /* Set to none-zero when the LCD needs to draw, decreased after every draw. Set to 2 in LCD routines so the LCD gets atleast 1 full redraw (first redraw is partial) */

//prevMenu and prevEncoderPosition are used to store the previous menu location when editing settings.
//当进入编辑设置时prevMenu and prevEncoderPosition 被用来存储上次的菜单位置
menuFunc_t prevMenu = NULL;
uint16_t prevEncoderPosition;
//Variables used when editing values.
//编辑值时用到的变量
const char* editLabel;
void* editValue;
int32_t minEditValue, maxEditValue;
menuFunc_t callbackFunc;
    
    
void lcd_status_screen(void)
{
  //keyPressed = 0;
  LCD12864_Clear();
  LCD12864_ShowString(0,2,"STATUS");
}  

void lcd_quick_feedback(void)
{
  lcdDrawUpdate = 2;
}

/** Menu action functions **/
static void menu_action_back(menuFunc_t data)
{
    currentMenu = data;
    encoderPosition = 0;
}
static void menu_action_submenu(menuFunc_t data)
{
    currentMenu = data;
    encoderPosition = 0;
}    


static void lcd_implementation_drawmenu_generic(uint8_t row,  const char* pstr, char pre_char, char post_char)
{
   LCD12864_ShowString(row,0,"                ");
  LCD12864_ShowString(row, 2, (uint8_t *)pstr);
//  LCD12864_ShowString(0, 2, (uint8_t *)pstr);
//  LCD12864_ShowString(1, 2, "nihoa");
  if ((pre_char == '>') || (pre_char == LCD_STR_UPLEVEL[0] ))
  {
    LCD12864_HightlightShow(row,0,16,1);
    //LCD12864_ShowString(row,0,"->");
  }
  
}

//const char *info = "info";
//#define 

/* Menu implementation */ //主菜单
static void lcd_main_menu()
{
  START_MENU();
  MENU_ITEM(submenu, "info", lcd_tune_menu);
  MENU_ITEM(back, "info", lcd_status_screen); 
  MENU_ITEM(back, "ifgdfgdfg", lcd_status_screen);
  MENU_ITEM(back, "sddfgdfr", lcd_status_screen);
  MENU_ITEM(back, "sdfgdfr", lcd_status_screen);
  MENU_ITEM(back, "23fgdfr", lcd_status_screen);
  MENU_ITEM(back, "342gdfr", lcd_status_screen);
  MENU_ITEM(back, "45gdfr", lcd_status_screen);
  END_MENU();
 
}

static void lcd_tune_menu()
{
  START_MENU();
  MENU_ITEM(back, "Main", lcd_main_menu);
  MENU_ITEM(back, "KLain", lcd_main_menu);
  MENU_ITEM(back, "SKain", lcd_main_menu);
  MENU_ITEM(back, "SOain", lcd_main_menu);
  MENU_ITEM(back, "Main", lcd_main_menu);
  END_MENU();
}

void menu_update(void)
{
  uint8_t keyMsg = keyPressed; //记录按下的键值
  //keyPressed = 0; //按下的键值信息清零
  
  //处理按键消息
	switch(keyMsg)  //keyPressed（按下的键值） 由外部中断获得
	{
		case KEY_UP_PRESSED://上移
      encoderPosition--;
    break;
		case KEY_DOWN_PRESSED: //下移
      encoderPosition++;
    break;
		case KEY_LEFT_PRESSED://返回键
      break;
		case KEY_RIGHT_PRESSED: //进入下一级菜单
      break;
		case KEY_MID_PRESSED: //确认键
      lcdDrawUpdate = 1;
      break;
    default:
      break;
  }
  
  if(keyMsg != 0) //有按键按下，屏幕需要刷新一次
  {
    lcdDrawUpdate = 1;
  }
  (*currentMenu)();
  
  if(lcdDrawUpdate) //刷新两次相当于一次完整更新
  {
    lcdDrawUpdate--;
  }
}

