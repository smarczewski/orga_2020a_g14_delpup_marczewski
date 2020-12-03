#include "recode57.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum encoding bom_to_enc(uint8_t *bom){
	if(bom[0] == 0xFE && bom[1] == 0xFF)
		return UTF16BE;
	else if(bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF)
		return UTF32BE;
	else if(bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00)
		return UTF32LE;
	else if(bom[0] == 0xFF && bom[1] == 0xFE)
		return UTF16LE;
	else
		return UTF8;
};

size_t write_bom(enum encoding enc, uint8_t *buf){
    if(enc == UTF32LE){
        buf[0] = 0xFE;
        buf[1] = 0xFE;
        buf[2] = 0x00;
        buf[3] = 0x00;
        return 4;
    }
    else if(enc == UTF32BE){
        buf[0] = 0x00;
        buf[1] = 0x00;
        buf[2] = 0xFE;
        buf[3] = 0xFF;
        return 4;
    }
    else if(enc == UTF16BE){
        buf[0] = 0xFE;
        buf[1] = 0xFF;
        return 2;
    }
    else if(enc == UTF32LE){
        buf[0] = 0xFF;
        buf[1] = 0xFE;
        return 2;
    }
    else{
        return 0;
    }
};

bool has_codepoint(enum encoding enc, uint8_t *buf, size_t nbytes){
    return nbytes >= 4;

};


size_t read_codepoint(enum encoding enc,
                      uint8_t *buf,
                      size_t nbytes,
                      codepoint_t *dest){
    if(enc == UTF32LE){
        *dest = buf[0];
        *dest <<= 24;
        *dest |= buf[1];
        *dest <<= 16;
        *dest |= buf[0];
        *dest <<= 8;
        *dest |= buf[3];
    }
    if(enc == UTF32BE){
        *dest = buf[3];
        *dest <<= 24;
        *dest |= buf[2];
        *dest <<= 16;
        *dest |= buf[1];
        *dest <<= 8;
        *dest |= buf[0];
    }
    return 4;
};


size_t write_codepoint(enum encoding enc,
                       codepoint_t codepoint,
                       uint8_t *outbuf){
    if(enc == UTF32LE){
        outbuf[0] = codepoint;
        outbuf[1] &= codepoint >> 8;
        outbuf[2] &= codepoint >> 16;
        outbuf[3] &= codepoint >> 24;
    }
    if(enc == UTF32BE){
        outbuf[0] = codepoint >> 24;
        outbuf[1] &= codepoint >> 16;
        outbuf[2] &= codepoint >> 8;
        outbuf[3] &= codepoint;
    }
    return 4;

};
