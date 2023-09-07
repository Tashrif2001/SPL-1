#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
//#include <openssl/rand.h>
#include "aes.h"
#define key_size 32

void pad_bytes(unsigned char *byteStream, size_t *len)
{
    size_t padLen = 16-(*len % 16);
    for(size_t i=0; i<padLen; i++)
    {
        byteStream[*len +i] = (unsigned char)padLen;
    }
    *len+=padLen;
}

void remove_padding(unsigned char *byteStream, size_t *len)
{
    if (*len == 0) {
        return;
    }

    unsigned char padLen = byteStream[*len - 1];

    if (padLen <= 16 && padLen > 0) {
        for (size_t i = *len - padLen; i < *len; i++) {
            if (byteStream[i] != padLen) {
                printf("Invalid padding!\n");
                return;
            }
        }
        *len -= padLen;
    } else {
        printf("Invalid padding value!\n");
    }
}

void stateArray(const unsigned char *byteStream, unsigned char state[4][4])
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            int idx = i*4+j;
            state[j][i]=byteStream[idx];
        }
    }
}
void read_file_to_byteStream(unsigned char *byteStream, unsigned char state[4][4])
{
    FILE *file = fopen("input.txt", "rb");
    if(file==NULL)
    {
        printf("Error reading file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    size_t len=0;

    while(len<size)
    {
        size_t read = fread(byteStream+len, 1, 16, file);
        len+=read;
    }
    fclose(file);
}

void decrypt(unsigned char state[4][4], unsigned char round_keys[240])
{
    FILE *decrypted = fopen("output.txt", "r");

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            unsigned char val;
            fscanf(decrypted, "%02x ", &val);
            state[i][j] = val;
        }
    }

    fclose(decrypted);

    printf("Last round (Decryption):\n");
    add_round_key(state, round_keys, 14);

    for (int round = 13; round >= 1; round--) {
        printf("Round %d (Decryption):\n", round);
        inv_shift_row(state);
        inv_substitute(state);
        add_round_key(state, round_keys, round);
        invMixCol(state);
    }
        inv_shift_row(state);
        inv_substitute(state);
        add_round_key(state, round_keys, 0);


    unsigned char decryptedOutput[256];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            decryptedOutput[i * 4 + j] = state[j][i];
        }
    }

}

void encrypt(unsigned char state[4][4], unsigned char round_keys[])
{
    add_round_key(state, round_keys, 0);

    for(int round=1; round<14; round++)
    {
        printf("Round %d: \n", round);
        substitute(state);
        shift_row(state);
        mixCol(state);
        add_round_key(state, round_keys, round);
    }

    printf("Last round:\n");
    substitute(state);
    shift_row(state);
    add_round_key(state, round_keys, 14);

    FILE *encrypted = fopen("output.txt", "w");
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
            fprintf(encrypted, "%02x ", state[i][j]);
    }
    fclose(encrypted);
}

int main()
{
    unsigned char byteStream[256];
    unsigned char state[4][4];
    unsigned char key[32];
    unsigned char round_keys[240];

    read_file_to_byteStream(byteStream, state);

    //pad_bytes(byteStream, &len);

    stateArray(byteStream, state);
    key_generation(key);
    key_expansion(key, round_keys);

    printf("Before encryption: \n");
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            printf("%02x ", state[i][j]);
        }
        printf("\n");
    }

    encrypt(state, round_keys);
    printf("\nAfter encryption: \n");

     for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            printf("%02x ", state[i][j]);
        }
        printf("\n");
    }

    decrypt(state, round_keys);
    printf("\nAfter decryption: \n");
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            printf("%02x ", state[i][j]);
        }
        printf("\n");
    }
    //huffman();
    return 0;
}
