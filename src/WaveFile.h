#ifndef _MODCVT_WAVE_FILE_H
#define _MODCVT_WAVE_FILE_H

#include <inttypes.h>

//////////////////////////////////////////////////////////
// WAVE format information

#pragma pack(1)

// Standard IFF chunks IDs
#define IFFID_FORM		0x4d524f46
#define IFFID_RIFF		0x46464952
#define IFFID_WAVE		0x45564157
#define IFFID_LIST		0x5453494C
#define IFFID_INFO		0x4F464E49

// IFF Info fields
#define IFFID_ICOP		0x504F4349
#define IFFID_IART		0x54524149
#define IFFID_IPRD		0x44525049
#define IFFID_INAM		0x4D414E49
#define IFFID_ICMT		0x544D4349
#define IFFID_IENG		0x474E4549
#define IFFID_ISFT		0x54465349
#define IFFID_ISBJ		0x4A425349
#define IFFID_IGNR		0x524E4749
#define IFFID_ICRD		0x44524349

// Wave IFF chunks IDs
#define IFFID_wave		0x65766177
#define IFFID_fmt		0x20746D66
#define IFFID_wsmp		0x706D7377
#define IFFID_pcm		0x206d6370
#define IFFID_data		0x61746164
#define IFFID_smpl		0x6C706D73
#define IFFID_xtra		0x61727478

typedef struct WAVEFILEHEADER
{
	uint32_t id_RIFF;		// "RIFF"
	uint32_t filesize;		// file length-8
	uint32_t id_WAVE;
} WAVEFILEHEADER;


typedef struct WAVEFORMATHEADER
{
	uint32_t id_fmt;		// "fmt "
	uint32_t hdrlen;		// 16
	uint16_t format;		// 1
	uint16_t channels;		// 1:mono, 2:stereo
	uint32_t freqHz;		// sampling freq
	uint32_t bytessec;		// bytes/sec=freqHz*samplesize
	uint16_t samplesize;	// sizeof(sample)
	uint16_t bitspersample;	// bits per sample (8/16)
} WAVEFORMATHEADER;


typedef struct WAVEDATAHEADER
{
	uint32_t id_data;		// "data"
	uint32_t length;		// length of data
} WAVEDATAHEADER;


typedef struct WAVESMPLHEADER
{
	// SMPL
	uint32_t smpl_id;		// "smpl"	-> 0x6C706D73
	uint32_t smpl_len;		// length of smpl: 3Ch	(54h with sustain loop)
	uint32_t dwManufacturer;
	uint32_t dwProduct;
	uint32_t dwSamplePeriod;	// 1000000000/freqHz
	uint32_t dwBaseNote;	// 3Ch = C-4 -> 60 + RelativeTone
	uint32_t dwPitchFraction;
	uint32_t dwSMPTEFormat;
	uint32_t dwSMPTEOffset;
	uint32_t dwSampleLoops;	// number of loops
	uint32_t cbSamplerData;
} WAVESMPLHEADER;


#endif
