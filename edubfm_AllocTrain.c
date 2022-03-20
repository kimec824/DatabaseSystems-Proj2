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
 * Module: edubfm_AllocTrain.c
 *
 * Description : 
 *  Allocate a new buffer from the buffer pool.
 *
 * Exports:
 *  Four edubfm_AllocTrain(Four)
 */


#include <errno.h>
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"


extern CfgParams_T sm_cfgParams;

/*@================================
 * edubfm_AllocTrain()
 *================================*/
/*
 * Function: Four edubfm_AllocTrain(Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Allocate a new buffer from the buffer pool.
 *  The used buffer pool is specified by the parameter 'type'.
 *  This routine uses the second chance buffer replacement algorithm
 *  to select a victim.  That is, if the reference bit of current checking
 *  entry (indicated by BI_NEXTVICTIM(type), macro for
 *  bufInfo[type].nextVictim) is set, then simply clear
 *  the bit for the second chance and proceed to the next entry, otherwise
 *  the current buffer indicated by BI_NEXTVICTIM(type) is selected to be
 *  returned.
 *  Before return the buffer, if the dirty bit of the victim is set, it 
 *  must be force out to the disk.
 *
 * Returns;
 *  1) An index of a new buffer from the buffer pool
 *  2) Error codes: Negative value means error code.
 *     eNOUNFIXEDBUF_BFM - There is no unfixed buffer.
 *     some errors caused by fuction calls
 */
Four edubfm_AllocTrain(
    Four 	type)			/* IN type of buffer (PAGE or TRAIN) */
{
    Four 	e;			/* for error */
    Four 	victim = -1;			/* return value */
    Four 	i, index;
    Four    find_unfixed = 0;
    
	/* Error check whether using not supported functionality by EduBfM */
	if(sm_cfgParams.useBulkFlush) ERR(eNOTSUPPORTED_EDUBFM);

    //모두 fixed인지 확인
    for(i = 0; i < BI_NBUFS(type); i++){
        if(BI_FIXED(type, i) == 0)
            find_unfixed++;
    }
    if(find_unfixed == 0) ERR(eNOUNFIXEDBUF_BFM);
    while(victim == -1){//pageNo가 -1인 page가 없었을 경우. REFER bit로 victim 찾기
        if(!BI_FIXED(type, BI_NEXTVICTIM(type))){
            if((BI_BITS(type, BI_NEXTVICTIM(type)) & REFER) != 0){ //REFER bit가 1인 경우
                BI_BITS(type, BI_NEXTVICTIM(type)) ^= REFER;
            }
            else{ //REFER bit가 0인 경우
                victim = BI_NEXTVICTIM(type);
            }
        }
        BI_NEXTVICTIM(type) = (BI_NEXTVICTIM(type)+1) % BI_NBUFS(type);
    }
    //수정되었는지
    //해당 buffer element에 저장되어 있는 페이지/트레인이 수정 되었는가? -> 내용을 disk로 flush
    if(BI_BITS(type, victim) & DIRTY != 0) edubfm_FlushTrain(&BI_KEY(type, victim), type);

    //해당 buffer element의 array index(hashTable entry)를 hashTable에서 삭제.
    edubfm_Delete(&BI_KEY(type, victim), type);
    
    //해당 buffer element에 대응하는 buf Table element 찾은 다음, 초기화
    //Question) 어떻게 초기화?
    BfMHashKey temp;
    temp.pageNo = -1;
    temp.volNo = 1000;
    BI_KEY(type, victim) = temp;
    BI_FIXED(type, victim) = 0;
    BI_BITS(type, victim) = 0;
    BI_NEXTHASHENTRY(type, victim) = -1;
    
    //해당 buffer element의 array index 반환
    return victim;
    
}  /* edubfm_AllocTrain */
