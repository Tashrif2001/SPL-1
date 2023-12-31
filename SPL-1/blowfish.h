#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_SIZE 56
#define MAX_TEXT_SIZE 4096

uint32_t P[18];
uint32_t S[4][256];

uint32_t f(uint32_t x)
{
    uint32_t h = S[0][x >> 24] + S[1][(x >> 16) & 0xff];
    return (h ^ S[2][(x >> 8) & 0xff]) + S[3][x & 0xff];
}

void blowfish_encrypt(uint32_t *L, uint32_t *R)
{
    for (short r = 0; r < 16; r++)
    {
        *L = *L ^ P[r];
        *R = f(*L) ^ *R;
     
        *R ^= *L;
        *L ^= *R;
        *R ^= *L;
    }

    *R ^= *L;
    *L ^= *R;
    *R ^= *L;
    *R = *R ^ P[16];
    *L = *L ^ P[17];
}

void blowfish_decrypt(uint32_t *L, uint32_t *R)
{
    for (short r = 17; r > 1; r--)
    {
        *L = *L ^ P[r];
        *R = f(*L) ^ *R;
       
        *R ^= *L;
        *L ^= *R;
        *R ^= *L;
    }
   
    *R ^= *L;
    *L ^= *R;
    *R ^= *L;
    *R = *R ^ P[1];
    *L = *L ^ P[0];
}


void blowfish_key_expand(const uint8_t *key, int key_len, char *input_file_path)
{
    uint32_t k;
    for (short i = 0, p = 0; i < 18; i++)
    {
        k = 0x00;
        for (short j = 0; j < 4; j++)
        {
            k = (k << 8) | (uint8_t)key[p];
            p = (p + 1) % key_len;
        }
        P[i] ^= k;
    }

    uint32_t l = 0x00, r = 0x00;
    for (short i = 0; i < 18; i += 2)
    {
        blowfish_encrypt(&l, &r);
        P[i] = l;
        P[i + 1] = r;
    }
    for (short i = 0; i < 4; i++)
    {
        for (short j = 0; j < 256; j += 2)
        {
            blowfish_encrypt(&l, &r);
            S[i][j] = l;
            S[i][j + 1] = r;
        }
    }

    char keyName[1000];
    strcpy(keyName, input_file_path);

    char SkeyName[1000];
    strcpy(SkeyName, input_file_path);

    int dot_position = -1;
    for (int i = strlen(keyName) - 1; i >= 0; i--)
    {
        if (keyName[i] == '.')
        {
            dot_position = i;
            break;
        }
    }

    if (dot_position == -1)
    {
        printf("Invalid file name.\n");
        exit(1);
    }

    strcpy(keyName + dot_position, ".Blowkey.txt");

    dot_position = -1;
    for (int i = strlen(SkeyName) - 1; i >= 0; i--)
    {
        if (SkeyName[i] == '.')
        {
            dot_position = i;
            break;
        }
    }

    strcpy(SkeyName + dot_position, ".Skey.txt");

    FILE *keyOutput = fopen(keyName, "wb");
    FILE *sKeyOutput = fopen(SkeyName, "wb");

    if (!keyOutput)
    {
        perror("Error opening key output file");
        exit(1);
    }
    if (!sKeyOutput)
    {
        perror("Error opening S key output file");
        fclose(keyOutput);
        exit(1);
    }

    if (fwrite(P, sizeof(uint32_t), 18, keyOutput) != 18)
    {
        perror("Error writing P array to the key output file");
        fclose(keyOutput);
        fclose(sKeyOutput);
        exit(1);
    }


    if (fwrite(S, sizeof(uint32_t), 4 * 256, sKeyOutput) != 4 * 256)
    {
        perror("Error writing S array to the S key output file");
        fclose(keyOutput);
        fclose(sKeyOutput);
        exit(1);
    }

    fclose(keyOutput);
    fclose(sKeyOutput);
}

void read_key_files(char *input_file_path) {

    char keyName[1000];
    char SkeyName[1000];
    strcpy(keyName, input_file_path);
    strcpy(SkeyName, input_file_path);

    int dot_position = -1;
    for (int i = strlen(keyName) - 1; i >= 0; i--) {
        if (keyName[i] == '.') {
            dot_position = i;
            break;
        }
    }

    if (dot_position == -1) {
        printf("Invalid file name.\n");
        exit(1);
    }

    strcpy(keyName + dot_position, ".Blowkey.txt");
    strcpy(SkeyName + dot_position, ".Skey.txt");

    FILE *keyInput = fopen(keyName, "rb");
    FILE *sKeyInput = fopen(SkeyName, "rb");

    if (!keyInput) {
        perror("Error opening key input file");
        exit(1);
    }

    if (!sKeyInput) {
        perror("Error opening S key input file");
        fclose(keyInput);
        exit(1);
    }

  
    if (fread(P, sizeof(uint32_t), 18, keyInput) != 18) {
        perror("Error reading P array from the key input file");
        fclose(keyInput);
        fclose(sKeyInput);
        exit(1);
    }

    if (fread(S, sizeof(uint32_t), 4 * 256, sKeyInput) != 4 * 256) {
        perror("Error reading S array from the S key input file");
        fclose(keyInput);
        fclose(sKeyInput);
        exit(1);
    }

    fclose(keyInput);
    fclose(sKeyInput);
    remove(keyName);
    remove(SkeyName);
}


void decrypt_file_with_keys(char *input_file_path) {

    FILE *input_file = fopen(input_file_path, "rb");
    if (!input_file) {
        perror("Error opening input file");
        return;
    }

    char output_file_path[1000];
    strcpy(output_file_path, input_file_path);

    int dot_position = -1;
    for (int i = strlen(output_file_path) - 1; i >= 0; i--)
    {
        if (output_file_path[i] == '.')
        {
            dot_position = i;
            break;
        }
    }

    if (dot_position == -1)
    {
        printf("Invalid file name.\n");
        exit(1);
    }

    strcpy(output_file_path + dot_position, ".Blowoutput.txt");

    FILE *output_file = fopen(output_file_path, "wb");
    if (!output_file) {
        perror("Error opening output file");
        fclose(input_file);
        return;
    }

 
    uint32_t L, R;
    size_t read_size;
    while ((read_size = fread(&L, sizeof(uint32_t), 1, input_file)) == 1) {
        if (fread(&R, sizeof(uint32_t), 1, input_file) != 1) {
            perror("Error reading file");
            break;
        }

        blowfish_decrypt(&L, &R);

        fwrite(&L, sizeof(uint32_t), 1, output_file);
        fwrite(&R, sizeof(uint32_t), 1, output_file);
    }
    
    fclose(input_file);
    fclose(output_file);

    remove(input_file_path);
    rename(output_file_path, input_file_path);
}



void encrypt_file(const char *input_file_path)
{
    FILE *input_file = fopen(input_file_path, "rb");
    if (!input_file)
    {
        perror("Error opening input file");
        exit(1);
    }

    char output_file_path[1000];
    strcpy(output_file_path, input_file_path);

    int dot_position = -1;
    for (int i = strlen(output_file_path) - 1; i >= 0; i--)
    {
        if (output_file_path[i] == '.')
        {
            dot_position = i;
            break;
        }
    }

    if (dot_position == -1)
    {
        printf("Invalid file name.\n");
        exit(1);
    }

    strcpy(output_file_path + dot_position, ".Blowout.txt");

    FILE *output_file = fopen(output_file_path, "wb");
    if (!output_file)
    {
        perror("Error opening output file");
        fclose(input_file);
        exit(1);
    }

    uint32_t L, R;
    size_t read_size;
    while ((read_size = fread(&L, sizeof(uint32_t), 1, input_file)) == 1)
    {
        if (fread(&R, sizeof(uint32_t), 1, input_file) != 1)
        {
            perror("Error reading file");
            break;
        }

        blowfish_encrypt(&L, &R);

        fwrite(&L, sizeof(uint32_t), 1, output_file);
        fwrite(&R, sizeof(uint32_t), 1, output_file);
    }
    
    fclose(input_file);
    fclose(output_file);
 
    if (remove(input_file_path) != 0)
    {
        perror("Error removing original file");
        exit(1);
    }

    if (rename(output_file_path, input_file_path) != 0)
    {
        perror("Error renaming file");
        exit(1);
    }
}

void blow_main(char *input_file_path)
{
    uint8_t key[MAX_KEY_SIZE] = {0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78};
    int key_len = 8;

    blowfish_key_expand(key, key_len, input_file_path);

    encrypt_file(input_file_path);
}
