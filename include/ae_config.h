#ifndef AE_CONFIG_H_
#define AE_CONFIG_H_

#define AE_CONFIG_INITIALIZER ((struct ae_config) { .argz = NULL, .len = 0, })

struct ae_config {
	char *argz;
	size_t len;
};

int ae_config_read(struct ae_config *conf, const char *fmt, ...);
int ae_config_read_from_string(struct ae_config *conf, const char *string);
const char *ae_config_get(const struct ae_config *conf, const char *key);
const char *ae_config_get_default(const struct ae_config *conf, const char *key,
		const char *def);
int ae_config_get_int(const struct ae_config *conf, const char *key, int def);
int ae_config_add(struct ae_config *conf, const char *key, const char *value);
int ae_config_add_int(struct ae_config *conf, const char *key, int value);
void ae_config_cleanup(struct ae_config *conf);
int ae_config_write(const struct ae_config *conf, const char *path, ...);

#endif /* AE_CONFIG_H_ */
