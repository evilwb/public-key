#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

// Source https://www.geeksforgeeks.org/modular-exponentiation-recursive/
uint power(uint base, uint exp, uint mod)
{
    if(base == 0)
        return 0;
    if(exp == 0)
        return 1;

    u_int64_t y;
    if(exp % 2 == 0){
        y = power(base, exp / 2, mod);
        y = (y * y) % mod;
    }

    else{
        y = base % mod;
        y = (y * power(base, exp - 1, mod) % mod) % mod;
    }

    return (int)((y + mod) % mod);
}

// Source https://www.geeksforgeeks.org/multiply-large-integers-under-large-modulo/
uint mult(u_int64_t l, u_int64_t r, uint mod)
{
    u_int64_t res = 0;

    l %= mod;

    while(r){
        if(r & 1)
            res = (res + l) % mod;

        l = (2 * l) % mod;
        r >>= 1;
    }
    return (uint)res;
}


int miller_rabin(uint p, uint s)
{
    uint u = 0;
    uint r = p-1;
    uint z, a;

    while(r % 2 == 0){
        r /= 2;
        u++;
    }

    for(int i = 1; i <= s; i++){
        a = (rand() % (p-4)) + 2;
        z = power(a, r, p);

        if(z != 1 && z != p-1){
            for(int j = 1; j < u; j++){
                z = (z*z) % p;
                if(z == 1)
                    return 0;   // composite
            }

            if(z != p-1)      
                return 0;       // composite
        }
    }
    return 1;                   // likely prime
}

void keygen()
{
    int seed;
    uint p;
    uint q;
    uint d;
    int good_prime;
    FILE * pubkey_file;
    FILE * prikey_file;

    printf("Enter a number for the seed: ");
    scanf("%d", &seed);
    srand(seed);

    do{
        good_prime = 0;
        q = rand();

        // make sure p is odd by setting the lowest bit to 1
        q = p | 1;              

        // make sure p 31 bit by setting highest bits
        q = q & 0x7FFFFFFF;     // set bit 32 to 0
        q = q | 0x40000000;     // set bit 31 to 1

        if(miller_rabin(q, 10) == 1 && q % 13 == 5)
            good_prime == 1;

        p = 2 * q + 1;
        good_prime = miller_rabin(p, 10);

    }while(!good_prime);

    d = rand() % (p-2) + 1;

    pubkey_file = fopen("pubkey.txt", "w");
    prikey_file = fopen("prikey.txt", "w");

    printf("Pubkey: %u %d %u\n writing to pubkey.txt...\n", p, 2, power(2, d, p));
    fprintf(pubkey_file, "%u %d %u", p, 2, power(2, d, p));

    printf("Prikey: %u %d %u\n writing to prikey.txt...\n", p, 2, d);
    fprintf(prikey_file, "%u %d %u", p, 2, d);

    fclose(pubkey_file);
    fclose(prikey_file);

}

void encrypt()
{
    FILE * pubkey_file, * ptext_file, * ctext_file;
    uint p, g, e, k, c1, c2, m;
    char currchar;

    pubkey_file = fopen("pubkey.txt", "r");
    fscanf(pubkey_file, "%u %u %u", &p, &g, &e);
    fclose(pubkey_file);

    ptext_file = fopen("ptext.txt", "r");
    ctext_file = fopen("ctext.txt", "w");

    while(feof(ptext_file) == 0){
        m = 0;
        for(int i = 0; i < 4; i++){
            currchar = fgetc(ptext_file);
            if(currchar != EOF)
                m |= currchar;
            if(i < 3)
                m <<= 8;
        }
        
        //todo check if m < p
        
        k = rand() % p;
        c1 = power(g, k, p);
        c2 = mult(power(e, k, p), m, p);

        printf("m: %u, C1: %u, C2: %u\n", m, c1, c2);
        fprintf(ctext_file, "%u %u ", c1, c2);
    }

    fclose(ptext_file);
    fclose(ctext_file);
}

void decrypt()
{
    FILE * prikey_file, * ctext_file, * ptext_file;
    uint p, d, c1, c2, m;
    char text_block[5];
    
    prikey_file = fopen("prikey.txt", "r");
    fscanf(prikey_file, "%u %u %u", &p, &d, &d);
    fclose(prikey_file);

    ctext_file = fopen("ctext.txt", "r");
    ptext_file = fopen("ptext.txt", "w");

    while(fscanf(ctext_file, "%u %u ", &c1, &c2) != EOF){
        m = mult(power(c1, p-1-d, p), c2, p);
        printf("m: %u\n", m);

        for(int i = 3; i >= 0; i--){
            text_block[i] = m & 255;
            m >>= 8;
        }
        text_block[4] = '\0';

        printf("Text block: %s\n", text_block);
        fprintf(ptext_file, "%s", text_block);
    }

    fclose(ctext_file);
    fclose(ptext_file);
}


int main(int argc, char** argv)
{
    int option;
    printf("Choose an option:\n(1)Generate Key\n(2)Encrypt\n(3)Decrypt\n: ");
    scanf("%d", &option);

    if(option == 1)
        keygen();
    else if(option == 2)
        encrypt();
    else if(option == 3)
        decrypt();
    else
        printf("%d is an invalid option\n", option);
}
