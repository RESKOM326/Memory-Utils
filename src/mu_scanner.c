/**
 * @file mu_scanner.c
 * @author Mark Dervishaj
 * @brief Implementation of mu_scanner.h
 * @version 0.1
 * @date 2022-08-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_scanner.h"
#include "inc/mu_memchunk.h"
#include "inc/mu_io.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  /* _GNU_SOURCE */

#define ALPHABET_LEN 256
#define max(a, b) ((a < b) ? b : a)

/* BAD CHARACTER RULE.
delta1 table: delta1[c] contains the distance between the last
character of pat and the rightmost occurrence of c in pat.

If c does not occur in pat, then delta1[c] = patlen.
If c is at string[i] and c != pat[patlen-1], we can safely shift i
  over by delta1[c], which is the minimum distance needed to shift
  pat forward to get string[i] lined up with some character in pat.
c == pat[patlen-1] returning zero is only a concern for BMH, which
  does not have delta2. BMH makes the value patlen in such a case.
  We follow this choice instead of the original 0 because it skips
  more. (correctness?)

This algorithm runs in alphabet_len+patlen time. */

static void make_delta1(INT64 *delta1, UCHAR *pat, ULONG patlen)
{
    for(int i=0; i < ALPHABET_LEN; i++)
    {
        delta1[i] = patlen;
    }
    for(int i=0; (ULONG) i < patlen; i++)
    {
        delta1[pat[i]] = patlen-1 - i;
    }
}

/* true if the suffix of word starting from word[pos] is a prefix of word */
static bool is_prefix(UCHAR *word, ULONG wordlen, INT64 pos)
{
    INT suffixlen = wordlen - pos;
    /* could also use the strncmp() library function here
    return ! strncmp(word, &word[pos], suffixlen); */
    for(INT i = 0; i < suffixlen; i++)
    {
        if (word[i] != word[pos+i])
        {
            return false;
        }
    }
    return true;
}

/* length of the longest suffix of word ending on word[pos]. suffix_length("dddbcabc", 8, 4) = 2 */
static ULONG suffix_length(UCHAR *word, ULONG wordlen, INT64 pos)
{
    ULONG i;
    /* increment suffix length i to the first mismatch or beginning of the word */
    for(i = 0; (word[pos-i] == word[wordlen-1-i]) && (i <= (ULONG) pos); i++);
    return i;
}

/* GOOD SUFFIX RULE.
delta2 table: given a mismatch at pat[pos], we want to align
with the next possible full match could be based on what we
know about pat[pos+1] to pat[patlen-1].

In case 1:
pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
the next plausible match starts at or after the mismatch.
If, within the substring pat[pos+1 .. patlen-1], lies a prefix
of pat, the next plausible match is here (if there are multiple
prefixes in the substring, pick the longest). Otherwise, the
next plausible match starts past the character aligned with
pat[patlen-1].

In case 2:
pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
mismatch tells us that we are not looking at the end of a match.
We may, however, be looking at the middle of a match.

The first loop, which takes care of case 1, is analogous to
the KMP table, adapted for a 'backwards' scan order with the
additional restriction that the substrings it considers as
potential prefixes are all suffixes. In the worst case scenario
pat consists of the same letter repeated, so every suffix is
a prefix. This loop alone is not sufficient, however:
Suppose that pat is "ABYXCDBYX", and text is ".....ABYXCDEYX".
We will match X, Y, and find B != E. There is no prefix of pat
in the suffix "YX", so the first loop tells us to skip forward
by 9 characters.
Although superficially similar to the KMP table, the KMP table
relies on information about the beginning of the partial match
that the BM algorithm does not have.

The second loop addresses case 2. Since suffix_length may not be
unique, we want to take the minimum value, which will tell us
how far away the closest potential match is. */

static void make_delta2(INT64 *delta2, UCHAR *pat, ULONG patlen)
{
    INT64 p;
    ULONG last_prefix_index = 1;

    /* first loop */
    for(p=patlen-1; p>=0; p--)
    {
        if(is_prefix(pat, patlen, p+1))
        {
            last_prefix_index = p+1;
        }
        delta2[p] = last_prefix_index + (patlen-1 - p);
    }

    /* second loop */
    for(p=0; (ULONG) p < patlen-1; p++)
    {
        ULONG slen = suffix_length(pat, patlen, p);
        if(pat[p - slen] != pat[patlen-1 - slen])
        {
            delta2[patlen-1 - slen] = patlen-1 - p + slen;
        }
    }
}

/* Returns pointer to first match. See also glibc memmem() (non-BM) and std::boyer_moore_searcher (first-match). */
static UCHAR* boyer_moore (UCHAR *string, ULONG stringlen, UCHAR *pat, ULONG patlen)
{
    INT64 delta1[ALPHABET_LEN];
    INT64 delta2[patlen];
    make_delta1(delta1, pat, patlen);
    make_delta2(delta2, pat, patlen);

    /* The empty pattern must be considered specially */
    if(patlen == 0)
    {
        return string;
    }

    ULONG i = patlen - 1;           /* str-idx */
    while(i < stringlen)
    {
        INT64 j = patlen - 1;       /* pat-idx */
        while(j >= 0 && (string[i] == pat[j]))
        {
            --i;
            --j;
        }
        if(j < 0)
        {
            return &string[i+1];
        }

        INT64 shift = max(delta1[string[i]], delta2[j]);
        i += shift;
    }
    return NULL;
}

ULONG* execute_scanner_BM(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    MU_MEM_CHUNK *modif_chunks = get_memory_chunks(target, 1, &size);
    MU_MEM_CHUNK *filtered = filter_memory_chunks(target, modif_chunks, &size);

    for(INT i = 0; i < size; i++)
    {
        MU_MEM_CHUNK chnk = filtered[i];
        UCHAR *bytes = read_chunk_data(target, chnk);
        UCHAR *ptr = NULL;
        ptr = boyer_moore(bytes, chnk.chunk_size, data, data_size);

        while(ptr)
        {
            ULONG offset = ptr - bytes;
            ULONG match = chnk.addr_start + offset;
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                exit(1);
            }
            matches[nmatches] = match;
            nmatches++;
            ptr = boyer_moore(&bytes[offset + data_size], (chnk.chunk_size - offset - data_size), data, data_size);
        }
        free(chnk.chunk_name);
        free(bytes);
    }
    *n_matches = nmatches;
    free(filtered);

    return matches;
}

ULONG* execute_scanner_STD(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    MU_MEM_CHUNK *modif_chunks = get_memory_chunks(target, 1, &size);
    MU_MEM_CHUNK *filtered = filter_memory_chunks(target, modif_chunks, &size);

    for(INT i = 0; i < size; i++)
    {
        MU_MEM_CHUNK chnk = filtered[i];
        UCHAR *bytes = read_chunk_data(target, chnk);
        UCHAR *ptr = NULL;
        ptr = memmem(bytes, chnk.chunk_size, data, data_size);

        while(ptr)
        {
            ULONG offset = ptr - bytes;
            ULONG match = chnk.addr_start + offset;
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                exit(1);
            }
            matches[nmatches] = match;
            nmatches++;
            ptr = memmem(&bytes[offset + data_size], (chnk.chunk_size - offset - data_size), data, data_size);
        }
        free(chnk.chunk_name);
        free(bytes);
    }
    *n_matches = nmatches;
    free(filtered);

    return matches;
}

ULONG* execute_scanner_STD_MTH(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    MU_MEM_CHUNK *modif_chunks = get_memory_chunks(target, 1, &size);
    MU_MEM_CHUNK *filtered = filter_memory_chunks(target, modif_chunks, &size);

    for(INT i = 0; i < size; i++)
    {
        MU_MEM_CHUNK chnk = filtered[i];
        UCHAR *bytes = read_chunk_data(target, chnk);
        UCHAR *ptr = NULL;
        ptr = memmem(bytes, chnk.chunk_size, data, data_size);

        while(ptr)
        {
            ULONG offset = ptr - bytes;
            ULONG match = chnk.addr_start + offset;
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                exit(1);
            }
            matches[nmatches] = match;
            nmatches++;
            ptr = memmem(&bytes[offset + data_size], (chnk.chunk_size - offset - data_size), data, data_size);
        }
        free(chnk.chunk_name);
        free(bytes);
    }
    *n_matches = nmatches;
    free(filtered);

    return matches;
}

MU_ERROR execute_filtering(PID target, ULONG **addresses, UCHAR *data, INT data_size, INT *n_matches)
{
    INT nmatches = 0;
    INT og_n_matches = *n_matches;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    INT i = 0;
    while(i < og_n_matches)
    {
        struct iovec local[1];
        struct iovec remote[1];
        UCHAR read[data_size];

        local[0].iov_base = read;
        local[0].iov_len = data_size;
        remote[0].iov_base = (void *) (*matches)[i];
        remote[0].iov_len = data_size;

        INT64 n_read = process_vm_readv(target, local, 1, remote, 1, 0);

        if(n_read < 0 || (ULONG) n_read != data_size)
        {
            exit(1);
        }
        int cmp = memcmp(read, data, data_size);
        if(cmp == 0)
        {
            ULONG match = (*matches)[i];
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                exit(1);
            }
            matches[nmatches] = match;
            nmatches++;
        }
        i++;
    }

    *n_matches = nmatches;

    free(*matches);
 
    *addresses = malloc((sizeof *matches)*(nmatches + 1));
    memcpy(*matches, matches, (sizeof *matches)*(nmatches + 1));
    free(matches);

    return ERR_OK;
}