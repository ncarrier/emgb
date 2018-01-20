#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include "platform.h"
#else
#include <argz.h>
#include <envz.h>
#endif

#include "ae_config.h"
#include "utils.h"

static void file_cleanup(FILE **f)
{
	if (f == NULL || *f == NULL)
		return;

	fclose(*f);
	*f = NULL;
}

static void string_cleanup(char **s)
{
	if (s == NULL || *s == NULL)
		return;

	free(*s);
	*s = NULL;
}

int ae_config_read(struct ae_config *conf, const char *fmt, ...)
{
	int ret;
	char cleanup(string_cleanup)*string = NULL;
	char cleanup(string_cleanup)*path = NULL;
	FILE cleanup(file_cleanup)*f = NULL;
	long size;
	size_t sret;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&path, fmt, args);
	if (ret == -1) {
		path = NULL;
		return -ENOMEM;
	}
	va_end(args);
	f = fopen(path, "rbe");
	if (f == NULL)
		return -errno;

	/* compute the size of the file */
	ret = fseek(f, 0, SEEK_END);
	if (ret == -1)
		return -errno;
	size = ftell(f);
	if (ret == -1)
		return -errno;
	ret = fseek(f, 0, SEEK_SET);
	if (ret == -1)
		return -errno;

	/* read all */
	string = calloc(size, 1);
	if (string == NULL)
		return -errno;

	sret = fread(string, 1, size, f);
	if (sret < (size_t)size)
		return feof(f) ? -EIO : ret;

	return ae_config_read_from_string(conf, string);
}

int ae_config_read_from_string(struct ae_config *conf, const char *string)
{
	return -argz_create_sep(string, '\n', &conf->argz, &conf->len);
}

const char *ae_config_get(const struct ae_config *conf, const char *key)
{
	return envz_get(conf->argz, conf->len, key);
}

const char *ae_config_get_default(const struct ae_config *conf, const char *key,
		const char *def)
{
	const char *ret;

	ret = envz_get(conf->argz, conf->len, key);
	if (ret == NULL)
		return def;

	return ret;
}

int ae_config_get_int(const struct ae_config *conf, const char *key, int def)
{
	const char *ret;

	ret = envz_get(conf->argz, conf->len, key);
	if (ret == NULL)
		return def;

	return atoi(ret);
}

int ae_config_add(struct ae_config *conf, const char *key, const char *value)
{
	return -envz_add(&conf->argz, &conf->len, key, value);
}

int ae_config_add_int(struct ae_config *conf, const char *key, int value)
{
	char str_value[100];

	snprintf(str_value, 100, "%d", value);

	return ae_config_add(conf, key, str_value);
}

void ae_config_cleanup(struct ae_config *conf)
{
	free(conf->argz);
	memset(conf, 0, sizeof(*conf));
}

int ae_config_write(const struct ae_config *conf, const char *fmt, ...)
{
	int ret;
	char cleanup(string_cleanup)*path = NULL;
	FILE cleanup(cleanup_file)*f = NULL;
	const char *entry;
	size_t sret;
	size_t len;
	const char newline = '\n';

	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&path, fmt, args);
	if (ret == -1) {
		path = NULL;
		return -ENOMEM;
	}
	va_end(args);

	f = fopen(path, "wbe");
	if (f == NULL)
		return -errno;

	entry = NULL;
	while ((entry = argz_next(conf->argz, conf->len, entry)) != NULL) {
		len = strlen(entry);
		sret = fwrite(entry, 1, len, f);
		if (sret != len)
			return -EIO;
		sret = fwrite(&newline, 1, 1, f);
		if (sret != 1)
			return -EIO;
	}

	return 0;
}
