/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubfm_Hash.c
 *
 * Description:
 *  Some functions are provided to support buffer manager.
 *  Each BfMHashKey is mapping to one table entry in a hash table(hTable),
 *  and each entry has an index which indicates a buffer in a buffer pool.
 *  An ordinary hashing method is used and linear probing strategy is
 *  used if collision has occurred.
 *
 * Exports:
 *  Four edubfm_LookUp(BfMHashKey *, Four)
 *  Four edubfm_Insert(BfMHaskKey *, Two, Four)
 *  Four edubfm_Delete(BfMHashKey *, Four)
 *  Four edubfm_DeleteAll(void)
 */


#include <stdlib.h> /* for malloc & free */
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"



/*@
 * macro definitions
 */  

/* Macro: BFM_HASH(k,type)
 * Description: return the hash value of the key given as a parameter
 * Parameters:
 *  BfMHashKey *k   : pointer to the key
 *  Four type       : buffer type
 * Returns: (Two) hash value
 */
#define BFM_HASH(k,type)	(((k)->volNo + (k)->pageNo) % HASHTABLESIZE(type))


/*@================================
 * edubfm_Insert()
 *================================*/
/*
 * Function: Four edubfm_Insert(BfMHashKey *, Two, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Insert a new entry into the hash table.
 *  If collision occurs, then use the linear probing method.
 *
 * Returns:
 *  error code
 *    eBADBUFINDEX_BFM - bad index value for buffer table
 */
Four edubfm_Insert(
    BfMHashKey 		*key,			/* IN a hash key in Buffer Manager */
    Two 		index,			/* IN an index used in the buffer pool */
    Four 		type)			/* IN buffer type */
{
    Four 		i;
    Two  		hashValue;
    CHECKKEY(key);    /*@ check validity of key */

    if( (index < 0) || (index > BI_NBUFS(type)) )
        ERR( eBADBUFINDEX_BFM );
    
    //key 값 이용하여 hash key value 계산하기
     hashValue = (key->pageNo + key->volNo) % HASHTABLESIZE(type);
    //해당 hash key value에 해당하는 hash table 자리에서 collision 발생하는지 살피기
    if(BI_HASHTABLEENTRY(type, hashValue) != -1){
        //발생하면 해당 buffer element의 nextHashEntry 변수에 array index를 저장(hash table entry)
        BI_NEXTHASHENTRY(type, index) = BI_HASHTABLEENTRY(type, hashValue);
        // BI_NEXTHASHENTRY(type, index) = 100;
        BI_HASHTABLEENTRY(type, hashValue) = index;
    }
    //collision 발생 안할 시새로운 array index를 삽입
    else{
        BI_HASHTABLEENTRY(type, hashValue) = index;
    }


    return( eNOERROR );

}  /* edubfm_Insert */



/*@================================
 * edubfm_Delete()
 *================================*/
/*
 * Function: Four edubfm_Delete(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the entry which corresponds to `key' and
 *  Delete the entry from the hash table.
 *
 * Returns:
 *  error code
 *    eNOTFOUND_BFM - The key isn't in the hash table.
 */
Four edubfm_Delete(
    BfMHashKey          *key,                   /* IN a hash key in buffer manager */
    Four                type )                  /* IN buffer type */
{
    Two                 i, prev;                
    Two                 hashValue;
    CHECKKEY(key);    /*@ check validity of key */
    //삭제할 hash key value 계산
    hashValue = (key->pageNo + key->volNo) % HASHTABLESIZE(type);
    //삭제하고자 하는 buffer element가 linked list 처음에 있는 경우
    //linked list head를 nextHashEntry에 있는 주소로 바꾸기
    if(EQUALKEY(&BI_KEY(type, BI_HASHTABLEENTRY(type, hashValue)), key)) {
        BI_HASHTABLEENTRY(type, hashValue) = BI_NEXTHASHENTRY(type, BI_HASHTABLEENTRY(type, hashValue));
    }
    //삭제하고자 하는 buffer element가 linked list 중간에 있는 경우
    else{
        i = hashValue;
        while(BI_NEXTHASHENTRY(type, i) != -1){
            printf("%d\n", i);
            prev = i;
            i = BI_NEXTHASHENTRY(type, i);
            if(EQUALKEY(&BI_KEY(type, i), key)){
                BI_NEXTHASHENTRY(type, prev) = BI_NEXTHASHENTRY(type, i);
            }
        }
    }
    return 0;
    

    ERR( eNOTFOUND_BFM );

}  /* edubfm_Delete */



/*@================================
 * edubfm_LookUp()
 *================================*/
/*
 * Function: Four edubfm_LookUp(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the given key in the hash table and return its
 *  corressponding index to the buffer table.
 *
 * Retruns:
 *  index on buffer table entry holding the train specified by 'key'
 *  (NOTFOUND_IN_HTABLE - The key don't exist in the hash table.)
 */
Four edubfm_LookUp(
    BfMHashKey          *key,                   /* IN a hash key in Buffer Manager */
    Four                type)                   /* IN buffer type */
{
    Two                 i, j;                   /* indices */
    Two                 hashValue;

    CHECKKEY(key);    /*@ check validity of key */

    hashValue = (key->pageNo + key->volNo) % HASHTABLESIZE(type);
    // for(i = 0; i <= HASHTABLESIZE(type); i++)
    // {
    //     if(i == hashValue)
    //     {
    //         return BI_HASHTABLEENTRY(type, hashValue);
    //     }
    // }
    return BI_HASHTABLEENTRY(type, hashValue);
    // return (NOTFOUND_IN_HTABLE);
    
}  /* edubfm_LookUp */



/*@================================
 * edubfm_DeleteAll()
 *================================*/
/*
 * Function: Four edubfm_DeleteAll(void)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Delete all hash entries.
 *
 * Returns:
 *  error code
 */
Four edubfm_DeleteAll(void)
{
    Two 	i;
    Four        tableSize;
    tableSize = HASHTABLESIZE(0);
    for(i = 0;i < tableSize; i++){
        BI_HASHTABLEENTRY(0,i) = -1;
    }
    tableSize = HASHTABLESIZE(1);
    for(i = 0;i < tableSize; i++){
        BI_HASHTABLEENTRY(1,i) = -1;
    }


    return(eNOERROR);

} /* edubfm_DeleteAll() */ 
