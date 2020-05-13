#include <sodium.h>
#include <assert.h>

int publish_via_bluetooth(const unsigned char *pk) {
  // This function should publish a public key via Bluetooth advertisement/message
  // Here, we instead simply print it to stdout for debugging (hex-encoded)
  int i;
  for (i=0; i<crypto_kx_PUBLICKEYBYTES; i++) {
    printf("%02X", pk[i]);
  }
  return 0;
}

int print_shared_secret(const unsigned char *x) {
  int i;
  for (i=0; i<crypto_kx_SESSIONKEYBYTES; i++) {
    printf("%02X", x[i]);
  }
  return 0;
}

int compare_bytes(const unsigned char *x, const unsigned char *y, const int length) {
  // Returns -1 if x < y
  //          0 if x == y
  //          1 if x > y
  int i;
  for (i=0; i<length; i++) {
    if (x[i] < y[i]) {
      return -1;
    } else if (x[i] > y[i]) {
      return 1;
    }
  }
  return 0;
}

int generate_contact_tokens(unsigned char rx[crypto_kx_SESSIONKEYBYTES],
                            unsigned char tx[crypto_kx_SESSIONKEYBYTES],
                            const unsigned char my_pk[crypto_kx_PUBLICKEYBYTES],
                            const unsigned char my_sk[crypto_kx_SECRETKEYBYTES],
                            const unsigned char other_pk[crypto_kx_PUBLICKEYBYTES])
{
  // This function either calls crypto_kx_server_session_keys or crypto_kx_client_session_keys,
  // depending on which whether we assign ourselves to be client or server

  // We will do this by comparing our public keys; whoever has the smaller
  // key will be the client, and whoever has the larger key will be the server
  int cmp;
  cmp = compare_bytes(my_pk, other_pk, crypto_kx_PUBLICKEYBYTES);
  if (cmp == 1) {
    // Then we're the server
    if (crypto_kx_server_session_keys(rx, tx, my_pk, my_sk, other_pk) != 0 ) {
      // Error handling for suspicious public key
      printf("Something went wrong with computing shared secret\n");
      exit(-1);
    }
  } else if (cmp == -1) {
    // Then we're the client
    if (crypto_kx_client_session_keys(rx, tx, my_pk, my_sk, other_pk) != 0 ) {
      // Error handling for suspicious public key
      printf("Something went wrong with computing shared secret\n");
      exit(-1);
    }
  } else if (cmp == 0) {
    printf("Somehow we have the same public key as the other party; this should ~never happen\n");
    exit(-2);
  } else {
    printf("Unknown error in cmp function\n");
    exit(-3);
  }

}

int main(void) {
  unsigned char alice_pk[crypto_kx_PUBLICKEYBYTES], alice_sk[crypto_kx_SECRETKEYBYTES];
  crypto_kx_keypair(alice_pk, alice_sk);  // Alice generates her public key and secret key
  printf("Alice's public key:\t");
  publish_via_bluetooth(alice_pk);
  printf("\n");
  
  unsigned char bob_pk[crypto_kx_PUBLICKEYBYTES], bob_sk[crypto_kx_SECRETKEYBYTES];
  crypto_kx_keypair(bob_pk, bob_sk);  // Bob generates his public key and secret key
  printf("Bob's public key:\t");
  publish_via_bluetooth(bob_pk);
  printf("\n");

  printf("\n");
  // Alice now knows Bob's public key and can derive contact tokens from it
  // The variables here are prefixed, since these are what Alice derives
  unsigned char a_bob2alice[crypto_kx_SESSIONKEYBYTES], a_alice2bob[crypto_kx_SESSIONKEYBYTES];
  generate_contact_tokens(a_bob2alice, a_alice2bob, alice_pk, alice_sk, bob_pk);
  printf("Alice's token for Bob2Alice:\t"); print_shared_secret(a_bob2alice); printf("\n");
  printf("Alice's token for Alice2Bob:\t"); print_shared_secret(a_alice2bob); printf("\n");

  // Bob also knows Alice's public key and can derive contact tokens from it
  unsigned char b_alice2bob[crypto_kx_SESSIONKEYBYTES], b_bob2alice[crypto_kx_SESSIONKEYBYTES];
  generate_contact_tokens(b_alice2bob, b_bob2alice, bob_pk, bob_sk, alice_pk);
  printf("Bob's token for Alice2Bob:\t"); print_shared_secret(b_alice2bob); printf("\n");
  printf("Bob's token for Bob2Alice:\t"); print_shared_secret(b_bob2alice); printf("\n");

  assert(compare_bytes(a_bob2alice, b_bob2alice, crypto_kx_SESSIONKEYBYTES)==0);
  assert(compare_bytes(a_alice2bob, b_alice2bob, crypto_kx_SESSIONKEYBYTES)==0);
  printf("\nAlice and Bob now have a pair of contact tokens that they can use to send messages to each other\n");
}
