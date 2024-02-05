// ____________________________ 
// ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█        │
// ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │
// █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │
// ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘
//  by Guillaume 'Aoineko' Blanchard under CC BY-SA license
//─────────────────────────────────────────────────────────────────────────────
// QR Code generator module
//─────────────────────────────────────────────────────────────────────────────
/* 
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include "qrcode_tiny.h"

#define INT16_MAX					0x7FFF
#define LONG_MAX					2147483647L // 0x7FFFFFFF

i16 abs(i16 a)
{
	return  (a > 0) ? a : -a;
}

i32 labs(i32 a)
{
	return  (a > 0) ? a : -a;
}

u16 strlen(const c8* str)
{
	u16 ret = 0;
	while (*str++)
		ret++;	
	return ret;
}

const c8* strchr(const c8* str, c8 chr)
{
	while (*str)
	{
		if (*str == chr)
			return str;
		str++;
	}
	return NULL;
}

void* memset(void* src, u8 val, u16 num)
{
	u8* d = (u8*)src;
	for (u16 i = 0; i < num; i++)
	{
		*d = val;
		d++;
	}
	return src;
}

static u8 g_Buffer[1024];

void* memmove(void* dst, const void* src, u16 num)
{
	u8* tmp = g_Buffer;
	const u8* s = (const u8*)src;
	u8* d = tmp;
	for (u16 i = 0; i < num; i++)
	{
		*d = *s;
		s++;
		d++;
	}

	s = tmp;
	d = (u8*)dst;
	for (u16 i = 0; i < num; i++)
	{
		*d = *s;
		s++;
		d++;
	}
	return dst;
}

void* memcpy(void* dst, const void* src, u16 num)
{
	const u8* s = (const u8*)src;
	u8* d = (u8*)dst;
	for (u16 i = 0; i < num; i++)
	{
		*d = *s;
		s++;
		d++;
	}
	return dst;
}

/*---- Forward declarations for private functions ----*/

// Regarding all public and private functions defined in this source file:
// - They require all pointer/array arguments to be not null unless the array length is zero.
// - They only read input scalar/array arguments, write to output pointer/array
//   arguments, and return scalar values; they are "pure" functions.
// - They don't read mutable global variables or write to any global variables.
// - They don't perform I/O, read the clock, print to console, etc.
// - They allocate a small and constant amount of stack memory.
// - They don't allocate or free any memory on the heap.
// - They don't recurse or mutually recurse. All the code
//   could be inlined into the top-level public functions.
// - They run in at most quadratic time with respect to input arguments.
//   Most functions run in linear time, and some in constant time.
//   There are no unbounded loops or non-obvious termination conditions.
// - They are completely thread-safe if the caller does not give the
//   same writable buffer to concurrent calls to these functions.

void appendBitsToBuffer(u16 val, u8 numBits, u8 buffer[], i16 *bitLen);

void addEccAndInterleave(u8 data[], enum QRCODE_ECC ecl, u8 result[]);
i16 getNumDataCodewords(enum QRCODE_ECC ecl);
i16 getNumRawDataModules();

void reedSolomonComputeDivisor(i8 degree, u8 result[]);
void reedSolomonComputeRemainder(const u8 data[], i16 dataLen, const u8 generator[], i8 degree, u8 result[]);
u8 reedSolomonMultiply(u8 x, u8 y);

void initializeFunctionModules(u8 qrcode[]);
static void drawLightFunctionModules(u8 qrcode[]);
static void drawFormatBits(enum QRCODE_ECC ecl, enum QRCODE_MASK mask, u8 qrcode[]);
i16 getAlignmentPatternPositions(u8 result[7]);
static void fillRectangle(u8 left, u8 top, u8 width, u8 height, u8 qrcode[]);

static void drawCodewords(const u8 data[], i16 dataLen, u8 qrcode[]);
static void applyMask(const u8 functionModules[], u8 qrcode[], enum QRCODE_MASK mask);
static i32 getPenaltyScore(const u8 qrcode[]);
static i16 finderPenaltyCountPatterns(const i16 runHistory[7]);
static i16 finderPenaltyTerminateAndCount(bool currentRunColor, i16 currentRunLength, i16 runHistory[7]);
static void finderPenaltyAddHistory(i16 currentRunLength, i16 runHistory[7]);

void setModuleBounded(u8 qrcode[], u8 x, u8 y, bool isDark);
void setModuleUnbounded(u8 qrcode[], u8 x, u8 y, bool isDark);
static bool getBit(i16 x, u8 i);

i16 calcSegmentBitLength(u16 numChars);
i16 getTotalBits(const struct QRCode_Segment segs[], u16 len);
static i8 numCharCountBits();

/*---- Private tables of constants ----*/

#if (!QRCODE_USE_BYTE_ONLY)
// The set of all legal characters in alphanumeric mode, where each character
// value maps to the index in the string. For checking text and encoding segments.
static const char *ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
#endif

// Sentinel value for use in only some functions.
#define LENGTH_OVERFLOW -1

// For generating error correction codes.
const i8 ECC_CODEWORDS_PER_BLOCK[4][41] = 
{
	// Version: (note that index 0 is for padding, and is set to an illegal value)
	//0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
	{-1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // Low
	{-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28},  // Medium
	{-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // Quartile
	{-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // High
};

#define QRCODE_REED_SOLOMON_DEGREE_MAX 30  // Based on the table above

// For generating error correction codes.
const i8 NUM_ERROR_CORRECTION_BLOCKS[4][41] = 
{
	// Version: (note that index 0 is for padding, and is set to an illegal value)
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
	{-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},  // Low
	{-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},  // Medium
	{-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},  // Quartile
	{-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},  // High
};

// For automatic mask pattern selection.
static const i16 PENALTY_N1 =  3;
static const i16 PENALTY_N2 =  3;
static const i16 PENALTY_N3 = 40;
static const i16 PENALTY_N4 = 10;



/*---- High-level QR Code encoding functions ----*/

// Public function - see documentation comment in header file.
bool QRCode_EncodeText(const char *text, u8 tempBuffer[], u8 qrcode[], enum QRCODE_ECC ecl, enum QRCODE_MASK mask)
{
	u16 textLen = (u16)strlen(text);
	if (textLen == 0)
		return QRCode_EncodeSegmentsAdvanced(NULL, 0, ecl, mask, tempBuffer, qrcode);
	u16 bufLen = (u16)QRCODE_BUFFER_LEN_FOR_VERSION(QRCODE_TINY_VERSION);
	
	struct QRCode_Segment seg;
	{
		if (textLen > bufLen)
			goto fail;
		for (u16 i = 0; i < textLen; i++)
			tempBuffer[i] = (u8)text[i];
		seg.mode = QRCODE_MODE_BYTE;
		seg.bitLength = calcSegmentBitLength(textLen);
		if (seg.bitLength == LENGTH_OVERFLOW)
			goto fail;
		seg.numChars = (i16)textLen;
		seg.data = tempBuffer;
	}
	return QRCode_EncodeSegmentsAdvanced(&seg, 1, ecl, mask, tempBuffer, qrcode);
	
fail:
	qrcode[0] = 0;  // Set size to invalid value for safety
	return FALSE;
}

// Appends the given number of low-order bits of the given value to the given byte-based
// bit buffer, increasing the bit length. Requires 0 <= numBits <= 16 and val < 2^numBits.
void appendBitsToBuffer(u16 val, u8 numBits, u8 buffer[], i16 *bitLen)
{
	for (i16 i = numBits - 1; i >= 0; i--, (*bitLen)++)
		buffer[*bitLen >> 3] |= ((val >> i) & 1) << (7 - (*bitLen & 7));
}

/*---- Low-level QR Code encoding functions ----*/

// Public function - see documentation comment in header file.
bool QRCode_EncodeSegmentsAdvanced(const struct QRCode_Segment segs[], u16 len, enum QRCODE_ECC ecl, enum QRCODE_MASK mask, u8 tempBuffer[], u8 qrcode[])
{
	// Find the minimal version number to use
	i16 dataUsedBits;
	{
		i16 dataCapacityBits = getNumDataCodewords(ecl) * 8;  // Number of data bits available
		dataUsedBits = getTotalBits(segs, len);
		if (dataUsedBits != LENGTH_OVERFLOW && dataUsedBits <= dataCapacityBits)
		{
			  // This version number is found to be suitable
		}
		else
		{ 
			qrcode[0] = 0;  // Set size to invalid value for safety
			return FALSE;
		}
	}
	
	// Increase the error correction level while the data still fits in the current version number
	if(QRCODE_BOOST_ECL)
	{
		for (u8 i = (u8)QRCODE_ECC_MEDIUM; i <= (u8)QRCODE_ECC_HIGH; i++) // From low to high
			if (dataUsedBits <= getNumDataCodewords((enum QRCODE_ECC)i) * 8)
				ecl = (enum QRCODE_ECC)i;
	}
	
	// Concatenate all segments to create the data bit string
	memset(qrcode, 0, (u16)QRCODE_BUFFER_LEN_FOR_VERSION(QRCODE_TINY_VERSION) * sizeof(QRCode_GetSize(qrcode)));
	i16 bitLen = 0;
	for (u16 i = 0; i < len; i++)
	{
		const struct QRCode_Segment *seg = &segs[i];
		appendBitsToBuffer((u16)seg->mode, 4, qrcode, &bitLen);
		appendBitsToBuffer((u16)seg->numChars, numCharCountBits(), qrcode, &bitLen);
		for (i16 j = 0; j < seg->bitLength; j++)
		{
			i16 bit = (seg->data[j >> 3] >> (7 - (j & 7))) & 1;
			appendBitsToBuffer((u16)bit, 1, qrcode, &bitLen);
		}
	}
	
	// Add terminator and pad up to a byte if applicable
	i16 dataCapacityBits = getNumDataCodewords(ecl) * 8;
	i16 terminatorBits = dataCapacityBits - bitLen;
	if (terminatorBits > 4)
		terminatorBits = 4;
	appendBitsToBuffer(0, terminatorBits, qrcode, &bitLen);
	appendBitsToBuffer(0, (8 - bitLen % 8) % 8, qrcode, &bitLen);
	
	// Pad with alternating bytes until data capacity is reached
	for (u8 padByte = 0xEC; bitLen < dataCapacityBits; padByte ^= 0xEC ^ 0x11)
		appendBitsToBuffer(padByte, 8, qrcode, &bitLen);
	
	// Compute ECC, draw modules
	addEccAndInterleave(qrcode, ecl, tempBuffer);
	initializeFunctionModules(qrcode);
	drawCodewords(tempBuffer, getNumRawDataModules() / 8, qrcode);
	drawLightFunctionModules(qrcode);
	initializeFunctionModules(tempBuffer);
	
	// Do masking
	if (mask == QRCODE_MASK_AUTO)
	{  // Automatically choose best mask
		i32 minPenalty = LONG_MAX;
		for (u8 i = 0; i < 8; i++)
		{
			enum QRCODE_MASK msk = (enum QRCODE_MASK)i;
			applyMask(tempBuffer, qrcode, msk);
			drawFormatBits(ecl, msk, qrcode);
			i32 penalty = getPenaltyScore(qrcode);
			if (penalty < minPenalty)
			{
				mask = msk;
				minPenalty = penalty;
			}
			applyMask(tempBuffer, qrcode, msk);  // Undoes the mask due to XOR
		}
	}
	applyMask(tempBuffer, qrcode, mask);  // Apply the final choice of mask
	drawFormatBits(ecl, mask, qrcode);  // Overwrite old format bits
	return TRUE;
}



/*---- Error correction code generation functions ----*/

// Appends error correction bytes to each block of the given data array, then interleaves
// bytes from the blocks and stores them in the result array. data[0 : dataLen] contains
// the input data. data[dataLen : rawCodewords] is used as a temporary work area and will
// be clobbered by this function. The final answer is stored in result[0 : rawCodewords].
void addEccAndInterleave(u8 data[], enum QRCODE_ECC ecl, u8 result[])
{
	// Calculate parameter numbers
	i8 numBlocks = NUM_ERROR_CORRECTION_BLOCKS[(u8)ecl][QRCODE_TINY_VERSION];
	i8 blockEccLen = ECC_CODEWORDS_PER_BLOCK[(u8)ecl][QRCODE_TINY_VERSION];
	i16 rawCodewords = getNumRawDataModules() / 8;
	i16 dataLen = getNumDataCodewords(ecl);
	i16 numShortBlocks = numBlocks - rawCodewords % numBlocks;
	i16 shortBlockDataLen = rawCodewords / numBlocks - blockEccLen;
	
	// Split data into blocks, calculate ECC, and interleave
	// (not concatenate) the bytes into a single sequence
	u8 rsdiv[QRCODE_REED_SOLOMON_DEGREE_MAX];
	reedSolomonComputeDivisor(blockEccLen, rsdiv);
	const u8 *dat = data;
	for (i8 i = 0; i < numBlocks; i++)
	{
		i16 datLen = shortBlockDataLen + (i < numShortBlocks ? 0 : 1);
		u8 *ecc = &data[dataLen];  // Temporary storage
		reedSolomonComputeRemainder(dat, datLen, rsdiv, blockEccLen, ecc);
		for (i16 j = 0, k = i; j < datLen; j++, k += numBlocks)
		{  // Copy data
			if (j == shortBlockDataLen)
				k -= numShortBlocks;
			result[k] = dat[j];
		}
		for (i16 j = 0, k = dataLen + i; j < blockEccLen; j++, k += numBlocks)  // Copy ECC
			result[k] = ecc[j];
		dat += datLen;
	}
}


// Returns the number of 8-bit codewords that can be used for storing data (not ECC),
// for the given version number and error correction level. The result is in the range [9, 2956].
i16 getNumDataCodewords(enum QRCODE_ECC ecl)
{
	u8 e = (u8)ecl;
	return getNumRawDataModules() / 8 - ECC_CODEWORDS_PER_BLOCK[e][QRCODE_TINY_VERSION] * NUM_ERROR_CORRECTION_BLOCKS[e][QRCODE_TINY_VERSION];
}


// Returns the number of data bits that can be stored in a QR Code of the given version number, after
// all function modules are excluded. This includes remainder bits, so it might not be a multiple of 8.
// The result is in the range [208, 29648]. This could be implemented as a 40-entry lookup table.
i16 getNumRawDataModules()
{
	i16 ver = (i16)QRCODE_TINY_VERSION;
	i16 result = (16 * ver + 128) * ver + 64;
	if (ver >= 2)
	{
		i16 numAlign = ver / 7 + 2;
		result -= (25 * numAlign - 10) * numAlign - 55;
		if (ver >= 7)
			result -= 36;
	}
	return result;
}



/*---- Reed-Solomon ECC generator functions ----*/

// Computes a Reed-Solomon ECC generator polynomial for the given degree, storing in result[0 : degree].
// This could be implemented as a lookup table over all possible parameter values, instead of as an algorithm.
void reedSolomonComputeDivisor(i8 degree, u8 result[])
{
	// Polynomial coefficients are stored from highest to lowest power, excluding the leading term which is always 1.
	// For example the polynomial x^3 + 255x^2 + 8x + 93 is stored as the uint8 array {255, 8, 93}.
	memset(result, 0, (u16)degree * sizeof(result[0]));
	result[degree - 1] = 1;  // Start off with the monomial x^0
	
	// Compute the product polynomial (x - r^0) * (x - r^1) * (x - r^2) * ... * (x - r^{degree-1}),
	// drop the highest monomial term which is always 1x^degree.
	// Note that r = 0x02, which is a generator element of this field GF(2^8/0x11D).
	u8 root = 1;
	for (i8 i = 0; i < degree; i++)
	{
		// Multiply the current product by (x - r^i)
		for (i8 j = 0; j < degree; j++)
		{
			result[j] = reedSolomonMultiply(result[j], root);
			if (j + 1 < degree)
				result[j] ^= result[j + 1];
		}
		root = reedSolomonMultiply(root, 0x02);
	}
}


// Computes the Reed-Solomon error correction codeword for the given data and divisor polynomials.
// The remainder when data[0 : dataLen] is divided by divisor[0 : degree] is stored in result[0 : degree].
// All polynomials are in big endian, and the generator has an implicit leading 1 term.
void reedSolomonComputeRemainder(const u8 data[], i16 dataLen, const u8 generator[], i8 degree, u8 result[])
{
	memset(result, 0, (u16)degree * sizeof(result[0]));
	for (i16 i = 0; i < dataLen; i++)
	{  // Polynomial division
		u8 factor = data[i] ^ result[0];
		memmove(&result[0], &result[1], (u16)(degree - 1) * sizeof(result[0]));
		result[degree - 1] = 0;
		for (i8 j = 0; j < degree; j++)
			result[j] ^= reedSolomonMultiply(generator[j], factor);
	}
}

#undef QRCODE_REED_SOLOMON_DEGREE_MAX


// Returns the product of the two given field elements modulo GF(2^8/0x11D).
// All inputs are valid. This could be implemented as a 256*256 lookup table.
u8 reedSolomonMultiply(u8 x, u8 y)
{
	// Russian peasant multiplication
	u8 z = 0;
	for (i8 i = 7; i >= 0; i--) {
		z = (u8)((z << 1) ^ ((z >> 7) * 0x11D));
		z ^= ((y >> i) & 1) * x;
	}
	return z;
}



/*---- Drawing function modules ----*/

// Clears the given QR Code grid with light modules for the given
// version's size, then marks every function module as dark.
void initializeFunctionModules(u8 qrcode[])
{
	// Initialize QR Code
	memset(qrcode, 0, (u16)((QRCODE_SIZE * QRCODE_SIZE + 7) / 8 + 1) * sizeof(QRCode_GetSize(qrcode)));
	qrcode[0] = QRCODE_SIZE;
	
	// Fill horizontal and vertical timing patterns
	fillRectangle(6, 0, 1, QRCODE_SIZE, qrcode);
	fillRectangle(0, 6, QRCODE_SIZE, 1, qrcode);
	
	// Fill 3 finder patterns (all corners except bottom right) and format bits
	fillRectangle(0, 0, 9, 9, qrcode);
	fillRectangle(QRCODE_SIZE - 8, 0, 8, 9, qrcode);
	fillRectangle(0, QRCODE_SIZE - 8, 9, 8, qrcode);
	
	// Fill numerous alignment patterns
	u8 alignPatPos[7];
	i16 numAlign = getAlignmentPatternPositions(alignPatPos);
	for (i16 i = 0; i < numAlign; i++)
	{
		for (i16 j = 0; j < numAlign; j++)
		{
			// Don't draw on the three finder corners
			if (!((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0)))
				fillRectangle(alignPatPos[i] - 2, alignPatPos[j] - 2, 5, 5, qrcode);
		}
	}
	
	// Fill version blocks
	if (QRCODE_TINY_VERSION >= 7)
	{
		fillRectangle(QRCODE_SIZE - 11, 0, 3, 6, qrcode);
		fillRectangle(0, QRCODE_SIZE - 11, 6, 3, qrcode);
	}
}


// Draws light function modules and possibly some dark modules onto the given QR Code, without changing
// non-function modules. This does not draw the format bits. This requires all function modules to be previously
// marked dark (namely by initializeFunctionModules()), because this may skip redrawing dark function modules.
static void drawLightFunctionModules(u8 qrcode[])
{
	// Draw horizontal and vertical timing patterns
	for (u8 i = 7; i < QRCODE_SIZE - 7; i += 2)
	{
		setModuleBounded(qrcode, 6, i, FALSE);
		setModuleBounded(qrcode, i, 6, FALSE);
	}
	
	// Draw 3 finder patterns (all corners except bottom right; overwrites some timing modules)
	for (i8 dy = -4; dy <= 4; dy++)
	{
		for (i8 dx = -4; dx <= 4; dx++)
		{
			i8 dist = abs(dx);
			if (abs(dy) > dist)
				dist = abs(dy);
			if (dist == 2 || dist == 4)
			{
				setModuleUnbounded(qrcode, 3 + dx, 3 + dy, FALSE);
				setModuleUnbounded(qrcode, QRCODE_SIZE - 4 + dx, 3 + dy, FALSE);
				setModuleUnbounded(qrcode, 3 + dx, QRCODE_SIZE - 4 + dy, FALSE);
			}
		}
	}
	
	// Draw numerous alignment patterns
	u8 alignPatPos[7];
	i16 numAlign = getAlignmentPatternPositions(alignPatPos);
	for (i16 i = 0; i < numAlign; i++)
	{
		for (i16 j = 0; j < numAlign; j++)
		{
			if ((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0))
				continue;  // Don't draw on the three finder corners
			for (i8 dy = -1; dy <= 1; dy++)
				for (i8 dx = -1; dx <= 1; dx++)
					setModuleBounded(qrcode, alignPatPos[i] + dx, alignPatPos[j] + dy, dx == 0 && dy == 0);
		}
	}
	
	// Draw version blocks
	if (QRCODE_TINY_VERSION >= 7)
	{
		// Calculate error correction code and pack bits
		i16 rem = QRCODE_TINY_VERSION;  // version is uint6, in the range [7, 40]
		for (u8 i = 0; i < 12; i++)
			rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
		i32 bits = (i32)QRCODE_TINY_VERSION << 12 | rem;  // uint18
		
		// Draw two copies
		for (u8 i = 0; i < 6; i++)
		{
			for (u8 j = 0; j < 3; j++)
			{
				u8 k = QRCODE_SIZE - 11 + j;
				setModuleBounded(qrcode, k, i, (bits & 1) != 0);
				setModuleBounded(qrcode, i, k, (bits & 1) != 0);
				bits >>= 1;
			}
		}
	}
}


// Draws two copies of the format bits (with its own error correction code) based
// on the given mask and error correction level. This always draws all modules of
// the format bits, unlike drawLightFunctionModules() which might skip dark modules.
static void drawFormatBits(enum QRCODE_ECC ecl, enum QRCODE_MASK mask, u8 qrcode[])
{
	// Calculate error correction code and pack bits
	static const i16 table[] = {1, 0, 3, 2};
	i16 data = table[(i16)ecl] << 3 | (i16)mask;  // errCorrLvl is uint2, mask is uint3
	i16 rem = data;
	for (i8 i = 0; i < 10; i++)
		rem = (rem << 1) ^ ((rem >> 9) * 0x537);
	i16 bits = (data << 10 | rem) ^ 0x5412;  // uint15
	
	// Draw first copy
	for (u8 i = 0; i <= 5; i++)
		setModuleBounded(qrcode, 8, i, getBit(bits, i));
	setModuleBounded(qrcode, 8, 7, getBit(bits, 6));
	setModuleBounded(qrcode, 8, 8, getBit(bits, 7));
	setModuleBounded(qrcode, 7, 8, getBit(bits, 8));
	for (u8 i = 9; i < 15; i++)
		setModuleBounded(qrcode, 14 - i, 8, getBit(bits, i));
	
	// Draw second copy
	for (u8 i = 0; i < 8; i++)
		setModuleBounded(qrcode, QRCODE_SIZE - 1 - i, 8, getBit(bits, i));
	for (u8 i = 8; i < 15; i++)
		setModuleBounded(qrcode, 8, QRCODE_SIZE - 15 + i, getBit(bits, i));
	setModuleBounded(qrcode, 8, QRCODE_SIZE - 8, TRUE);  // Always dark
}


// Calculates and stores an ascending list of positions of alignment patterns
// for this version number, returning the length of the list (in the range [0,7]).
// Each position is in the range [0,177), and are used on both the x and y axes.
// This could be implemented as lookup table of 40 variable-length lists of unsigned bytes.
i16 getAlignmentPatternPositions(u8 result[7]) 
{
	if (QRCODE_TINY_VERSION == 1)
		return 0;
	u8 numAlign = QRCODE_TINY_VERSION / 7 + 2;
	i16 step = (QRCODE_TINY_VERSION == 32) ? 26 : (QRCODE_TINY_VERSION * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
	for (i16 i = numAlign - 1, pos = QRCODE_TINY_VERSION * 4 + 10; i >= 1; i--, pos -= step)
		result[i] = (u8)pos;
	result[0] = 6;
	return numAlign;
}


// Sets every module in the range [left : left + width] * [top : top + height] to dark.
static void fillRectangle(u8 left, u8 top, u8 width, u8 height, u8 qrcode[]) 
{
	for (u8 dy = 0; dy < height; dy++)
		for (u8 dx = 0; dx < width; dx++)
			setModuleBounded(qrcode, left + dx, top + dy, TRUE);
}



/*---- Drawing data modules and masking ----*/

// Draws the raw codewords (including data and ECC) onto the given QR Code. This requires the initial state of
// the QR Code to be dark at function modules and light at codeword modules (including unused remainder bits).
static void drawCodewords(const u8 data[], i16 dataLen, u8 qrcode[]) 
{
	i16 i = 0;  // Bit index into the data
	// Do the funny zigzag scan
	for (i16 right = QRCODE_SIZE - 1; right >= 1; right -= 2) 
	{  // Index of right column in each column pair
		if (right == 6)
			right = 5;
		for (i16 vert = 0; vert < QRCODE_SIZE; vert++) 
		{  // Vertical counter
			for (i8 j = 0; j < 2; j++) 
			{
				i16 x = right - j;  // Actual x coordinate
				bool upward = ((right + 1) & 2) == 0;
				i16 y = upward ? QRCODE_SIZE - 1 - vert : vert;  // Actual y coordinate
				if (!QRCode_GetModule(qrcode, x, y) && i < dataLen * 8) 
				{
					bool dark = getBit(data[i >> 3], 7 - (i & 7));
					setModuleBounded(qrcode, x, y, dark);
					i++;
				}
				// If this QR Code has any remainder bits (0 to 7), they were assigned as
				// 0/FALSE/light by the constructor and are left unchanged by this method
			}
		}
	}
}


// XORs the codeword modules in this QR Code with the given mask pattern
// and given pattern of function modules. The codeword bits must be drawn
// before masking. Due to the arithmetic of XOR, calling applyMask() with
// the same mask value a second time will undo the mask. A final well-formed
// QR Code needs exactly one (not zero, two, etc.) mask applied.
static void applyMask(const u8 functionModules[], u8 qrcode[], enum QRCODE_MASK mask) 
{
	for (u8 y = 0; y < QRCODE_SIZE; y++) 
	{
		for (u8 x = 0; x < QRCODE_SIZE; x++) 
		{
			if (QRCode_GetModule(functionModules, x, y))
				continue;
			bool invert;
			switch ((i16)mask) 
			{
				case 0:  invert = (x + y) % 2 == 0;                    break;
				case 1:  invert = y % 2 == 0;                          break;
				case 2:  invert = x % 3 == 0;                          break;
				case 3:  invert = (x + y) % 3 == 0;                    break;
				case 4:  invert = (x / 3 + y / 2) % 2 == 0;            break;
				case 5:  invert = x * y % 2 + x * y % 3 == 0;          break;
				case 6:  invert = (x * y % 2 + x * y % 3) % 2 == 0;    break;
				case 7:  invert = ((x + y) % 2 + x * y % 3) % 2 == 0;  break;
				default: return;
			}
			bool val = QRCode_GetModule(qrcode, x, y);
			setModuleBounded(qrcode, x, y, val ^ invert);
		}
	}
}


// Calculates and returns the penalty score based on state of the given QR Code's current modules.
// This is used by the automatic mask choice algorithm to find the mask pattern that yields the lowest score.
static i32 getPenaltyScore(const u8 qrcode[]) 
{
	i32 result = 0;
	
	// Adjacent modules in row having same color, and finder-like patterns
	for (u8 y = 0; y < QRCODE_SIZE; y++) 
	{
		bool runColor = FALSE;
		i16 runX = 0;
		i16 runHistory[7] = {0};
		for (u8 x = 0; x < QRCODE_SIZE; x++) 
		{
			if (QRCode_GetModule(qrcode, x, y) == runColor) 
			{
				runX++;
				if (runX == 5)
					result += PENALTY_N1;
				else if (runX > 5)
					result++;
			}
			else 
			{
				finderPenaltyAddHistory(runX, runHistory);
				if (!runColor)
					result += finderPenaltyCountPatterns(runHistory) * PENALTY_N3;
				runColor = QRCode_GetModule(qrcode, x, y);
				runX = 1;
			}
		}
		result += finderPenaltyTerminateAndCount(runColor, runX, runHistory) * PENALTY_N3;
	}
	// Adjacent modules in column having same color, and finder-like patterns
	for (u8 x = 0; x < QRCODE_SIZE; x++) 
	{
		bool runColor = FALSE;
		i16 runY = 0;
		i16 runHistory[7] = {0};
		for (u8 y = 0; y < QRCODE_SIZE; y++) 
		{
			if (QRCode_GetModule(qrcode, x, y) == runColor) 
			{
				runY++;
				if (runY == 5)
					result += PENALTY_N1;
				else if (runY > 5)
					result++;
			}
			else
			{
				finderPenaltyAddHistory(runY, runHistory);
				if (!runColor)
					result += finderPenaltyCountPatterns(runHistory) * PENALTY_N3;
				runColor = QRCode_GetModule(qrcode, x, y);
				runY = 1;
			}
		}
		result += finderPenaltyTerminateAndCount(runColor, runY, runHistory) * PENALTY_N3;
	}
	
	// 2*2 blocks of modules having same color
	for (u8 y = 0; y < QRCODE_SIZE - 1; y++) 
	{
		for (u8 x = 0; x < QRCODE_SIZE - 1; x++) 
		{
			bool color = QRCode_GetModule(qrcode, x, y);
			if (color == QRCode_GetModule(qrcode, x + 1, y) && color == QRCode_GetModule(qrcode, x, y + 1) && color == QRCode_GetModule(qrcode, x + 1, y + 1))
				result += PENALTY_N2;
		}
	}
	
	// Balance of dark and light modules
	i16 dark = 0;
	for (u8 y = 0; y < QRCODE_SIZE; y++) 
	{
		for (u8 x = 0; x < QRCODE_SIZE; x++) 
		{
			if (QRCode_GetModule(qrcode, x, y))
				dark++;
		}
	}
	i16 total = QRCODE_SIZE * QRCODE_SIZE;  // Note that size is odd, so dark/total != 1/2
	// Compute the smallest integer k >= 0 such that (45-5k)% <= dark/total <= (55+5k)%
	i16 k = (i16)((labs(dark * 20L - total * 10L) + total - 1) / total) - 1;
	result += k * PENALTY_N4;
	return result;
}


// Can only be called immediately after a light run is added, and
// returns either 0, 1, or 2. A helper function for getPenaltyScore().
static i16 finderPenaltyCountPatterns(const i16 runHistory[7]) 
{
	i16 n = runHistory[1];
	bool core = n > 0 && runHistory[2] == n && runHistory[3] == n * 3 && runHistory[4] == n && runHistory[5] == n;
	// The maximum QR Code size is 177, hence the dark run length n <= 177.
	// Arithmetic is promoted to i16, so n*4 will not overflow.
	return (core && runHistory[0] >= n * 4 && runHistory[6] >= n ? 1 : 0) + (core && runHistory[6] >= n * 4 && runHistory[0] >= n ? 1 : 0);
}


// Must be called at the end of a line (row or column) of modules. A helper function for getPenaltyScore().
static i16 finderPenaltyTerminateAndCount(bool currentRunColor, i16 currentRunLength, i16 runHistory[7]) 
{
	if (currentRunColor) 
	{  // Terminate dark run
		finderPenaltyAddHistory(currentRunLength, runHistory);
		currentRunLength = 0;
	}
	currentRunLength += QRCODE_SIZE;  // Add light border to final run
	finderPenaltyAddHistory(currentRunLength, runHistory);
	return finderPenaltyCountPatterns(runHistory);
}


// Pushes the given value to the front and drops the last value. A helper function for getPenaltyScore().
static void finderPenaltyAddHistory(i16 currentRunLength, i16 runHistory[7]) 
{
	if (runHistory[0] == 0)
		currentRunLength += QRCODE_SIZE;  // Add light border to initial run
	memmove(&runHistory[1], &runHistory[0], 6 * sizeof(runHistory[0]));
	runHistory[0] = currentRunLength;
}



/*---- Basic QR Code information ----*/

// Public function - Returns the color of the module at the given coordinates, which must be in bounds.
bool QRCode_GetModule(const u8 qrcode[], u8 x, u8 y)
{
	i16 index = y * QRCODE_SIZE + x;
	return getBit(qrcode[(index >> 3) + 1], index & 7);
}

// Returns the color of the module at the given coordinates, which must be in bounds.
u8 QRCode_GetByte(const u8 qrcode[], u8 x, u8 y)
{
	i16 index = y * QRCODE_SIZE + x;
	return qrcode[(index >> 3) + 1];
}

// Sets the color of the module at the given coordinates, which must be in bounds.
void setModuleBounded(u8 qrcode[], u8 x, u8 y, bool isDark)
{
	i16 index = y * QRCODE_SIZE + x;
	i16 bitIndex = index & 7;
	i16 byteIndex = (index >> 3) + 1;
	if (isDark)
		qrcode[byteIndex] |= 1 << bitIndex;
	else
		qrcode[byteIndex] &= (1 << bitIndex) ^ 0xFF;
}


// Sets the color of the module at the given coordinates, doing nothing if out of bounds.
void setModuleUnbounded(u8 qrcode[], u8 x, u8 y, bool isDark) 
{
	if (0 <= x && x < QRCODE_SIZE && 0 <= y && y < QRCODE_SIZE)
		setModuleBounded(qrcode, x, y, isDark);
}

//static const u16 g_Bits[16] = 
//{
//	0b0000000000000001,
//	0b0000000000000010,
//	0b0000000000000100,
//	0b0000000000001000,
//	0b0000000000010000,
//	0b0000000000100000,
//	0b0000000001000000,
//	0b0000000010000000,
//	0b0000000100000000,
//	0b0000001000000000,
//	0b0000010000000000,
//	0b0000100000000000,
//	0b0001000000000000,
//	0b0010000000000000,
//	0b0100000000000000,
//	0b1000000000000000,
//};

// Returns TRUE iff the i'th bit of x is set to 1. Requires x >= 0 and 0 <= i <= 14.
static bool getBit(i16 x, u8 i) 
{
	return ((x >> i) & 1) != 0;
	//return (g_Bits[i] & x) != 0;
}



/*---- Segment handling ----*/

// Returns the number of data bits needed to represent a segment
// containing the given number of characters using the given mode. Notes:
// - Returns LENGTH_OVERFLOW on failure, i.e. numChars > INT16_MAX
//   or the number of needed bits exceeds INT16_MAX (i.e. 32767).
// - Otherwise, all valid results are in the range [0, INT16_MAX].
// - For byte mode, numChars measures the number of bytes, not Unicode code points.
// - For ECI mode, numChars must be 0, and the worst-case number of bits is returned.
//   An actual ECI segment can have shorter data. For non-ECI modes, the result is exact.
i16 calcSegmentBitLength(u16 numChars) 
{
	// All calculations are designed to avoid overflow on all platforms
	if (numChars > (u16)INT16_MAX)
		return LENGTH_OVERFLOW;
	i32 result = (i32)numChars;
	result *= 8;
	if (result > INT16_MAX)
		return LENGTH_OVERFLOW;
	return (i16)result;
}

// Public function - see documentation comment in header file.

// Calculates the number of bits needed to encode the given segments at the given version.
// Returns a non-negative number if successful. Otherwise returns LENGTH_OVERFLOW if a segment
// has too many characters to fit its length field, or the total bits exceeds INT16_MAX.
i16 getTotalBits(const struct QRCode_Segment segs[], u16 len) 
{
	i32 result = 0;
	for (u16 i = 0; i < len; i++) 
	{
		i16 numChars  = segs[i].numChars;
		i16 bitLength = segs[i].bitLength;
		i8 ccbits = numCharCountBits();
		if (numChars >= (1L << ccbits))
			return LENGTH_OVERFLOW;  // The segment's length doesn't fit the field's bit width
		result += 4L + ccbits + bitLength;
		if (result > INT16_MAX)
			return LENGTH_OVERFLOW;  // The sum might overflow an i16 type
	}
	return (i16)result;
}


// Returns the bit width of the character count field for a segment in the given mode
// in a QR Code at the given version number. The result is in the range [0, 16].
static i8 numCharCountBits()
{
	i8 i = (QRCODE_TINY_VERSION + 7) / 17;
	static const i8 temp[] = { 8, 16, 16 };
	return temp[i];
}


#undef LENGTH_OVERFLOW
