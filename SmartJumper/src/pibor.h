/*Selective privatization*/
/*Jan Ciesko, Sergi Mateo, Xavi Teruel, BSC, 2014*/

typedef unsigned long int uint64_t;
typedef uint64_t ptr_t;

#define INT_TYPE u64Int
#define  LOCK_BUSY 1
#define LOCK_FREE 0

typedef struct {
	int * owners;
	int current_idx;
} task_owner_assoc_t;

typedef struct {
	int owner;
	int owner_orig;
} owner_t;

typedef struct {
	INT_TYPE value; //16bytes
	void * tag;

} entry_t;

typedef struct {
	entry_t * entries;
	int fill;
} red_buffer_t;

typedef struct {
	red_buffer_t * buffers_thread;
} red_buffers_t;

typedef struct {
	void * target;
	red_buffers_t * buffers;
	volatile char _global_struc_lock;
	int num_regions, buffer_size, region_size, num_buf_entries;
	uint64_t alloced_size, low_bits_offset, base_correction;
	char * locks;

	int num_red_tasks;
	/*task_owner_assoc_t * task_owner_assoc;
	 owner_t *owners;*/

} reduction_t;

reduction_t red; //basic thread-team global reduction manager

inline static void lock_aquire_red_1(int lock_id) {
	volatile char * state = &red.locks[lock_id];
	if (!__sync_val_compare_and_swap(state, LOCK_FREE, LOCK_BUSY))
		return;
//	if ( ( * state == LOCK_FREE) && !__sync_lock_test_and_set( state, LOCK_BUSY ) ) return;
	nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin",
			"Spin");
	spin: while (*state == LOCK_BUSY) {
	}
	if (__sync_lock_test_and_set(state, LOCK_BUSY))
		goto spin;
	nanos_instrument_end_burst("reduction-lock", "spin");
	return;
}

inline static int lock_aquire_red_2(int lock_id) {
	return __sync_val_compare_and_swap(&red.locks[lock_id], LOCK_FREE,
			LOCK_BUSY);
}

inline static void lock_aquire_red_3(volatile char * lock) {
	//if ( ( * lock == LOCK_FREE) && !__sync_lock_test_and_set( lock, LOCK_BUSY ) ) return;
	if (!__sync_val_compare_and_swap(lock, LOCK_FREE, LOCK_BUSY))
		return;
	nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin",
			"Spin");
	spin: while (*lock == LOCK_BUSY) {
	}
	if (__sync_lock_test_and_set(lock, LOCK_BUSY))
		goto spin;
	nanos_instrument_end_burst("reduction-lock", "spin");
	return;
}

inline static void lock_release_red_1(int lock_id) {
	volatile char * state = &red.locks[lock_id];
	__sync_lock_release(state);
}

inline static void lock_release_red_2(int lock_id) {
	red.locks[lock_id] = (char) LOCK_FREE;
}

inline static void lock_release_red_3(volatile char * lock) {
	__sync_lock_release(lock);
}

uint64_t compute_log2(uint64_t in) {
	uint64_t out;
	for (in *= 0.5, out = 0; in >= 1.0; in *= 0.5, out++)
		;
	return out;
}

void red_free() {
	for (int i = 0; i < omp_get_num_threads(); i++) {
		if (red.buffers[i].buffers_thread != NULL) {
			for (int j = 0; j < red.num_regions; j++) {
				if (red.buffers[i].buffers_thread[j].entries != NULL) {
					red_buffer_t * buf =
							(red_buffer_t *) &red.buffers[i].buffers_thread[j];
					free(buf->entries);
				}
			}
			free(red.buffers[i].buffers_thread);
		}
	}
	free(red.buffers);
	free(red.locks);
}

red_buffers_t * red_request_storage(void * target,
		uint64_t target_size /*in bytes*/, int max_privstorage_size/*KB*/,
		int buff_size/*KB*/, int reg_size/*KB*/) {
	int i, j, tid, bit_offset, regions, adr_norm_log, num_reg;
	uint64_t adr_norm;

	lock_aquire_red_3(&red._global_struc_lock);
	if (red.buffers == NULL) {
		red.buffers = (red_buffers_t*) malloc(
				omp_get_num_threads() * sizeof(red_buffers_t));

		for (i = 0; i < omp_get_num_threads(); i++) {
			red.buffers[i].buffers_thread = NULL;
		}

		//prepare configuration
		adr_norm = (uint64_t) target_size;
		adr_norm_log = compute_log2(adr_norm);
		red.target = target;
		red.base_correction = ((uint64_t) target);

		//detemine buffer_size, num_regs
		if (max_privstorage_size == 0 && reg_size != 0 && buff_size != 0) {
			num_reg = target_size / (reg_size * 1024);
		} else if (max_privstorage_size != 0 && reg_size == 0
				&& buff_size != 0) {
			//UNTESTED
			int per_thread_storage = max_privstorage_size
					/ omp_get_num_threads();
			num_reg = per_thread_storage / buff_size;
		} else if (max_privstorage_size != 0 && reg_size != 0
				&& buff_size == 0) {
			//UNTESTED
			int per_thread_storage = max_privstorage_size
					/ omp_get_num_threads();
			num_reg = target_size / (reg_size * 1024);
			buff_size = per_thread_storage / num_reg;

		} else {
			printf(
					"[PIBOR] Possibly incorrect parameters detected. Setting default\n");
			num_reg = 1024;
			buff_size = 1024;

		}
		red.region_size = reg_size * 1024;
		red.buffer_size = buff_size * 1024;
		red.num_buf_entries = red.buffer_size / sizeof(entry_t);

		//compute numbers of regions
		for (i = adr_norm_log - 1; i >= 0; --i) {
			regions =
					((uint64_t) adr_norm >> i) /*+ 1; !!! +1 is needed for generic benchmarks where the size of data is not always pow2*/;
			if (regions >= num_reg) {
				bit_offset = i;
				break;
			}
		}

		red.num_regions = regions;
		red.low_bits_offset = bit_offset;

		//prepare locks for regions
		red.locks = (char*) malloc(red.num_regions * sizeof(char));
		for (i = 0; i < red.num_regions; ++i) {
			red.locks[i] = LOCK_FREE;
		}

		red.num_red_tasks = omp_get_num_threads();
		lock_release_red_3(&red._global_struc_lock);

	} else {
		lock_release_red_3(&red._global_struc_lock);
	}

	tid = omp_get_thread_num();
	if (red.buffers[tid].buffers_thread == NULL) {
		nanos_instrument_begin_burst("reduction-init", "Reduction Init", "init",
				"Init reduction");
		//printf("Malloc for thread:%i\n", tid);
		red.buffers[tid].buffers_thread = (red_buffer_t *) malloc(
				red.num_regions * sizeof(red_buffer_t));
		for (j = 0; j < red.num_regions; j++) {
			red.buffers[tid].buffers_thread[j].fill = 0;
			red.buffers[tid].buffers_thread[j].entries = NULL; //(entry_t*) malloc (buffer_size * sizeof(entry_t));
		}
		nanos_instrument_end_burst("reduction-init", "init");
	}

	return &red.buffers[tid];
}

void red_reduce(INT_TYPE * target) {
	int block, block_mod, start;
	start = 0;

	if (red.num_red_tasks > red.num_regions)
		red.num_red_tasks = red.num_regions;

	TILESIZE(red.num_regions, red.num_red_tasks, block, block_mod);

	for (int m = 0; m < red.num_red_tasks; m++) {
		if (m == red.num_red_tasks - 1)
			block += block_mod;
#pragma omp task concurrent(target)
		{
			for (int i = start; i < start + block; i++) //region
					{
				for (int j = 0; j < omp_get_num_threads(); j++) //thread
						{
					target;
					if (red.buffers[j].buffers_thread != NULL) {
						if (red.buffers[j].buffers_thread[i].entries != NULL) {
							red_buffer_t * buf =
									(red_buffer_t *) &red.buffers[j].buffers_thread[i]; //find owning memory and reduce
							for (int k = 0; k < buf->fill; k++) {
								*((INT_TYPE*) buf->entries[k].tag) ^=
										buf->entries[k].value;
							}
							buf->fill = 0;
						}
					}
				}
			}
		}
		start += block;
	}
#pragma omp taskwait
}

void inline single_buf_reduce(red_buffer_t * buf) {
	for (int i = 0; i < buf->fill; i++) {
		*((INT_TYPE*) buf->entries[i].tag) ^= buf->entries[i].value;
	}
	buf->fill = 0;
}

#define SMARTREDUCE

inline void red_get_storage(void * adr, red_buffers_t * bufs,
		void ** adr_priv) {
	int i = ((uint64_t) adr - red.base_correction) >> red.low_bits_offset;

	red_buffer_t * buf = &(bufs->buffers_thread[i]);

	if (buf->entries == NULL) {
		//buf->entries = (entry_t*) malloc (red.buffer_size * sizeof(entry_t));
		posix_memalign((void**) &buf->entries, 4096, red.buffer_size);
#pragma omp atomic
		red.alloced_size += (red.buffer_size);
	}

	if (buf->fill >= red.num_buf_entries) {
		//nanos_instrument_begin_burst ( "reduction-global", "Reduction global", "toGlobal", "Block reduction" );
#ifdef SMARTREDUCE
		int j = i;
		nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin",
				"Spin");
		while (buf->fill) {
			if (!lock_aquire_red_2(i)) {
				single_buf_reduce(buf);
				lock_release_red_2(i);
			} else {
				//try to reduce a buffer for another region
				j = ++j % red.num_regions;
				if (bufs->buffers_thread[j].fill
						> red.num_buf_entries - red.num_buf_entries / 8) {
					if (!lock_aquire_red_2(j)) {
						single_buf_reduce(&bufs->buffers_thread[j]);
						lock_release_red_2(j);

					}
				}
			}

		}
		nanos_instrument_end_burst("reduction-lock", "spin");
#else
#ifdef BUFFERSTEALING
		int j = i;
		while(bufs->buffers_thread[i].fill)
		{
			if(!lock_aquire_red_2(i))
			{
				single_buf_reduce(buf);

				for(int k = 0; k < omp_get_num_threads(); k++)
				{
					if(red.buffers[k].buffers_thread != NULL)
					{
						if(red.buffers[k].buffers_thread[i].fill == red.num_buf_entries /*> red.buffer_size - red.buffer_size / 8*/)
						{
							single_buf_reduce(&red.buffers[k].buffers_thread[i]);
						}
					}
				}
				lock_release_red_2(i);
			} else
			{
				j = ++j % red.num_regions;
				if (bufs->buffers_thread[j].fill > red.num_buf_entries - red.num_buf_entries / 8)
				{
					if (!lock_aquire_red_2(j))
					{
						single_buf_reduce(&bufs->buffers_thread[j]);
						lock_release_red_2(j);

					}
				}
			}
		}

#else
		lock_aquire_red_1(i);
		single_buf_reduce(buf);
		lock_release_red_1(i);
#endif
#endif

		//	nanos_instrument_end_burst ( "reduction-global", "toGlobal" );
	}

	*adr_priv = &buf->entries[buf->fill];
	buf->entries[buf->fill].tag = adr;
	buf->fill++;
}

