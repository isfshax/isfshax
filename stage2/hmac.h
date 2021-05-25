#ifndef _HMAC_H
#define _HMAC_H

#include "types.h"
#include "sha.h"

#define HMAC_SIZE   (SHA_HASH_SIZE)

typedef struct {
	u8 key[SHA_BLOCK_SIZE];
	sha_ctx hash_ctx;
} hmac_ctx;

void hmac_init(hmac_ctx* ctx, const u8* key, int size);
void hmac_update(hmac_ctx* ctx, const void* data, int size);
void hmac_final(hmac_ctx *ctx, u8 *hmac); 

#endif /* _HMAC_H */
