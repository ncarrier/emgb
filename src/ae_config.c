#define _GNU_SOURCE
#include <stdlib.h>
#include <limits.h>
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

static int ae_config_add(struct ae_config *conf, const char *key,
		const char *value)
{
	return -envz_add(&conf->argz, &conf->len, key, value);
}

int ae_config_read(struct ae_config *conf, const char *path)
{
	char cleanup(cleanup_string)*string = NULL;
	FILE cleanup(cleanup_file)*f = NULL;
	long size;
	size_t sret;

	f = fopen(path, "rbe");
	if (f == NULL)
		return -errno;

	/* compute the size of the file */
	size = get_file_size(f);
	if (size < 0)
		return size;

	/* read all */
	string = calloc(size + 1, 1);
	if (string == NULL)
		return -errno;

	sret = fread(string, 1, size, f);
	if (sret < (size_t)size)
		return -EIO;

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

const char *ae_config_get_default(struct ae_config *conf, const char *key,
		const char *def)
{
	const char *ret;

	ret = envz_get(conf->argz, conf->len, key);
	if (ret == NULL) {
		ae_config_add(conf, key, def);
		return def;
	}

	return ret;
}

int ae_config_get_int(struct ae_config *conf, const char *key, int def)
{
	const char *ret;
	long value;
	char *endptr;
	char def_str[50];

	ret = envz_get(conf->argz, conf->len, key);
	if (ret == NULL || *ret == '\0')
		goto err;

	value = strtol(ret, &endptr, 0);
	if (*endptr != '\0') {
		fprintf(stderr, "Invalid integer value '%s' for key %s\n", ret,
				key);
		goto err;
	}
	if (value > INT_MAX || value < INT_MIN) {
		fprintf(stderr, "Out of range integer value '%s' for key %s\n",
				ret, key);
		goto err;
	}

	return value;
err:
	snprintf(def_str, 50, "%d", def);
	ae_config_add(conf, key, def_str);
	return def;
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

int ae_config_write(const struct ae_config *conf, const char *path)
{
	FILE cleanup(cleanup_file)*f = NULL;
	const char *entry;
	size_t sret;
	size_t len;
	const char newline = '\n';

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
