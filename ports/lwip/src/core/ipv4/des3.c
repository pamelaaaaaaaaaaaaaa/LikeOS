/* des3.c - PaulOS embedded operating system
   Copyright (C) 2002, 2003  Paul Sheer

   Rights to copy and modify this program are restricted to strict copyright
   terms. These terms can be found in the file LICENSE distributed with this
   software.

   This software is provided "AS IS" and WITHOUT WARRANTY, either express or
   implied, including, without limitation, the warranties of NON-INFRINGEMENT,
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO
   THE QUALITY OF THIS SOFTWARE IS WITH YOU. Under no circumstances and under
   no legal theory, whether in tort (including negligence), contract, or
   otherwise, shall the copyright owners be liable for any direct, indirect,
   special, incidental, or consequential damages of any character arising as a
   result of the use of this software including, without limitation, damages
   for loss of goodwill, work stoppage, computer failure or malfunction, or any
   and all other commercial damages or losses. This limitation of liability
   shall not apply to liability for death or personal injury resulting from
   copyright owners' negligence to the extent applicable law prohibits such
   limitation. Some jurisdictions do not allow the exclusion or limitation of
   incidental or consequential damages, so this exclusion and limitation may
   not apply to You.
*/

#ifdef HAVE_IPSEC

#ifndef HAVE_BLOWFISH
#error HAVE_BLOWFISH must be set for IPSEC with Blowfish+MD5 encryption
#endif

#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/ipsec.h"
#include "netif/validate.h"

#include <types.h>
#include <assert.h>
#include <des.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

struct des3_arg {
    des_key_schedule ks1, ks2, ks3;
};

void *des3_init (u8_t * key, int key_len, u8_t * block_size)
{
    struct des3_arg *des3_arg;
    des_cblock des_key;
    des3_arg = mem_malloc (sizeof (struct des3_arg));

    *block_size = 8;

    memset (&des_key, '\0', 8);
    memcpy (&des_key, key, min (key_len, 8));
    des_set_odd_parity (&des_key);
    des_set_key (&des_key, des3_arg->ks1);

    memset (&des_key, '\0', 8);
    if (key_len > 8)
	memcpy (&des_key, key + 8, min (key_len - 8, 8));
    des_set_odd_parity (&des_key);
    des_set_key (&des_key, des3_arg->ks2);

    memset (&des_key, '\0', 8);
    if (key_len > 16)
	memcpy (&des_key, key + 16, min (key_len - 16, 16));
    else
	memcpy (&des_key, key, min (key_len, 8));
    des_set_odd_parity (&des_key);
    des_set_key (&des_key, des3_arg->ks2);

    return (void *) des3_arg;
}

void des3_done (void *arg)
{
    assert (arg);
    mem_free (arg);
}

void des3_decrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct des3_arg *des3_arg = (struct des3_arg *) alg_arg;
    des_ede3_cbc_encrypt (data, data, data_len, des3_arg->ks1, des3_arg->ks2, des3_arg->ks3,
			  (des_cblock *) iv, 0);
}

void des3_encrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct des3_arg *des3_arg = (struct des3_arg *) alg_arg;
    des_ede3_cbc_encrypt (data, data, data_len, des3_arg->ks1, des3_arg->ks2, des3_arg->ks3,
			  (des_cblock *) iv, 1);
}

const struct crypt_alg des3_alg = { des3_init, des3_done, des3_decrypt, des3_encrypt };

#endif				/* HAVE_IPSEC */