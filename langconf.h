#ifndef LANGCONF_H_
#define LANGCONF_H_

#define PATH_LANGCONF "language/lang.conf"

struct langinfo {
	char shortname[5];
	char name[50];
	char font[50];
	int bitmapmenu;
};

struct fontinfo {
	char font[50];
};

extern int langcount;		 /* count of languages in array */
extern struct langinfo *langs;	 /* array of languages */

extern struct langinfo *deflang; /* points to the default language */

extern int fontcount;
extern struct fontinfo *fonts;

void free_langconf(void);
void init_langconf(void);

int find_language(const char *name);

#endif
