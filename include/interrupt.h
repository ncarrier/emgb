#ifndef INTERRUPT
#define INTERRUPT

struct gb;
void doInterupt(struct gb *gb_s);
void timer(struct gb *gb_s);
void serial(struct gb *gb_s);
void joypad(struct gb *gb_s);
void lcd(struct gb *gb_s);
void vblank(struct gb *gb_s);

#define INT_VBLANK (1 << 0)
#define INT_LCDSTAT (1 << 1)
#define INT_TIMER (1 << 2)
#define INT_SERIAL (1 << 3)
#define INT_JOYPAD (1 << 4)

struct s_interupt {
	unsigned char interMaster;
	unsigned char interFlag;
	unsigned char interEnable;
};

#endif
