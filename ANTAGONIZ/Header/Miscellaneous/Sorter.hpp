#pragma once
// ---- ---- //

#include <cstdint>

// ---- ---- //

class Sorter {
public:
	Sorter();
	~Sorter();

	Sorter(const Sorter&) = delete;
	Sorter& operator=(const Sorter&) = delete;

	Sorter(Sorter&&) = delete;
	Sorter& operator=(Sorter&&) = delete;

public:
	bool valid() { return _valid; }

	uint64_t capacity() { return _capacity; }
	void set_capacity(uint64_t capacity);
	void clear();
	void ensure_capacity(uint64_t required_capacity);

	// Initializes the given buffer with incrementing values, from 0 to count.
	bool init_indices(uint32_t indices_gl_buffer_id, uint32_t count);

	// Sorts the given buffer in place.
	// Additional memory usage : (count * 4) bytes.
	//   For 2^20 (1 048 576 = 4 MiB) elements, this corresponds to 4 194 304 additional bytes (4 MiB).
	bool sort_uints(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_uints_64(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_ints(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_ints_64(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_floats(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_floats_64(uint32_t key_gl_buffer_id, uint32_t count);

	// Sorts the given buffer and secondary buffer in place.
	//   The secondary buffer is updated similarly to the key buffer.
	// Additional memory usage : (count * 8) bytes.
	//   For 2^20 (1 048 576 = 4 MiB) elements and 2^20 (1 048 576 = 4 MiB) secondary values, this corresponds to 8 388 608 additional bytes (8 MiB).
	bool sort_multi_uints(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_uints_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_ints(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_ints_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_floats(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_floats_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);

	// Sorts the given buffer by sorting the indices in place. The key buffer will NOT be changed.
	// Additional memory usage : (count * 4) bytes.
	//   For 2^20 (1 048 576 = 4 MiB) elements and 2^20 (1 048 576 = 4 MiB) indices, this corresponds to 4 194 304 additional bytes (4 MiB).
	bool sort_indexed_uints(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_uints_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_ints(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_ints_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_floats(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_floats_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);

private:
	bool sort_raws(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_raws_64(uint32_t key_gl_buffer_id, uint32_t count);
	bool sort_multi_raws(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_multi_raws_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count);
	bool sort_indexed_raws(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);
	bool sort_indexed_raws_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count);

	bool bitflip_keys(uint32_t key_gl_buffer_id, uint32_t count);
	bool bitflip_keys_64(uint32_t key_gl_buffer_id, uint32_t count);
	bool encode_floats_keys(uint32_t key_gl_buffer_id, uint32_t count);
	bool encode_floats_keys_64(uint32_t key_gl_buffer_id, uint32_t count);
	bool decode_floats_keys(uint32_t key_gl_buffer_id, uint32_t count);
	bool decode_floats_keys_64(uint32_t key_gl_buffer_id, uint32_t count);

private:
	uint32_t _work_gl_buffer_id = 0;
	uint64_t _capacity = 0;

	uint32_t _downsweep_gl_program_id = 0;
	uint32_t _downsweep_64_gl_program_id = 0;
	uint32_t _downsweep_mlt_gl_program_id = 0;
	uint32_t _downsweep_mlt_64_gl_program_id = 0;
	uint32_t _downsweep_idx_gl_program_id = 0;
	uint32_t _downsweep_idx_64_gl_program_id = 0;

	uint32_t _spine_gl_program_id = 0;

	uint32_t _upsweep_gl_program_id = 0;
	uint32_t _upsweep_64_gl_program_id = 0;
	uint32_t _upsweep_mlt_gl_program_id = 0;
	uint32_t _upsweep_mlt_64_gl_program_id = 0;
	uint32_t _upsweep_idx_gl_program_id = 0;
	uint32_t _upsweep_idx_64_gl_program_id = 0;

	uint32_t _init_indices_gl_program_id = 0;

	uint32_t _bitflip_keys_gl_program_id = 0;
	uint32_t _bitflip_keys_64_gl_program_id = 0;
	uint32_t _encode_float_keys_gl_program_id = 0;
	uint32_t _encode_float_keys_64_gl_program_id = 0;
	uint32_t _decode_float_keys_gl_program_id = 0;
	uint32_t _decode_float_keys_64_gl_program_id = 0;

	bool _valid = false;
};