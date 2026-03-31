#include "bootloader.h"
#include <string.h>

// Simple SHA-256 implementation (simplified for demonstration)
// In production, use mbedTLS or hardware crypto

static const uint32_t SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n) ((x) >> (n))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIGMA0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIGMA1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIGMA_0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define SIGMA_1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

void compute_sha256(uint8_t* data, uint32_t size, uint8_t* hash)
{
    // Simplified SHA-256 implementation
    // In production, use hardware crypto or mbedTLS
    
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    // Pad the data
    uint64_t bit_len = size * 8;
    uint32_t padded_size = ((size + 8 + 63) / 64) * 64;
    uint8_t* padded_data = (uint8_t*)malloc(padded_size);
    
    if (!padded_data) {
        memset(hash, 0, 32);
        return;
    }
    
    memcpy(padded_data, data, size);
    padded_data[size] = 0x80;
    
    for (uint32_t i = size + 1; i < padded_size - 8; i++) {
        padded_data[i] = 0;
    }
    
    // Append length
    for (int i = 0; i < 8; i++) {
        padded_data[padded_size - 8 + i] = (bit_len >> (56 - i * 8)) & 0xFF;
    }
    
    // Process blocks
    for (uint32_t offset = 0; offset < padded_size; offset += 64) {
        uint32_t w[64];
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], h_val = h[7];
        
        // Prepare message schedule
        for (int t = 0; t < 16; t++) {
            w[t] = (padded_data[offset + t * 4] << 24) |
                   (padded_data[offset + t * 4 + 1] << 16) |
                   (padded_data[offset + t * 4 + 2] << 8) |
                   padded_data[offset + t * 4 + 3];
        }
        
        for (int t = 16; t < 64; t++) {
            w[t] = SIGMA_1(w[t - 2]) + w[t - 7] + SIGMA_0(w[t - 15]) + w[t - 16];
        }
        
        // Main loop
        for (int t = 0; t < 64; t++) {
            uint32_t t1 = h_val + SIGMA1(e) + CH(e, f, g) + SHA256_K[t] + w[t];
            uint32_t t2 = SIGMA0(a) + MAJ(a, b, c);
            
            h_val = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        
        // Update hash values
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
    }
    
    // Output hash
    for (int i = 0; i < 8; i++) {
        hash[i * 4] = (h[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = h[i] & 0xFF;
    }
    
    free(padded_data);
}

uint32_t compute_crc32(uint32_t addr, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;
    uint8_t* data = (uint8_t*)addr;
    
    for (uint32_t i = 0; i < size; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 * (crc & 1));
        }
    }
    
    return ~crc;
}

uint16_t compute_crc16(uint8_t* data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (CRC16_POLY * (crc & 1));
        }
    }
    
    return crc;
}
