#ifndef SERVICEBASE_H
#define SERVICEBASE_H

#ifdef __cplusplus
extern "C" {
#endif

// Declare callback function signatures.
typedef int (*hbl_service_start_funct)(void);
typedef int(*hbl_service_stop_funct)(void);

// Declare special types.
typedef struct hbl_service_config_t
{
	hbl_service_start_funct start;
	hbl_service_stop_funct stop;
} hbl_service_config_t;

// Helpers to create and delete configuration.
hbl_service_config_t* hbl_service_create_config();
void hbl_service_free_config(hbl_service_config_t* c);

// Entry point for the service.
int hbl_service_run(hbl_service_config_t* conf);

#ifdef __cplusplus
}
#endif

#endif
