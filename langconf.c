#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "langconf.h"

static FILE *langfile = NULL;

struct langinfo *deflang;

struct langinfo *langs;
int langcount;

struct fontinfo *fonts;
int fontcount;

static int get_sh_value(const char *var, char *dst)
{
	static char buf[256];
	char *line, *value;

	rewind(langfile);
	while ((line = fgets(buf, sizeof(buf), langfile)))
	{
		while (isspace(*line))
			line++;
		while (isspace(line[strlen(line)-1]))
			line[strlen(line)-1] = '\0';

		if (*line == '#' || (value = strchr(line, '=')) == NULL)
			continue;

		*value++ = '\0';

		if (strcmp(line, var))
			continue;

		if (*value == '"' && value[strlen(value)-1] == '"')
		{
			value++;
			value[strlen(value)-1] = '\0';
		}

		strcpy(dst, value);

		return 0;
	}

	*dst = '\0';

	return 1;
}

static void add_language(const char *name, int loc)
{
	char buf[50];

	strcpy(langs[loc].shortname, name);

	sprintf(buf, "%s_name", name);
	get_sh_value(buf, langs[loc].name);

	sprintf(buf, "%s_font", name);
	get_sh_value(buf, langs[loc].font);

	sprintf(buf, "%s_bitmapmenu", name);
	if (!get_sh_value(buf, buf))
	        langs[loc].bitmapmenu = !strcasecmp(buf, "yes") || !strcasecmp(buf, "true")
				     || !strcasecmp(buf, "1");
}

static void add_font(const char *name, int loc)
{
	strcpy(fonts[loc].font, name);
}

static int cmp_languages(const void *a, const void *b)
{
	return strcmp(((const struct langinfo*)a)->name,
		      ((const struct langinfo*)b)->name);
}

static int cmp_fonts(const void *a, const void *b)
{
	return strcmp(((const struct fontinfo*)a)->font,
		      ((const struct fontinfo*)b)->font);
}

int find_language(const char *name)
{
	int i;

	for (i = 0; i < langcount; i++)
	{
		if (!strcmp(name, langs[i].name))
			return i;
	}

	return -1;
}

void init_langconf(void)
{
	char buf[256], *tmp;
	int i;

	langfile = fopen(PATH_LANGCONF, "r");
	if (langfile == NULL)
	{
		perror("fopen");
		exit(1);
	}

	if (get_sh_value("LANGUAGES", buf) || buf[0] == '\0')
	{
		printf("ERROR: no languages found.\n");
		exit(1);
	}

	langcount = 1;
	while ((tmp = strrchr(buf, ' ')))
	{
		*tmp = '\0';
		langcount++;
	}

	langs = (struct langinfo *)malloc(sizeof(struct langinfo) * langcount);
	if (langs == NULL)
		exit(1);

	for (tmp = buf, i = 0; i < langcount && *tmp; tmp += strlen(tmp)+1, i++)
		add_language(tmp, i);

	qsort(langs, langcount, sizeof(struct langinfo), cmp_languages);

	get_sh_value("DEFAULT_LANGUAGE", buf);
	for (i = 0; i < langcount; i++)
		if (!strcmp(langs[i].shortname, buf))
			deflang = &langs[i];

	if (get_sh_value("FONTS", buf) || buf[0] == '\0')
	{
		printf("ERROR: no fonts found.\n");
		exit(1);
	}

	fontcount = 1;
	while ((tmp = strrchr(buf, ' ')))
	{
		*tmp = '\0';
		fontcount++;
	}

	fonts = (struct fontinfo *)malloc(sizeof(struct fontinfo) * fontcount);
	if (fonts == NULL)
	{
		printf("ERROR: Failed to initilize fonts.\n");
		exit(1);
	}

	for (tmp = buf, i = 0; i < fontcount && *tmp; tmp += strlen(tmp)+1, i++)
	{
		add_font(tmp, i);
	}

	qsort(fonts, fontcount, sizeof(struct fontinfo), cmp_fonts);

	fclose(langfile);
	langfile = NULL;
}

void free_langconf(void)
{
	if (langs)
		free(langs);
	if (fonts)
		free(fonts);

	langcount = 0;
	fontcount = 0;
	deflang = NULL;
	langs = NULL;
	fonts = NULL;
}
