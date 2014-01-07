#ifndef MBUG_2151_TARGETS_H
#define MBUG_2151_TARGETS_H 1
typedef char** tokenize_state_t;

typedef void (target_parse_func_type)(tokenize_state_t *);

enum {
	MBUG_2151_DMV7008_ON  = 0,
	MBUG_2151_DMV7008_OFF = 1,
	MBUG_2151_DMV7008_INC = 2,
	MBUG_2151_DMV7008_DEC = 3
};

void parse_target_ab440s(tokenize_state_t *state);
void parse_target_he302eu(tokenize_state_t *state);
void parse_target_dmv7008(tokenize_state_t *state);
void parse_target_ikt201(tokenize_state_t *state);

struct {
	char *name;
	target_parse_func_type *parse_function; 
} mbug_2151_targets[] = {
	{ "AB440S",  parse_target_ab440s },
	{ "HE302EU", parse_target_he302eu },
	{ "DMV7008", parse_target_dmv7008 },
	{ "GT7008",  parse_target_dmv7008 },
	{ "FIF4280", parse_target_dmv7008 },
	{ "IKT201",  parse_target_ikt201  },
};

#define NUM_MBUG_2151_TARGETS ((sizeof(mbug_2151_targets))/(sizeof(mbug_2151_targets[0])))

#endif
