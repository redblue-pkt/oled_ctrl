/*
 *   Copyright (C) redblue 2019
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>
#include "oled_main.h"
#include "oled_freetype.h"
#include "oled_driver.h"
#include "oled_picture.h"

typedef struct
{
	char *arg;
	char *arg_long;
	char *arg_description;
} tArgs;

tArgs vArgs[] =
{
	{ "-b", " --setBrightness		", "Args: brightness\n\tSet oled brightness" },
	{ "-c", " --clear			", "Args: No argumens\n\tClear oled display" },
        { "-d", " --deepStandby		", "Args: No argumens\n\tEnter deep standby" },
	{ "-p", " --showPicture		", "Args: picture location\n\tShow picture" },
	{ "-tu", " --setTextUp		", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in up (Args2 are optional)" },
	{ "-tc", " --setTextCenter		", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in center (Args2 are optional)" },
	{ "-td", " --setTextDown		", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in down (Args2 are optional)" },
	{ "-tud", " --setTextUpDifferent	", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in up (Args2 are optional)" },
	{ "-tcd", " --setTextCenterDifferent	", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in center (Args2 are optional)" },
	{ "-tdd", " --setTextDownDifferent	", "Args1: text\tArgs2: color in hex\n\tSet color text to oled in down (Args2 are optional)" },
	{ NULL, NULL, NULL }
};

void usage(char *prg, char *cmd)
{
	int i;

	FILE *fvu = fopen("/proc/stb/info/vumodel", "r");
	if (fvu) {
		char tmp[63];
		if (fscanf(fvu, "%s", &tmp) == 1) {
			strcpy(vumodel, tmp);
		}
		fclose(fvu);
	}

	/* or printout a default usage */
	fprintf(stderr, "Oled control tool, version 1.01\n");
	if (fvu)
	{
		fprintf(stderr, "Found vu stb: %s\n", vumodel);
	}
	else
	{
		fprintf(stderr, "This stb is not supported\n");
		exit(1);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "General usage:\n\n");
	fprintf(stderr, "%s argument [optarg1] [optarg2] [optarg3]\n", prg);

	for (i = 0; ; i++)
	{
		if (vArgs[i].arg == NULL)
			break;
		if ((cmd == NULL) || (strcmp(cmd, vArgs[i].arg) == 0) || (strstr(vArgs[i].arg_long, cmd) != NULL))
			fprintf(stderr, "%s %s %s\n", vArgs[i].arg, vArgs[i].arg_long, vArgs[i].arg_description);
	}
	exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	if (argc > 1)

	{
		i = 1;
		while (i < argc)
		{
			if ((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--setBrightness") == 0))
			{
				if (i + 1 <= argc)
				{
					int brightness;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing brightness value\n");
						usage(argv[0], NULL);
					}
					brightness = atoi(argv[i + 1]);
					if (brightness < 0 || brightness > 10)
					{
						fprintf(stderr, "Brightness value out of range\n");
                                        	usage(argv[0], NULL);
					}
					/* set display brightness */
					lcd_brightness(brightness);
				}
				i += 1;
			}
			else if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--clear") == 0))
			{
				/* clear the display */
				driver_init();
				lcd_clear();
			}
			else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--deepStandby") == 0))
			{
				/* enter in deep standby */
				driver_init();
				lcd_deepstandby();
			}
			else if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--showPicture") == 0))
			{
				if (i + 1 <= argc)
				{
					const char *filename;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing filename value\n");
						usage(argv[0], NULL);
					}
					filename = argv[i + 1];
					/* set display picture */
					driver_init();
					lcd_clear();
					show_image(filename);
				}
				i += 1;
			}
			else if ((strcmp(argv[i], "-tu") == 0) || (strcmp(argv[i], "--setTextUp") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_UP_COLOR;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					lcd_clear();
					init_freetype();
					lcd_print_text_up(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else if ((strcmp(argv[i], "-tc") == 0) || (strcmp(argv[i], "--setTextCenter") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1]== NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_CENTER_COLOR;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					lcd_clear();
					init_freetype();
					lcd_print_text_center(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else if ((strcmp(argv[i], "-td") == 0) || (strcmp(argv[i], "--setTextDown") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_DOWN_COLOR;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					lcd_clear();
					init_freetype();
					lcd_print_text_down(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else if ((strcmp(argv[i], "-tud") == 0) || (strcmp(argv[i], "--setTextUpDifferent") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_UP_COLOR_DIFFERENT;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					lcd_clear();
					init_freetype();
					lcd_print_text_up_different(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else if ((strcmp(argv[i], "-tcd") == 0) || (strcmp(argv[i], "--setTextUpDifferent") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_CENTER_COLOR_DIFFERENT;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					init_freetype();
					lcd_print_text_center_different(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else if ((strcmp(argv[i], "-tdd") == 0) || (strcmp(argv[i], "--setTextDownDifferent") == 0))
			{
				if (i + 2 <= argc)
				{
					const char *text;
					const char *tmp_color;
					int color;
					if (argv[i + 1] == NULL)
					{
						fprintf(stderr, "Missing text value\n");
						usage(argv[0], NULL);
					}
					if (argv[i + 2] == NULL)
					{
						color = LCD_DOWN_COLOR_DIFFERENT;
					}
					else
					{
						tmp_color = argv[i + 2];
						color = (int)strtol(tmp_color, NULL, 0);
					}
					text = argv[i + 1];
					/* set display text */
					driver_init();
					lcd_clear();
					init_freetype();
					lcd_print_text_down_different(text, color, 0, TEXT_ALIGN_CENTER);
					lcd_draw_text();
				}
				i += 2;
			}
			else
			{
				usage(argv[0], NULL);
			}
			i++;
		}
	}
	else
	{
		usage(argv[0], NULL);
	}
	return 0;
}
