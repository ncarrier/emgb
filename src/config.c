#define _GNU_SOURCE
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "config.h"

int config_init(struct config *config)
{
	int ret;

	snprintf(config->dir, PATH_MAX, "%s/" CONFIG_DIR,
			getenv("HOME"));
	ret = mkdir(config->dir
#ifndef _WIN32
			, 0755
#endif
			);
	if (ret < 0 && errno != EEXIST) {
		fprintf(stderr, "mkdir: %m\n");
		return -ENOMEM;
	}
	ret = asprintf(&config->file, "%s/config",
			config->dir);
	if (ret == -1) {
		fprintf(stderr, "asprintf: %s\n", strerror(ENOMEM));
		return -ENOMEM;
	}

	ret = ae_config_read(&config->config, config->file);
	if (ret == 0)
		return 0;
	if (ret != -ENOENT) {
		fprintf(stderr, "ae_config_read: %s\n", strerror(-ret));
		return -ret;
	}

	return ae_config_read_from_string(&config->config, "");
}

int config_write(struct config *config)
{
	return ae_config_write(&config->config, config->file);
}

void config_cleanup(struct config *config)
{
	ae_config_cleanup(&config->config);
	if (config->file != NULL)
		free(config->file);
}
