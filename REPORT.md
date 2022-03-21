# EduBfM Report

Name: Eunchae Kim

Student id: 20180297

# Problem Analysis

Disk와 BufferPool 사이의 page/train이동을 관리하는 BufferManager를 만들어야 한다. 


# Design For Problem Solving

## High Level
BufferManager는 GetTrain, FreeTrain, SerDirty, FlushAll, DiscardAll 이라는 API function들로 구현할 수 있다.


## Low Level

- EduBfM_GetTrain()

요청한 key에 해당하는 page/train을 bufferpool로 옮긴다. 

- EduBfM_FreeTrain()

요청한 key에 해당하는 page/train을 unfix한다.

- EduBfM_SetDirty()

요청한 key에 해당하는 page/train의 DIRTY bit를 1로 set한다.

- EduBfM_FlushAll()

Buffer Table의 모든 element들의 수정된 사항을 Disk에 기록한다.

- EduBfM_DiscardAll()

Buffer Table의 모든 element들의 내용을 삭제한다.


# Mapping Between Implementation And the Design

## API Functions

- EduBfM_GetTrain

주어진 page가 들어갈 buffer pool의 index를 구한다. 
해당 index가 -1일 경우 버퍼에 새로운 자리를 할당받고, 해시 테이블에도 새로운 버퍼 엘리먼트의 값을 적용한다. 디스크로부터 page의 내용을 읽어서 버퍼 풀에 쓴 다음, 새로 저장된 값이 있는 버퍼 풀의 포인터를 리턴한다.
index가 -1이 아닐 경우 버퍼에 존재하는 해당 page의 값들을 업데이트하고 그 page에 해당하는 버퍼 풀의 포인터를 리턴한다.

- EduBfM_FreeTrain

LookUp함수를 이용하여 주어진 train에 해당하는 index를 찾고, 해당 index의 FIXED 값이 1보다 큰 경우 1 줄인다.
FIXED 값이 0보다 작거나 같은 경우 에러 메시지를 출력한다.

- EduBfM_SetDirty

LookUp함수를 이용하여 주어진 train에 해당하는 buffer table element의 bits값에 DIRTY bit를 1로 설정한다.

- EduBfM_FlushAll

page, train buffer table의 모든 element에 대해 edubfm_FlushTrain을 호출한다.

- EduBfM_DiscardAll

page, train buffer table의 모든 element에 대해 pageNo에는 -1, volNo에는 1000, fixed, bits에는 0을 입력한다.

## Internal Functions

- edubfm_ReadTrain

RDsM_ReadTrain 함수를 호출한다.

- edubfm_AllocTrain

buffer table이 모두 fixed가 1인지 확인하여 모두 fixed가 1이라면 eNOUNFIXEDBBUF_BFM을 에러처리한다.
victim이 나올 때까지 buffer table을 돌며 REFER bit를 검사한다. 이 과정을 통해 victim을 찾는다.
victim인 buffer element이 DIRTY일 경우 FlushTrain한다.
victim인 buffer element에 해당하는 값을 hash table에서 삭제한다.
buffer table의 victim element를 초기화한다.

- edubfm_Insert

주어진 key 값에 hash 함수를 적용하여 hash key value를 구한다.
hash key value에 해당하는 hash table entry를 살펴보고 기존에 그 자리에 존재하는 index가 있는지 조사한다. 
index가 없다면 해당 hash table entry에 index를 입력한다.
index가 있다면 해당 buffer table element의 NextHashValue로 이어지는 linked list의 마지막에 index를 입력한다.

- edubfm_Delete

주어진 key 값에 hash 함수를 적용하여 hash key value를 구한다.
해당 hash key value에 해당하는 hash table entry를 살펴보고 그 index가 buffer table에서 가리키는 값의 key와 주어진 key 값을  비교한다.
만약 같다면 linked list에서 head를 가리키는 포인터를 그 다음 element를 가리키도록 한다.
다르다면 linked list를 순회하면서 주어진 key값과 같은 key값을 가지는 buffer table element를 찾아 그 index를 i라고 하자. linked list상에서 i의 이전에 있는 element의 NextHashValue가 linked list상에서 i의 다음 element를 가리키도록 한다.

- edubfm_LookUp

key값에 hash 함수를 적용하여 hash key value를 계산하고, hash table에서 해당 값을 hash value로 하는 hash table entry 값을 return한다.

- edubfm_FlushTrain

LookUP 함수를 이용해서 key값에 해당하는 buffer table element를 찾고, 그 element의 DIRTY bit가 1이면 RDsM_WriteTrain을 실행시킨다. 이후 BITS의 DIRTY Bit를 삭제한다.