/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/
#define ESIF_TRACE_ID ESIF_TRACEMODULE_WEBSERVER

#include "esif_ws_algo.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */

static const char g_esif_ws_algo_asciivalue_of_six_bits[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static UInt32 esif_ws_algo_circular_left_shift (UInt32, UInt8);

static UInt32 esif_ws_algo_byteswap_unsigned_long (UInt32);

static UInt32 esif_ws_algo_func0to19 (UInt32, UInt32, UInt32);

static UInt32 esif_ws_algo_func40to59 (UInt32, UInt32, UInt32);

static UInt32 esif_ws_algo_func20to39_or_60to79 (UInt32, UInt32, UInt32);


static UInt32 esif_ws_algo_circular_left_shift (
	UInt32 shitf_num,
	UInt8 value
	)
{
	return (shitf_num << value) | (shitf_num >> (32 - value));
}


static UInt32 esif_ws_algo_byteswap_unsigned_long (UInt32 byte_to_swap)
{
	return ((byte_to_swap) << 24) | ((byte_to_swap) >> 24) | (((byte_to_swap) & 0x0000ff00) << 8) | (((byte_to_swap) & 0x00ff0000) >> 8);
}


static UInt32 esif_ws_algo_func0to19 (
	UInt32 second_hash_val,
	UInt32 third_hash_val,
	UInt32 fourth_hash_val
	)
{
	return (second_hash_val & third_hash_val) ^ ((~second_hash_val) & fourth_hash_val);
}


static UInt32 esif_ws_algo_func40to59 (
	UInt32 second_hash_val,
	UInt32 third_hash_val,
	UInt32 fourth_hash_val
	)
{
	return (second_hash_val & third_hash_val) ^ (second_hash_val & fourth_hash_val) ^ (third_hash_val & fourth_hash_val);
}


static UInt32 esif_ws_algo_func20to39_or_60to79 (
	UInt32 second_hash_val,
	UInt32 third_hash_val,
	UInt32 fourth_hash_val
	)
{
	return (second_hash_val ^ third_hash_val) ^ fourth_hash_val;
}


/*
 *******************************************************************************
 ** PUBLIC INTERFACE
 *******************************************************************************
 */

void esif_ws_algo_sha_context_init (shaCtx *state)
{
	state->hash_values_array[0] = 0x67452301;

	state->hash_values_array[1] = 0xefcdab89;

	state->hash_values_array[2] = 0x98badcfe;

	state->hash_values_array[3] = 0x10325476;

	state->hash_values_array[4] = 0xc3d2e1f0;

	state->length = 0;
}


#define MASK 0x0000000f

typedef UInt32 (*esif_ws_algo_func_ptr)(UInt32 x, UInt32 y, UInt32 z);

void esif_ws_algo_add_block_to_sha (
	shaCtx *state,
	const void *block
	)
{
	UInt32 chunk_of_initial_values[5]={0};
	UInt32 array_of_16_words[16]={0};
	UInt32 temp_value1=0;
	UInt32 temp_value2=0;
	UInt8 index=0;
	UInt8 thiry_two_bit_word_index=0;
	UInt8 kvalues_index=0;
	UInt8 twenty_value_processed_flag=0;

	esif_ws_algo_func_ptr algo_func_ptr_array[] = {
		esif_ws_algo_func0to19,
		esif_ws_algo_func20to39_or_60to79,
		esif_ws_algo_func40to59,
		esif_ws_algo_func20to39_or_60to79
	};


	UInt32 array_of_kvalues[4] = {FUNC0_TO_19_KVALUE, FUNC20_TO_39_KVALUE, FUNC40_TO_59_KVALUE, FUNC60_TO_79_KVALUE};


	for (index = 0; index < 16; ++index)

		array_of_16_words[index] = esif_ws_algo_byteswap_unsigned_long(((UInt32*)block)[index]);


	esif_ccb_memcpy(chunk_of_initial_values, state->hash_values_array, 5 * sizeof(UInt32));

	for (kvalues_index = 0, twenty_value_processed_flag = 0, index = 0; index <= 79; ++index) {
		thiry_two_bit_word_index = index & MASK;

		if (index >= 16) {
			array_of_16_words[thiry_two_bit_word_index] = esif_ws_algo_circular_left_shift(array_of_16_words[(thiry_two_bit_word_index + 13) & MASK] ^
																						   array_of_16_words[(thiry_two_bit_word_index + 8) & MASK] ^
																						   array_of_16_words[(thiry_two_bit_word_index + 2) & MASK] ^
																						   array_of_16_words[thiry_two_bit_word_index],
																						   1);
		}


		temp_value1 =
			esif_ws_algo_circular_left_shift(chunk_of_initial_values[0],
											 5) +
			(temp_value2 = algo_func_ptr_array[kvalues_index](chunk_of_initial_values[1], chunk_of_initial_values[2], chunk_of_initial_values[3])) +
			chunk_of_initial_values[4] +
			array_of_kvalues[kvalues_index] +
			array_of_16_words[thiry_two_bit_word_index];

		memmove(&(chunk_of_initial_values[1]), &(chunk_of_initial_values[0]), 4 * sizeof(UInt32));

		chunk_of_initial_values[0] = temp_value1;

		chunk_of_initial_values[2] = esif_ws_algo_circular_left_shift(chunk_of_initial_values[2], 30);

		twenty_value_processed_flag++;

		if (twenty_value_processed_flag == 20) {
			twenty_value_processed_flag = 0;
			kvalues_index = (kvalues_index + 1) % 4;
		}
	}


	for (index = 0; index < 5; ++index)
		state->hash_values_array[index] += chunk_of_initial_values[index];

	state->length += 512;
}


void esif_ws_algo_add_last_block_to_sha (
	shaCtx *state,
	const void *block,
	UInt16 length
	)
{
	UInt8 i=0;
	UInt8 lb[SHA1_BLOCK_BYTES]={0};

	while (length >= NUM_BITS_BLOCK) {
		esif_ws_algo_add_block_to_sha(state, block);

		length -= NUM_BITS_BLOCK;

		block   = (UInt8*)block + SHA1_BLOCK_BYTES;
	}

	state->length += length;

	esif_ccb_memset(lb, 0, SHA1_BLOCK_BYTES);

	esif_ccb_memcpy(lb, block, (length + 7) >> 3);


	lb[length >> 3] |= 0x80 >> (length & 0x07);

	if (length > 512 - 64 - 1) {
		esif_ws_algo_add_block_to_sha(state, lb);

		state->length -= 512;

		esif_ccb_memset(lb, 0, SHA1_BLOCK_BYTES);
	}


	for (i = 0; i < 8; ++i)

		lb[56 + i] = ((UInt8*)&(state->length))[7 - i];

	esif_ws_algo_add_block_to_sha(state, lb);
}


void esif_ws_algo_hash_sha_context (
	void *dest,
	shaCtx *state
	)
{
	UInt8 i=0;

	for (i = 0; i < 5; ++i)
		((UInt32*)dest)[i] = esif_ws_algo_byteswap_unsigned_long(state->hash_values_array[i]);

}


void esif_ws_algo_hash_sha_algo (
	void *destination,
	const void *message,
	UInt32 length
	)
{
	shaCtx sha_ctx ={0};

	esif_ws_algo_sha_context_init(&sha_ctx);

	while (length & (~0x0001ff)) {
		esif_ws_algo_add_block_to_sha(&sha_ctx, message);

		message = (UInt8*)message + NUM_BITS_BLOCK / 8;

		length -= NUM_BITS_BLOCK;
	}

	esif_ws_algo_add_last_block_to_sha(&sha_ctx, message, (UInt16/* JDH*/)length);

	esif_ws_algo_hash_sha_context(destination, &sha_ctx);
}


void esif_ws_algo_encode_base64_value (
	char *destination,
	const void *source,
	UInt16 length
	)
{
	UInt16 i=0, j=0;

	UInt8 block_of_bytes[4]={0};

	for (i = 0; i < length / 3; ++i) {
		block_of_bytes[0] = (((UInt8*)source)[i * 3 + 0]) >> 2;

		block_of_bytes[1] = (((((UInt8*)source)[i * 3 + 0]) << 4) | ((((UInt8*)source)[i * 3 + 1]) >> 4)) & 0x3F;

		block_of_bytes[2] = (((((UInt8*)source)[i * 3 + 1]) << 2) | ((((UInt8*)source)[i * 3 + 2]) >> 6)) & 0x3F;

		block_of_bytes[3] = (((UInt8*)source)[i * 3 + 2]) & 0x3F;

		for (j = 0; j < 4; ++j)

			*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[j]];
	}

	switch (length % 3) {
	case 0:
		break;

	case 1:
		block_of_bytes[0] = (((UInt8*)source)[i * 3 + 0]) >> 2;

		block_of_bytes[1] = ((((UInt8*)source)[i * 3 + 0]) << 4) & 0x3F;


		*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[0]];


		*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[1]];

		*destination++ = '=';

		*destination++ = '=';
		break;

	case 2:
		block_of_bytes[0] = (((UInt8*)source)[i * 3 + 0]) >> 2;

		block_of_bytes[1] = (((((UInt8*)source)[i * 3 + 0]) << 4) | ((((UInt8*)source)[i * 3 + 1]) >> 4)) & 0x3F;

		block_of_bytes[2] = ((((UInt8*)source)[i * 3 + 1]) << 2) & 0x3F;


		*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[0]];


		*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[1]];


		*destination++ = g_esif_ws_algo_asciivalue_of_six_bits[block_of_bytes[2]];

		*destination++ = '=';
		break;

	default:
		break;
	}

	*destination = '\0';
}


