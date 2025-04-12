// Dla s = 1, 96, funkcja ustawia nowy stan jako negację wejścia.
void transition_test2_f(uint64_t *next_state, uint64_t const *input,
	uint64_t const *state, size_t n, size_t s) {

    (void) n;
    (void) state;

	if (s == 1)
		next_state[0] = !input[0];
	else if (s == 96) {
		next_state[0] = ~input[0];
		next_state[1] = ~input[1] & 0x00000000ffffffff;
	}
}

void transition_add_f(uint64_t *next_state, uint64_t const *input,
	uint64_t const *state, size_t n, size_t s) {

	next_state[1] = state[1] + input[1];
}


static int test2(void) {
	
	moore_t *a1 = ma_create_simple(100, 100, &transition_add_f);
	ma_set_state(a1, (uint64_t [2]) {0, 1});
	uint64_t const *a1_out = ma_get_output(a1);
	ma_set_input(a1, (uint64_t [2]) {0, 0});

	// Argumenty poza zakresem, więc połączenie nie powinno się wykonać.
	ma_connect(a1, 64, a1, 64, 100);
	ma_step(&a1, 1);
	ASSERT(a1_out[1] == (uint64_t) 1);

	// Teraz już powinno się wykonać.
	ma_connect(a1, 64, a1, 64, 36);

	for (size_t i = 0; i < 5; i++)
		ma_step(&a1, 1);
	ASSERT(a1_out[1] == 32);

	const size_t a2_size = 8;
	moore_t *a2[a2_size];
	uint64_t const *a2_output[a2_size];

	for (size_t i = 0; i < a2_size; i++) {
		a2[i] = ma_create_simple(1, 1, &transition_test2_f);
		a2_output[i] = ma_get_output(a2[i]);
		ASSERT(a2_output[i][0] == 0);
		
		ma_set_input(a2[i], (uint64_t [1]) {1});
		ma_connect(a2[i], 0, a1, 0, 1);
		ma_step(&a2[i], 1);
		ASSERT(a2_output[i][0] == 1);
	}

	moore_t *a3 = ma_create_simple(96, 96, &transition_test2_f);
	ma_set_state(a3, (uint64_t [2]) {~(uint64_t) 0, 0x00000000ffffffff});
	uint64_t const *a3_output = ma_get_output(a3);
	ASSERT(a3_output[1] == 0x00000000ffffffff && a3_output[0] == ~(uint64_t) 0);
	
	for (size_t i = 0; i < 96; i++)
		ma_connect(a3, i, a2[i % 8], 0, 1);
	ma_step(&a3, 1);
	ASSERT(a3_output[1] == 0 && a3_output[0] == 0);

	ma_set_state(a1, (uint64_t [2]) {1, 0});
	ma_step(a2, a2_size);
	for (size_t i = 0; i < a2_size; i++) {
		ASSERT(a2_output[i][0] == 0);
	}

	for (size_t i = 0; i < a2_size; i++)
		ma_delete(a2[i]);
	ma_delete(a1);
	ma_delete(a3);
	
	return PASS;
}	
