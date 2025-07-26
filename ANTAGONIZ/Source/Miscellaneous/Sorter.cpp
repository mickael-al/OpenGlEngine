#include "Sorter.hpp"
// ---- ---- //

#include <glcore.hpp>

// ---- ---- //

constexpr uint64_t RADIX = 256;
constexpr uint64_t WORKGROUP_SIZE = 512;
constexpr uint64_t PARTITION_DIVISION = 8;
constexpr uint64_t PARTITION_SIZE = PARTITION_DIVISION * WORKGROUP_SIZE;

constexpr float GROWTH_RATIO = 1.0f;

constexpr const char* SHARED_HEADER = R"SHADER(
#version 460 core

#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable

#define SINGLE 0
#define MULTI 1
#define INDEXED 2

#define BITS_32 0
#define BITS_64 1

// ---- ---- //

const uint RADIX = 256;
const uint WORKGROUP_SIZE = 512;
const uint PARTITION_DIVISION = 8;
const uint PARTITION_SIZE = PARTITION_DIVISION * WORKGROUP_SIZE;

// ---- ---- //

layout(local_size_x = WORKGROUP_SIZE) in;

const uint LOCAL_ID = gl_LocalInvocationID.x
		+ gl_LocalInvocationID.y * gl_WorkGroupSize.x 
		+ gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y;

const uint WORKGROUP_ID = gl_WorkGroupID.x
	    + gl_WorkGroupID.y * gl_NumWorkGroups.x
		+ gl_WorkGroupID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y;

const uint THREAD_ID = LOCAL_ID + WORKGROUP_ID * WORKGROUP_SIZE;

// ---- ---- //

layout(binding = 0, std430) buffer Histograms {
	uint _histograms[];
};

layout(binding = 1, std430) buffer PartitionHistograms {
	uint _partition_histograms[];
};

layout(binding = 2, std430) buffer InputKeys {
	uint _input_keys[];
};

layout(binding = 3, std430) buffer OutputKeys {
	uint _output_keys[];
};

layout(binding = 4, std430) buffer InputIndices {
	uint _input_indices[];
};

layout(binding = 5, std430) buffer OutputIndices {
	uint _output_indices[];
};

// ---- ---- //

uint read_key(uint i) {
	return _input_keys[i];
}

void write_key(uint i, uint value) {
	_output_keys[i] = value;
}

// ---- ---- //

layout(location = 0) uniform uint _count;
layout(location = 1) uniform uint _pass;

// ---- ---- //
)SHADER";

constexpr const char* DOWNSWEEP_SHADER = R"SHADER(
shared uint local_histogram[PARTITION_SIZE];
shared uint local_histogram_sum[RADIX];

uvec4 get_excl_subgroup_mask(uint id) {
	uint shift = (1 << bitfieldExtract(id, 0, 5)) - 1;
	int x = int(id) >> 5;
	return uvec4(
		(shift & ((-1 - x) >> 31)) | ((0 - x) >> 31),
		(shift & ((0 - x) >> 31)) | ((1 - x) >> 31),
		(shift & ((1 - x) >> 31)) | ((2 - x) >> 31),
		(shift & ((2 - x) >> 31)) | ((3 - x) >> 31)
	);
}

uint get_bit_count(uvec4 value) {
	uvec4 result = bitCount(value);
	return result[0] + result[1] + result[2] + result[3];
}

void main() {
	int pass = int(_pass);

	uint subthread_id = gl_SubgroupInvocationID;  // 0..31 or 0..63
	uint subgroup_id = gl_SubgroupID;  // 0..15 or 0..7
	uint workthread_id = subgroup_id * gl_SubgroupSize + subthread_id;
	uvec4 subgroup_mask = get_excl_subgroup_mask(subthread_id);

	uint part_id = gl_WorkGroupID.x;
	uint part_offset = part_id * PARTITION_SIZE;

	if (part_offset >= _count) return;

	if (workthread_id < RADIX)
		for (int i = 0; i < gl_NumSubgroups; i++) 
			local_histogram[gl_NumSubgroups * workthread_id + i] = 0;
	barrier();

	// Load from global memory, local histogram and offset.
	uint local_keys[PARTITION_DIVISION];
#if (MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_64
	uint local_keys_other[PARTITION_DIVISION];
#endif

#if MODE == MULTI || MODE == INDEXED
	uint local_indices[PARTITION_DIVISION];
#endif
	uint local_radix[PARTITION_DIVISION];
	uint local_offsets[PARTITION_DIVISION];
	uint subgroup_histogram[PARTITION_DIVISION];

	for (int i = 0; i < PARTITION_DIVISION; i++) {
		uint offset = part_offset + (gl_SubgroupSize * PARTITION_DIVISION) * subgroup_id + i * gl_SubgroupSize + subthread_id;

#if MODE == INDEXED
		uint index = offset < _count ? _input_indices[offset] : 0;
		local_indices[i] = index;
#elif MODE == MULTI
		local_indices[i] = offset < _count ? _input_indices[offset] : 0;
#endif

#if MODE == INDEXED
		uint key_index = index;
#else
		uint key_index = offset;
#endif

#if FORMAT == BITS_64
		uint key = offset < _count ? read_key(key_index * 2 + pass / 4) : 0xFFFFFFFF;
#elif FORMAT == BITS_32
		uint key = offset < _count ? read_key(key_index) : 0xFFFFFFFF;
#endif

		local_keys[i] = key;

#if (MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_64
		local_keys_other[i] = offset < _count ? read_key(key_index * 2 + (pass / 4 + 1) % 2) : 0xFFFFFFFF;
#endif

		uint radix = bitfieldExtract(key, (pass % 4) * 8, 8);
		local_radix[i] = radix;

		// Mask per digit.
		uvec4 mask = subgroupBallot(true);

#pragma unroll
		for (int j = 0; j < 8; ++j) {
			uint digit = (radix >> j) & 1;
			uvec4 ballot = subgroupBallot(digit == 1);
			// digit - 1 is 0 or 0xffffffff. xor to flip.
			mask &= uvec4(digit - 1) ^ ballot;
		}

		// Subgroup level offset for radix.
		uint subgroup_offset = get_bit_count(subgroup_mask & mask);
		uint radix_count = get_bit_count(mask);

		// Elect a representative per radix, add to histogram.
		if (subgroup_offset == 0) {
			atomicAdd(local_histogram[radix * gl_NumSubgroups + subgroup_id], radix_count);
			subgroup_histogram[i] = radix_count;
		} else {
			subgroup_histogram[i] = 0;
		}

		local_offsets[i] = subgroup_offset;
	}
	barrier();

	// Local histogram reduce 4096 or 2048.
	for (uint i = workthread_id; i < gl_NumSubgroups * RADIX; i += WORKGROUP_SIZE) {
		uint v = local_histogram[i];
		uint sum = subgroupAdd(v);
		uint excl = subgroupExclusiveAdd(v);
		local_histogram[i] = excl;

		if (subthread_id == 0)
			local_histogram_sum[i / gl_SubgroupSize] = sum;
	}
	barrier();

	// Local histogram reduce 128 or 32.
	uint intermediate_offset = gl_NumSubgroups * RADIX / gl_SubgroupSize;
	if (workthread_id < intermediate_offset) {
		uint v = local_histogram_sum[workthread_id];
		uint sum = subgroupAdd(v);
		uint excl = subgroupExclusiveAdd(v);
		local_histogram_sum[workthread_id] = excl;
	
		if (subthread_id == 0)
			local_histogram_sum[intermediate_offset + workthread_id / gl_SubgroupSize] = sum;
	}
	barrier();

	// Local histogram reduce 4 or 1.
	uint intermediate_size = max(gl_NumSubgroups * RADIX / gl_SubgroupSize / gl_SubgroupSize, 1);
	if (workthread_id < intermediate_size) {
		uint v = local_histogram_sum[intermediate_offset + workthread_id];
		uint excl = subgroupExclusiveAdd(v);
		local_histogram_sum[intermediate_offset + workthread_id] = excl;
	}
	barrier();

	// Local histogram add 128.
	if (workthread_id < intermediate_offset)
		local_histogram_sum[workthread_id] += local_histogram_sum[intermediate_offset + workthread_id / gl_SubgroupSize];
	barrier();

	// Local histogram add 4096.
	for (uint i = workthread_id; i < gl_NumSubgroups * RADIX; i += WORKGROUP_SIZE)
		local_histogram[i] += local_histogram_sum[i / gl_SubgroupSize];
	barrier();

	// Post-scan stage.
	for (uint i = 0; i < PARTITION_DIVISION; i++) {
		uint radix = local_radix[i];
		local_offsets[i] += local_histogram[radix * gl_NumSubgroups + subgroup_id];

		barrier();
		if (subgroup_histogram[i] > 0)
			atomicAdd(local_histogram[radix * gl_NumSubgroups + subgroup_id], subgroup_histogram[i]);
		barrier();
	}

	// After atomicAdd, local_histogram contains inclusive sum.
	if (workthread_id < RADIX) {
		uint v = workthread_id == 0 ? 0 : local_histogram[workthread_id * gl_NumSubgroups - 1];
		
		local_histogram_sum[workthread_id] = 
			_histograms[pass * RADIX + workthread_id] 
			+ _partition_histograms[part_id * RADIX + workthread_id] - v;
	}
	barrier();

	// Rearrange keys. grouping keys together makes target_offset to be almost sequential, grants huge speed boost.
	// Now local_histogram is unused, so alias memory.
	for (uint i = 0; i < PARTITION_DIVISION; i++)
		local_histogram[local_offsets[i]] = local_keys[i];
	barrier();

	// Binning.
	for (uint i = workthread_id; i < PARTITION_SIZE; i += WORKGROUP_SIZE) {
		uint key = local_histogram[i];
		uint radix = bitfieldExtract(key, (pass % 4) * 8, 8);
		uint target_offset = local_histogram_sum[radix] + i;

#if (MODE == MULTI || MODE == INDEXED) || ((MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_64)
		local_keys[i / WORKGROUP_SIZE] = target_offset;
#endif

#if (MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_32
		if (target_offset < _count)
			write_key(target_offset, key);
#elif (MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_64
		if (target_offset < _count)
			write_key(target_offset * 2 + pass / 4, key);
#endif
	}

#if MODE == MULTI || MODE == INDEXED
	barrier();

	for (uint i = 0; i < PARTITION_DIVISION; i++)
		local_histogram[local_offsets[i]] = local_indices[i];
	barrier();

	for (uint i = workthread_id; i < PARTITION_SIZE; i += WORKGROUP_SIZE) {
		uint index = local_histogram[i];
		uint target_offset = local_keys[i / WORKGROUP_SIZE];

		if (target_offset < _count)
			_output_indices[target_offset] = index;
	}
#endif

#if (MODE == SINGLE || MODE == MULTI) && FORMAT == BITS_64
	barrier();

	for (uint i = 0; i < PARTITION_DIVISION; i++)
		local_histogram[local_offsets[i]] = local_keys_other[i];
	barrier();

	for (uint i = workthread_id; i < PARTITION_SIZE; i += WORKGROUP_SIZE) {
		uint key_other = local_histogram[i];
		uint target_offset = local_keys[i / WORKGROUP_SIZE];

		if (target_offset < _count)
			write_key(target_offset * 2 + (pass / 4 + 1) % 2, key_other);
	}
#endif
}
)SHADER";

constexpr const char* SPINE_SHADER = R"SHADER(
shared uint reduction;

// We only need array length equal to subgroup size = 32 or 64,
// but 128 shouldn't affect performance.
shared uint intermediate[128];

void main() {
	int pass = int(_pass);
	uint subthread_id = gl_SubgroupInvocationID;
	uint subgroup_id = gl_SubgroupID;
	uint workthread_id = subgroup_id * gl_SubgroupSize + subthread_id;
	uint radix = gl_WorkGroupID.x;

	uint part_count = (_count + PARTITION_SIZE - 1) / PARTITION_SIZE;

	if (workthread_id == 0)
		reduction = 0;
	barrier();

	for (uint i = 0; i * WORKGROUP_SIZE < part_count; i++) {
		uint part_id = i * WORKGROUP_SIZE + workthread_id;
		uint value = part_id < part_count ? _partition_histograms[part_id * RADIX + radix] : 0;
		uint excl = subgroupExclusiveAdd(value) + reduction;
		uint sum = subgroupAdd(value);

		if (subgroupElect())
			intermediate[subgroup_id] = sum;
		barrier();

		if (workthread_id < gl_NumSubgroups) {
			uint excl = subgroupExclusiveAdd(intermediate[workthread_id]);
			uint sum = subgroupAdd(intermediate[workthread_id]);
			intermediate[workthread_id] = excl;

			if (workthread_id == 0)
				reduction += sum;
		}
		barrier();

		if (part_id < part_count) {
			excl += intermediate[subgroup_id];
			_partition_histograms[part_id * RADIX + radix] = excl;
		}
		barrier();
	}

	if (gl_WorkGroupID.x == 0) {
		// One workgroup is responsible for global histogram prefix sum.
		if (workthread_id < RADIX) {
			uint value = _histograms[pass * RADIX + workthread_id];
			uint excl = subgroupExclusiveAdd(value);
			uint sum = subgroupAdd(value);

			if (subgroupElect())
				intermediate[subgroup_id] = sum;
			barrier();

			if (workthread_id < RADIX / gl_SubgroupSize) {
				uint excl = subgroupExclusiveAdd(intermediate[workthread_id]);
				intermediate[workthread_id] = excl;
			}
			barrier();

			excl += intermediate[subgroup_id];
			_histograms[pass * RADIX + workthread_id] = excl;
		}
	}
}
)SHADER";

constexpr const char* UPSWEEP_SHADER = R"SHADER(
shared uint local_histogram[RADIX];

void main() {
	int pass = int(_pass);
	uint subthread_id = gl_SubgroupInvocationID;
	uint subgroup_id = gl_SubgroupID;
	uint workthread_id = subgroup_id * gl_SubgroupSize + subthread_id;

	uint part_id = gl_WorkGroupID.x;
	uint part_offset = part_id * PARTITION_SIZE;

	 // Discard all workgroup invocations.
	if (part_offset >= _count) return;

	if (workthread_id < RADIX)
		local_histogram[workthread_id] = 0;
	barrier();

	// Local histogram.
	for (uint i = 0; i < PARTITION_DIVISION; i++) {
		uint offset = part_offset + i * WORKGROUP_SIZE + workthread_id;

#if MODE == INDEXED
		uint key_index = _input_indices[offset];
#else
		uint key_index = offset;
#endif

#if FORMAT == BITS_64
		key_index = key_index * 2 + pass / 4;
#endif

		uint key = offset < _count ? read_key(key_index) : 0xFFFFFFFF;

		uint radix = bitfieldExtract(key, (pass % 4) * 8, 8);
		atomicAdd(local_histogram[radix], 1);
	}
	barrier();

	if (workthread_id < RADIX) {
		// Set to partition histogram.
		_partition_histograms[part_id * RADIX + workthread_id] = local_histogram[workthread_id];

		// Add to global histogram.
	    atomicAdd(_histograms[pass * RADIX + workthread_id], local_histogram[workthread_id]);
	}
}
)SHADER";

constexpr const char* INIT_INDICES_SHADER = R"SHADER(
void main() {
	if (THREAD_ID >= _count)
		return;

	_input_indices[THREAD_ID] = THREAD_ID;
}
)SHADER";

constexpr const char* BITFLIP_KEYS_SHADER = R"SHADER(
void main() {
	if (THREAD_ID >= _count)
		return;

#if FORMAT == BITS_32
	uint id = THREAD_ID;
#elif FORMAT == BITS_64
	uint id = THREAD_ID * 2 + 1;
#endif

	write_key(id, read_key(id) ^ 0x80000000);
}
)SHADER";

constexpr const char* ENCODE_FLOAT_KEYS_SHADER = R"SHADER(
void main() {
	if (THREAD_ID >= _count)
		return;

#if FORMAT == BITS_32

	uint k = read_key(THREAD_ID);
	if ((k & 0x80000000) != 0)
		write_key(THREAD_ID, ~k);
	else
		write_key(THREAD_ID, k ^ 0x80000000);

#elif FORMAT == BITS_64

	uint k = read_key(THREAD_ID * 2 + 1);
	if ((k & 0x80000000) != 0) {
		write_key(THREAD_ID * 2 + 0, ~read_key(THREAD_ID * 2 + 0));
		write_key(THREAD_ID * 2 + 1, ~k);
	} else
		write_key(THREAD_ID * 2 + 1, k ^ 0x80000000);

#endif

}
)SHADER";

constexpr const char* DECODE_FLOAT_KEYS_SHADER = R"SHADER(
void main() {
	if (THREAD_ID >= _count)
		return;

#if FORMAT == BITS_32

	uint k = read_key(THREAD_ID);
	if ((k & 0x80000000) == 0)
		write_key(THREAD_ID, ~k);
	else
		write_key(THREAD_ID, k ^ 0x80000000);

#elif FORMAT == BITS_64

	uint k = read_key(THREAD_ID * 2 + 1);
	if ((k & 0x80000000) == 0) {
		write_key(THREAD_ID * 2 + 0, ~read_key(THREAD_ID * 2 + 0));
		write_key(THREAD_ID * 2 + 1, ~k);
	} else
		write_key(THREAD_ID * 2 + 1, k ^ 0x80000000);

#endif

}
)SHADER";

// ---- ---- //

uint64_t calculate_partition_count(uint64_t count) {
	return ((count + PARTITION_SIZE - 1) / PARTITION_SIZE);
}

uint32_t create_program(const char* source_code, const char* defines) {
	uint32_t gl_shader_id = glCreateShader(GL_COMPUTE_SHADER);

	const char* lines[3] = { SHARED_HEADER, defines, source_code };
	glShaderSource(gl_shader_id, 3, lines, nullptr);
	glCompileShader(gl_shader_id);

	GLint compiled = GL_FALSE;
	glGetShaderiv(gl_shader_id, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		glDeleteShader(gl_shader_id);
		return 0;
	}

	uint32_t gl_program_id = glCreateProgram();

	glAttachShader(gl_program_id, gl_shader_id);
	glLinkProgram(gl_program_id);

	glDeleteShader(gl_shader_id);

	GLint linked = GL_FALSE;
	glGetProgramiv(gl_program_id, GL_LINK_STATUS, &linked);
	if (!linked) {
		glDeleteProgram(gl_program_id);
		return 0;
	}

	return gl_program_id;
}

Sorter::Sorter() {
	if ((_downsweep_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE SINGLE\n#define FORMAT BITS_32")) == 0) return;
	if ((_downsweep_64_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE SINGLE\n#define FORMAT BITS_64")) == 0) return;

	if ((_downsweep_mlt_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE MULTI\n#define FORMAT BITS_32")) == 0) return;
	if ((_downsweep_mlt_64_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE MULTI\n#define FORMAT BITS_64")) == 0) return;

	if ((_downsweep_idx_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE INDEXED\n#define FORMAT BITS_32")) == 0) return;
	if ((_downsweep_idx_64_gl_program_id = create_program(DOWNSWEEP_SHADER, "#define MODE INDEXED\n#define FORMAT BITS_64")) == 0) return;

	if ((_spine_gl_program_id = create_program(SPINE_SHADER, "")) == 0) return;

	if ((_upsweep_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE SINGLE\n#define FORMAT BITS_32")) == 0) return;
	if ((_upsweep_64_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE SINGLE\n#define FORMAT BITS_64")) == 0) return;

	if ((_upsweep_mlt_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE MULTI\n#define FORMAT BITS_32")) == 0) return;
	if ((_upsweep_mlt_64_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE MULTI\n#define FORMAT BITS_64")) == 0) return;

	if ((_upsweep_idx_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE INDEXED\n#define FORMAT BITS_32")) == 0) return;
	if ((_upsweep_idx_64_gl_program_id = create_program(UPSWEEP_SHADER, "#define MODE INDEXED\n#define FORMAT BITS_64")) == 0) return;

	if ((_init_indices_gl_program_id = create_program(INIT_INDICES_SHADER, "")) == 0) return;

	if ((_bitflip_keys_gl_program_id = create_program(BITFLIP_KEYS_SHADER, "#define FORMAT BITS_32")) == 0) return;
	if ((_bitflip_keys_64_gl_program_id = create_program(BITFLIP_KEYS_SHADER, "#define FORMAT BITS_64")) == 0) return;

	if ((_encode_float_keys_gl_program_id = create_program(ENCODE_FLOAT_KEYS_SHADER, "#define FORMAT BITS_32")) == 0) return;
	if ((_encode_float_keys_64_gl_program_id = create_program(ENCODE_FLOAT_KEYS_SHADER, "#define FORMAT BITS_64")) == 0) return;

	if ((_decode_float_keys_gl_program_id = create_program(DECODE_FLOAT_KEYS_SHADER, "#define FORMAT BITS_32")) == 0) return;
	if ((_decode_float_keys_64_gl_program_id = create_program(DECODE_FLOAT_KEYS_SHADER, "#define FORMAT BITS_64")) == 0) return;

	_valid = true;
}

Sorter::~Sorter() {
	_valid = false;

	glDeleteBuffers(1, &_work_gl_buffer_id);
	_capacity = 0;

	glDeleteProgram(_downsweep_gl_program_id);
	glDeleteProgram(_downsweep_64_gl_program_id);

	glDeleteProgram(_downsweep_mlt_gl_program_id);
	glDeleteProgram(_downsweep_mlt_64_gl_program_id);

	glDeleteProgram(_downsweep_idx_gl_program_id);
	glDeleteProgram(_downsweep_idx_64_gl_program_id);

	glDeleteProgram(_spine_gl_program_id);

	glDeleteProgram(_upsweep_gl_program_id);
	glDeleteProgram(_upsweep_64_gl_program_id);

	glDeleteProgram(_upsweep_mlt_gl_program_id);
	glDeleteProgram(_upsweep_mlt_64_gl_program_id);

	glDeleteProgram(_upsweep_idx_gl_program_id);
	glDeleteProgram(_upsweep_idx_64_gl_program_id);

	glDeleteProgram(_bitflip_keys_gl_program_id);
	glDeleteProgram(_bitflip_keys_64_gl_program_id);

	glDeleteProgram(_encode_float_keys_gl_program_id);
	glDeleteProgram(_encode_float_keys_64_gl_program_id);

	glDeleteProgram(_decode_float_keys_gl_program_id);
	glDeleteProgram(_decode_float_keys_64_gl_program_id);

	glDeleteProgram(_init_indices_gl_program_id);
}

void Sorter::set_capacity(uint64_t capacity) {
	if (_capacity != 0) {
		glDeleteBuffers(1, &_work_gl_buffer_id); _work_gl_buffer_id = 0;
		_capacity = 0;
	}

	if (capacity != 0) {
		glCreateBuffers(1, &_work_gl_buffer_id);
		glNamedBufferStorage(_work_gl_buffer_id, capacity, nullptr, 0);

		_capacity = capacity;
	}
}

void Sorter::clear() {
	set_capacity(0);
}

void Sorter::ensure_capacity(uint64_t required_capacity) {
	if (_capacity >= required_capacity)
		return;

	set_capacity(required_capacity);
}


bool Sorter::init_indices(uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_init_indices_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, indices_gl_buffer_id);

	glUseProgram(_init_indices_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return false;
}

bool Sorter::sort_uints(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!sort_raws(key_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_uints_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!sort_raws_64(key_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_ints(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;
	if (!sort_raws(key_gl_buffer_id, count)) return false;
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_ints_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_raws_64(key_gl_buffer_id, count)) return false;
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_floats(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys(key_gl_buffer_id, count)) return false;
	if (!sort_raws(key_gl_buffer_id, count)) return false;
	if (!decode_floats_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_floats_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_raws_64(key_gl_buffer_id, count)) return false;
	if (!decode_floats_keys_64(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_indexed_uints(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!sort_indexed_raws(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_indexed_uints_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!sort_indexed_raws_64(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_indexed_ints(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;
	if (!sort_indexed_raws(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_indexed_ints_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_indexed_raws_64(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_indexed_floats(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys(key_gl_buffer_id, count)) return false;
	if (!sort_indexed_raws(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	if (!decode_floats_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_indexed_floats_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_indexed_raws_64(key_gl_buffer_id, indices_gl_buffer_id, count)) return false;
	if (!decode_floats_keys_64(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_multi_uints(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!sort_multi_raws(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_multi_uints_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!sort_multi_raws_64(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	return true;
}

bool Sorter::sort_multi_ints(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;
	if (!sort_multi_raws(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	if (!bitflip_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_multi_ints_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_multi_raws_64(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	if (!bitflip_keys_64(key_gl_buffer_id, count)) return false;

	return true;

}

bool Sorter::sort_multi_floats(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys(key_gl_buffer_id, count)) return false;
	if (!sort_multi_raws(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	if (!decode_floats_keys(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_multi_floats_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!encode_floats_keys_64(key_gl_buffer_id, count)) return false;
	if (!sort_multi_raws_64(key_gl_buffer_id, secondary_gl_buffer_id, count)) return false;
	if (!decode_floats_keys_64(key_gl_buffer_id, count)) return false;

	return true;
}

bool Sorter::sort_raws(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (4ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + count * 4ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		4ull * RADIX * 4ull,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 4ull * RADIX * 4ull, partition_count * RADIX * 4ull);

	for (uint32_t pass = 0; pass < 4; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 0) % 2, key_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 1) % 2, _work_gl_buffer_id, hist_size, count * 4ull);

		glProgramUniform1ui(_downsweep_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_gl_program_id, 1, pass);

		glUseProgram(_upsweep_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::sort_raws_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (8ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + count * 8ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		8ull * RADIX * 4ull,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_64_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_64_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 8ull * RADIX * 4ull, partition_count * RADIX * 4ull);

	for (uint32_t pass = 0; pass < 8; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 0) % 2, key_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 1) % 2, _work_gl_buffer_id, hist_size, count * 8ull);

		glProgramUniform1ui(_downsweep_64_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_64_gl_program_id, 1, pass);

		glUseProgram(_upsweep_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::sort_multi_raws(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t aligned_count = (count + 63ull) / 64ull * 64ull;
	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (4ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + aligned_count * 8ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		4 * RADIX * 4,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_mlt_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_mlt_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 4ull * RADIX * 4ull, partition_count * RADIX * 4ull);

	for (uint32_t pass = 0; pass < 4; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 0) % 2, key_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 1) % 2, _work_gl_buffer_id, hist_size + 0, aligned_count * 4ull);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 0) % 2, secondary_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 1) % 2, _work_gl_buffer_id, hist_size + aligned_count * 4ull, aligned_count * 4ull);

		glProgramUniform1ui(_downsweep_mlt_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_mlt_gl_program_id, 1, pass);

		glUseProgram(_upsweep_mlt_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_mlt_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::sort_multi_raws_64(uint32_t key_gl_buffer_id, uint32_t secondary_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t aligned_count = (count + 63ull) / 64ull * 64ull;
	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (8ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + aligned_count * 12ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		8 * RADIX * 4,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_mlt_64_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_mlt_64_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 8ull * RADIX * 4ull, partition_count * RADIX * 4ull);

	for (uint32_t pass = 0; pass < 8; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 0) % 2, key_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2 + (pass + 1) % 2, _work_gl_buffer_id, hist_size + 0, aligned_count * 8ull);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 0) % 2, secondary_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 1) % 2, _work_gl_buffer_id, hist_size + aligned_count * 8ull, aligned_count * 4ull);

		glProgramUniform1ui(_downsweep_mlt_64_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_mlt_64_gl_program_id, 1, pass);

		glUseProgram(_upsweep_mlt_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_mlt_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::sort_indexed_raws(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (4ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + count * 4ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		4 * RADIX * 4,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_idx_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_idx_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 4ull * RADIX * 4ull, partition_count * RADIX * 4ull);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);

	for (uint32_t pass = 0; pass < 4; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 0) % 2, indices_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 1) % 2, _work_gl_buffer_id, hist_size, count * 4ull);

		glProgramUniform1ui(_downsweep_idx_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_idx_gl_program_id, 1, pass);

		glUseProgram(_upsweep_idx_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_idx_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::sort_indexed_raws_64(uint32_t key_gl_buffer_id, uint32_t indices_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	uint64_t partition_count = calculate_partition_count(count);
	uint64_t hist_size = (8ull * RADIX + partition_count * RADIX) * 4ull;
	ensure_capacity(hist_size + count * 4ull);

	uint32_t zero = 0;
	glClearNamedBufferSubData(_work_gl_buffer_id, GL_R32UI, 0,
		8 * RADIX * 4,
		GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

	glProgramUniform1ui(_downsweep_idx_64_gl_program_id, 0, count);
	glProgramUniform1ui(_spine_gl_program_id, 0, count);
	glProgramUniform1ui(_upsweep_idx_64_gl_program_id, 0, count);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _work_gl_buffer_id);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, _work_gl_buffer_id, 8ull * RADIX * 4ull, partition_count * RADIX * 4ull);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);

	for (uint32_t pass = 0; pass < 8; pass++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 0) % 2, indices_gl_buffer_id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4 + (pass + 1) % 2, _work_gl_buffer_id, hist_size, count * 4ull);

		glProgramUniform1ui(_downsweep_idx_64_gl_program_id, 1, pass);
		glProgramUniform1ui(_spine_gl_program_id, 1, pass);
		glProgramUniform1ui(_upsweep_idx_64_gl_program_id, 1, pass);

		glUseProgram(_upsweep_idx_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_spine_gl_program_id);
		glDispatchCompute(RADIX, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(_downsweep_idx_64_gl_program_id);
		glDispatchCompute(partition_count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	return true;
}

bool Sorter::bitflip_keys(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_bitflip_keys_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_bitflip_keys_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}

bool Sorter::bitflip_keys_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_bitflip_keys_64_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_bitflip_keys_64_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}

bool Sorter::encode_floats_keys(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_encode_float_keys_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_encode_float_keys_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}

bool Sorter::encode_floats_keys_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_encode_float_keys_64_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_encode_float_keys_64_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}

bool Sorter::decode_floats_keys(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_decode_float_keys_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_decode_float_keys_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}

bool Sorter::decode_floats_keys_64(uint32_t key_gl_buffer_id, uint32_t count) {
	if (!_valid)
		return false;

	glProgramUniform1ui(_decode_float_keys_64_gl_program_id, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_gl_buffer_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, key_gl_buffer_id);

	glUseProgram(_decode_float_keys_64_gl_program_id);
	glDispatchCompute((count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return true;
}
