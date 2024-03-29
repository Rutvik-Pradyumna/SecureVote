#include <bits/stdc++.h>
#include <gmp.h>
using namespace std;

void generateKeys(mpz_t& e, mpz_t& d, mpz_t& n) 
{
    mpz_t p,q,phi_n;
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    mpz_inits(p,q,phi_n,NULL);

    // Generate two large random prime numbers
    mpz_urandomb(p,state,512);
    mpz_urandomb(q,state,512);
    mpz_nextprime(p,p);
    mpz_nextprime(q,q);

    // Calculate n = p * q
    mpz_mul(n,p,q);

    // Calculate phi(n) = (p-1)*(q-1)
    mpz_sub_ui(p,p,1);
    mpz_sub_ui(q,q,1);
    mpz_mul(phi_n,p,q);

    // Choose a random number e such that 1 < e < phi(n) and gcd(e, phi(n)) = 1
    mpz_urandomm(e,state,phi_n);
    while (mpz_cmp_ui(e,1)<=0||mpz_cmp(e,phi_n)>=0||mpz_gcd_ui(NULL,e,mpz_get_ui(phi_n))!=1) 
    {
        mpz_urandomm(e,state,phi_n);
    }
    // Calculate d = e^-1 mod phi(n)
    mpz_invert(d,e,phi_n);

    mpz_clears(p,q,phi_n,NULL);
    gmp_randclear(state);
}

void encryptRSA(const mpz_t& plaintext,const mpz_t& e,const mpz_t& n,mpz_t& ciphertext) 
{
    mpz_powm(ciphertext,plaintext,e,n);
}

// Function to decrypt a ciphertext
void decryptRSA(const mpz_t& ciphertext,const mpz_t& d,const mpz_t& n,mpz_t& plaintext) 
{
    mpz_powm(plaintext,ciphertext,d,n);
}
char intToChar(int i) 
{
    // Ensure i is within the range of printable characters
    i=i%95; // ASCII printable characters start from ' ' (32) and end at '~' (126)
    if(i<0)i+=95; // Adjust negative values
    return ' '+i;
}
int charToInt(char c)
{
    int i=c-' '; // Get the offset from the first printable ASCII character
    if(i<0||i>=95)i=0; // Ensure it's within the range of printable characters
    return i;
}

string StringencryptRSA(const string& plaintext,const mpz_t& e,const mpz_t& n) 
{
    // Convert plaintext to integer
    mpz_t num_mpz;
    mpz_init(num_mpz);
    mpz_import(num_mpz,plaintext.size(),1,sizeof(plaintext[0]),0,0,plaintext.c_str());

    // Encrypt the integer
    mpz_powm(num_mpz,num_mpz,e,n);

    // Export the encrypted integer as a string
    size_t size=mpz_sizeinbase(num_mpz,256);
    string ciphertext(size,'\0');
    mpz_export(&ciphertext[0],&size,1,sizeof(ciphertext[0]),0,0,num_mpz);

    mpz_clear(num_mpz);
    return ciphertext;
}

string StringdecryptRSA(const string& ciphertext,const mpz_t& d,const mpz_t& n) {
    // Import the ciphertext string as an integer
    mpz_t num_mpz;
    mpz_init(num_mpz);
    mpz_import(num_mpz,ciphertext.size(),1,sizeof(ciphertext[0]),0,0,ciphertext.c_str());

    // Decrypt the integer
    mpz_powm(num_mpz,num_mpz,d,n);

    // Export the decrypted integer as a string
    size_t size=mpz_sizeinbase(num_mpz, 256);
    std::string plaintext(size,'\0');
    mpz_export(&plaintext[0],&size,1,sizeof(plaintext[0]),0,0,num_mpz);

    mpz_clear(num_mpz);
    return plaintext;
}


std::string mpz_to_string(const mpz_t num) {
    // Determine the size of the buffer needed to hold the string representation
    size_t size = mpz_sizeinbase(num, 10) + 2; // Base 10 + sign + null terminator

    // Allocate memory for the buffer
    char* buffer = new char[size];

    // Convert the mpz_t variable to a string
    mpz_get_str(buffer, 10, num);

    // Create a string from the buffer
    std::string result(buffer);

    // Free the allocated memory
    delete[] buffer;

    return result;
}

void string_to_mpz(const std::string& str, mpz_t num) {
    mpz_set_str(num, str.c_str(), 10); // Assuming base 10
}