#include <iostream>
#include "RSA.hh"
using namespace std;
int main() 
{
    mpz_t e,d,n;
    mpz_inits(e,d,n,NULL);

    generateKeys(e,d,n);
    cout<<"private key"<<endl;
    gmp_printf("%Zd\n",e);
    cout<<"public key"<<endl;
    gmp_printf("%Zd\n",d);

    string plaintext="ab:2:cd:de:fgh:ij:1";

    // Encrypt the message
    string ciphertext=StringencryptRSA(plaintext,e,n);

    cout<<"Plaintext: "<<plaintext<<endl;
    cout<<"Ciphertext: "<<ciphertext<<endl;

    // Decrypt the ciphertext
    string decryptedtext=StringdecryptRSA(ciphertext,d,n);
    cout<<"Decrypted text: "<<decryptedtext<<endl;

    // Clean up
    mpz_clears(e,d,n,NULL);

    return 0;
    //mpz_set_ui(plaintext,123456);
    //encryptRSA(plaintext,e,n,ciphertext);
    //decryptRSA(ciphertext,d,n,decryptedtext);
}