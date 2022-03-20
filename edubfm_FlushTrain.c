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
 * Module: edubfm_FlushTrain.c
 *
 * Description : 
 *  Write a train specified by 'trainId' into the disk.
 *
 * Exports:
 *  Four edubfm_FlushTrain(TrainID *, Four)
 */


#include "EduBfM_common.h"
#include "RDsM.h"
#include "RM.h"
#include "EduBfM_Internal.h"



/*@================================
 * edubfm_FlushTrain()
 *================================*/
/*
 * Function: Four edubfm_FlushTrain(TrainID*, Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Write a train specified by 'trainId' into the disk.
 *  Construct a hash key using the TrainID 'trainId'(actually same)
 *  in order to look up the buffer in the buffer pool. If it is successfully
 *  found, then force it out to the disk using RDsM, especially
 *  RDsM_WriteTrain().
 *
 * Returns:
 *  error code
 *    some errors caused by function calls
 */
Four edubfm_FlushTrain(
    TrainID 			*trainId,		/* IN train to be flushed */
    Four   			type)			/* IN buffer type */
{
    Four 			e;			/* for errors */
    Four 			index;			/* for an index */
    
	/* Error check whether using not supported functionality by EduBfM */
	if (RM_IS_ROLLBACK_REQUIRED()) ERR(eNOTSUPPORTED_EDUBFM);
    //lookup 함수 이용해서 buffer element의 array index 찾기
    index = edubfm_LookUp(trainId, type);
    if (index == NOTFOUND_IN_HTABLE) ERR(NOTFOUND_IN_HTABLE);
    //array index를 이용해서 찾은 buffer element의 dirty bit가 1인 경우 
    if(BI_BITS(type, index) & DIRTY != 0){
        //해당 page/train을 disk에 기록(RDsM_WriteTrain 이용)
        RDsM_WriteTrain(BI_BUFFER(type, index), trainId, BI_BUFSIZE(type));
        //printf("flush %d\n",index);
        //Dirty bit unset
        BI_BITS(type, index)--;
    }

	
    return( eNOERROR );

}  /* edubfm_FlushTrain */
