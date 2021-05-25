#include "hmac.h"
#include "sha.h"
#include "hmac.h"
#include "string.h"

#define HMAC_IPAD   0x36
#define HMAC_OPAD   0x5C

void hmac_init(hmac_ctx* ctx, const u8* key, int size)
{
    int i;

    memset(ctx->key, 0, sizeof(ctx->key));

    if (size > sizeof(ctx->key))
        sha_hash(key, ctx->key, size);
    else
        memcpy(ctx->key, key, size);

    for (i = 0; i < sizeof(ctx->key); i++)
        ctx->key[i] ^= HMAC_IPAD;

    sha_init(&ctx->hash_ctx);
    sha_update(&ctx->hash_ctx, ctx->key, sizeof(ctx->key));
}

void hmac_update(hmac_ctx* ctx, const void* data, int size)
{
    sha_update(&ctx->hash_ctx, data, size);
}

void hmac_final(hmac_ctx* ctx, u8* hmac)
{
    u8 hash[SHA_HASH_SIZE];
    int i;

    sha_final(&ctx->hash_ctx, hash);

    for (i = 0; i < sizeof(ctx->key); i++)
        ctx->key[i] ^= HMAC_IPAD ^ HMAC_OPAD;

    sha_init(&ctx->hash_ctx);
    sha_update(&ctx->hash_ctx, ctx->key, sizeof(ctx->key));
    sha_update(&ctx->hash_ctx, hash, sizeof(hash));
    sha_final(&ctx->hash_ctx, hmac);
}
