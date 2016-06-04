
#include <stdlib.h>
#include "dir_svc_client.h"

void KADirectoryNodeFree(KADirectoryNode *instance)
{
	if (instance->_name) 
		free(instance->_name);

    if (instance->_dataBufferPtr)
		dsDataBufferDeAllocate(instance->_directoryRef, instance->_dataBufferPtr);

    if (instance->_nodeRef)
        dsCloseDirNode(instance->_nodeRef);
		
	free(instance);
}

KADirectoryNode *KADirectoryNodeCreate(tDirReference directoryRef, tDataListPtr nodePtr)
{
	KADirectoryNode *instance = calloc(1, sizeof(KADirectoryNode));
	if (!instance)
		return NULL;

	do {
		instance->_directoryRef = directoryRef;

		tDirStatus status = dsOpenDirNode(instance->_directoryRef, nodePtr, &instance->_nodeRef);
		if (status != eDSNoErr)
			break;
			
		instance->_dataBufferPtr = dsDataBufferAllocate(instance->_directoryRef, 2048);
		if (!instance->_dataBufferPtr)
			break;
		
		instance->_name = dsGetPathFromList(instance->_directoryRef, nodePtr, "/");
		if (!instance->_name)
			break;
			
		return instance;
	}
	while (false);

	KADirectoryNodeFree(instance);
	return NULL;
}

KADirectoryNode *KADirectoryNodeCreateFromUserRecord(tDirReference directoryRef, CFDictionaryRef user_record)
{
	KADirectoryNode *auth_node = NULL;
	const void *metanode_name = NULL;
	
	if (!user_record)
		return NULL;
	
	do
	{
		if (!CFDictionaryGetValueIfPresent(user_record, CFSTR(kDSNAttrMetaNodeLocation), &metanode_name) || !metanode_name)
			break;

		if (CFStringGetTypeID() != CFGetTypeID(metanode_name))
			break;
		
		char metanode_name_cstring[1024];
		if (!CFStringGetCString((CFStringRef)metanode_name, metanode_name_cstring, sizeof(metanode_name_cstring), kCFStringEncodingUTF8))
			break;

		tDataListPtr metanode_name_list_ptr = dsBuildFromPath(directoryRef, metanode_name_cstring, "/");
		
		if (!metanode_name_list_ptr)
			break;

		auth_node = KADirectoryNodeCreate(directoryRef, metanode_name_list_ptr);

		dsDataListDeallocate(directoryRef, metanode_name_list_ptr);
	} 
	while (false);
	
	return auth_node;
}

// double the data buffer we're using with this node as long as we're no allocating 1MB
// return true on success 
inline tDataBufferPtr double_databuffer_or_bail(tDirReference directoryRef, tDataBufferPtr dataBufferPtr)
{
	unsigned long newBufferSize = 1024;
	if (dataBufferPtr)
	{
		newBufferSize = dataBufferPtr->fBufferSize * 2;
		dsDataBufferDeAllocate(directoryRef, dataBufferPtr);
		if (newBufferSize > 1024*1024)
			return NULL;
	}
	return dsDataBufferAllocate(directoryRef, newBufferSize);
}

CFMutableDictionaryRef get_record_attributes(KADirectoryNode *node, tRecordEntryPtr record_entry_ptr, tAttributeListRef attr_list_ref)
{
	unsigned long rec_attr_count = record_entry_ptr->fRecordAttributeCount, i, j;
	
	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, rec_attr_count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!attributes)
		return NULL;
	
    for (i=1; i<= rec_attr_count; i++)
    {
        tAttributeValueListRef attr_value_list_ref;
        tAttributeEntryPtr attr_info_ptr;
        dsGetAttributeEntry( node->_nodeRef, node->_dataBufferPtr, attr_list_ref, i,
                             &attr_value_list_ref, &attr_info_ptr );
        unsigned long attr_value_count = attr_info_ptr->fAttributeValueCount;
		
		CFStringRef key = CFStringCreateWithBytes(kCFAllocatorDefault, 
								(const UInt8 *)attr_info_ptr->fAttributeSignature.fBufferData, 
								attr_info_ptr->fAttributeSignature.fBufferLength,
								kCFStringEncodingUTF8,
								false);
		
		CFMutableArrayRef values = NULL;
		if (attr_value_count > 1)
		{
			values = CFArrayCreateMutable(kCFAllocatorDefault, attr_value_count, &kCFTypeArrayCallBacks);
			if (!values)
				return NULL;
		}
		
		CFStringRef value = NULL;
		for (j=1; j <= attr_value_count; j++)
		{
			tAttributeValueEntryPtr value_entry_ptr;
			if (!dsGetAttributeValue( node->_nodeRef, node->_dataBufferPtr, j, attr_value_list_ref, &value_entry_ptr))
			{
				value = CFStringCreateWithBytes(kCFAllocatorDefault, 
							(const UInt8 *)value_entry_ptr->fAttributeValueData.fBufferData, 
							value_entry_ptr->fAttributeValueData.fBufferLength,
							kCFStringEncodingUTF8,
							false);
				if (value)
				{
					if (attr_value_count > 1)
					{
						CFArrayAppendValue(values, value);
						CFRelease(value);
					}
				}
			}
		}

		if (attr_value_count > 1)
		{
			CFDictionaryAddValue(attributes, key, values);
			CFRelease(values);
		} else {
			CFDictionaryAddValue(attributes, key, value);
			CFRelease(value);
		}
		
		CFRelease(key);

        dsDeallocAttributeEntry(node->_directoryRef, attr_info_ptr);
        dsCloseAttributeValueList(attr_value_list_ref);
    }

	return attributes;
}

CFMutableDictionaryRef get_record_at_index(KADirectoryNode *node, long unsigned index)
{
    tRecordEntryPtr record_entry_ptr = NULL; 
    tAttributeListRef attr_list_ref;
	CFMutableDictionaryRef attributes = NULL;
	
	assert(index > 0); // indexes start with 1
	
	if (dsGetRecordEntry(node->_nodeRef, node->_dataBufferPtr,
		index, // start count at 1
		&attr_list_ref, &record_entry_ptr) == eDSNoErr)
	{
		attributes = get_record_attributes(node, record_entry_ptr, attr_list_ref);
		dsCloseAttributeList(attr_list_ref);
		dsDeallocRecordEntry(node->_directoryRef, record_entry_ptr);
	}

	return attributes;
}

// Only returning the first node (used for finding eDSSearchNodeName)
tDirStatus KADirectoryClientGetNode(tDirReference directoryRef, tDirPatternMatch pattern, KADirectoryNode **node)
{
    tDirStatus status = eDSNoErr;	
    unsigned long node_count = 0;
    tContextData continuation_data_ptr = NULL;
	tDataBufferPtr _dataBufferPtr = dsDataBufferAllocate(directoryRef, 1024);
	
	for (;;)
	{
		status = dsFindDirNodes(directoryRef, _dataBufferPtr, NULL, pattern, &node_count, &continuation_data_ptr);

		if ((status != eDSBufferTooSmall) || !(_dataBufferPtr = double_databuffer_or_bail(directoryRef,_dataBufferPtr)))
			break;
	}

    if (status == eDSNoErr)
	{
		tDataListPtr node_name_ptr = NULL;

		if (continuation_data_ptr)
			dsReleaseContinueData(directoryRef, continuation_data_ptr);

		// eDSNoNodeFound would've been returned already
		assert( node_count > 0 );
		
		status = dsGetDirNodeName(directoryRef, _dataBufferPtr,
								  1, // get first node
								  &node_name_ptr);

		if (status == eDSNoErr)
		{
			if (node)
				*node = KADirectoryNodeCreate(directoryRef, node_name_ptr);
			dsDataListDeallocate(directoryRef, node_name_ptr);
		}
	}
	
    if (_dataBufferPtr)
		dsDataBufferDeAllocate(directoryRef, _dataBufferPtr);

	return status;
}


CFArrayRef find_records_by_attribute_value_internal(KADirectoryNode *node, tDataListPtr recordTypeDataList, tDataNodePtr attributeTypeDataNode, tDirPatternMatch matchType, tDataNodePtr attributeValueDataNode, tDataListPtr returnAttributesDataList)
{  
    tDirStatus status = 0;
    long unsigned record_count = 0; // This is an input variable too, consider just retrieving 1 record
    tContextData continuation_data_ptr = NULL;
    CFMutableArrayRef results = NULL;

	do {
		for (;;) 
		{
			status = dsDoAttributeValueSearchWithData(
				node->_nodeRef, node->_dataBufferPtr,
				recordTypeDataList,
				attributeTypeDataNode, 
				matchType,
				attributeValueDataNode,
				returnAttributesDataList,	/* (NULL is documented to be legal but that's a documentation bug) */
				false,			/* attr info and data */
				&record_count,
				&continuation_data_ptr);
			
			// Of all errors we only try to recover from eDSBufferTooSmall here
			if ((status != eDSBufferTooSmall) || !(node->_dataBufferPtr = double_databuffer_or_bail(node->_directoryRef, node->_dataBufferPtr)))
			break;
		} 

		// other problems are not recoverable
        if ( status )
			break;

		// there are continuations that have _no_ records
			
        if (record_count > 0) 
        {
			long unsigned i;

			if (!results)
				results = CFArrayCreateMutable(kCFAllocatorDefault, record_count, &kCFTypeArrayCallBacks);

			for (i=1; i <= record_count; i++)
			{
				CFMutableDictionaryRef record = get_record_at_index(node, i);  // KADirectoryRecordGet(const char **name, CFArrayRef *attrs);
				if (record)
				{
					CFArrayAppendValue(results, record);
					CFRelease(record);
				}
			}
        }
        
    } while (continuation_data_ptr != NULL);

    if (continuation_data_ptr)
		dsReleaseContinueData(node->_nodeRef, continuation_data_ptr);

	return results;
}

CFArrayRef find_records_by_attribute_value(KADirectoryNode *node, const char *recordType, const char *attributeType, tDirPatternMatch matchType, const char *attributeValue, const char *attributesReturned)
{
	tDataNodePtr attributeTypeDataNode = dsDataNodeAllocateString(node->_directoryRef, kDSNAttrAuthenticationAuthority);		
	tDataNodePtr attributeValueDataNode = dsDataNodeAllocateString(node->_directoryRef, attributeValue);
	tDataListPtr recordTypeDataList = dsBuildListFromStrings(node->_directoryRef, recordType, NULL);
	tDataListPtr returnAttributesDataList = dsBuildListFromStrings(node->_directoryRef, attributesReturned, NULL);

	CFArrayRef results = find_records_by_attribute_value_internal(node, recordTypeDataList, attributeTypeDataNode, matchType, attributeValueDataNode, returnAttributesDataList);

	if (attributeTypeDataNode)
		dsDataNodeDeAllocate(node->_directoryRef, attributeTypeDataNode);
	if (attributeValueDataNode)
		dsDataNodeDeAllocate(node->_directoryRef, attributeValueDataNode);
	if (recordTypeDataList)
	    dsDataListDeallocate(node->_directoryRef, recordTypeDataList);
	if (returnAttributesDataList)
	    dsDataListDeallocate(node->_directoryRef, returnAttributesDataList);

	return results;
}

CFArrayRef find_user_records_by_name(KADirectoryNode *node, const char *username)
{  
    tDirStatus status = 0;
    long unsigned record_count = 0; // This is an input variable too
    tContextData continuation_data_ptr = NULL;
    CFMutableArrayRef results = NULL;

	tDataListPtr recordTypeDataList = dsBuildListFromStrings(node->_directoryRef, kDSStdRecordTypeUsers, NULL);
	tDataListPtr returnAttributesDataList = dsBuildListFromStrings(node->_directoryRef, kDSAttributesAll, NULL);
    tDataListPtr record_name_ptr = dsBuildListFromStrings(node->_directoryRef, username, NULL);
	
	do {
	
		for (;;)
		{
			status = dsGetRecordList(node->_nodeRef, node->_dataBufferPtr,
				record_name_ptr, eDSExact,
				recordTypeDataList,
				returnAttributesDataList,
				false,	/* attr info and data */
				&record_count,
				&continuation_data_ptr);
				
			// Of all errors we only try to recover from eDSBufferTooSmall here
			if ((status != eDSBufferTooSmall) || !(node->_dataBufferPtr = double_databuffer_or_bail(node->_directoryRef, node->_dataBufferPtr)))
			break;
		} 

        if (status)
			break;
			
        if (record_count > 0) 
        {
			long unsigned i;

			if (!results)
				results = CFArrayCreateMutable(kCFAllocatorDefault, record_count, &kCFTypeArrayCallBacks);

			for (i=1; i <= record_count; i++)
			{
				CFMutableDictionaryRef record = get_record_at_index(node, i);
				
				if (record)
				{
					CFArrayAppendValue(results, record);
					CFRelease(record);
				}
			}
        }
        
    } while (continuation_data_ptr != NULL);

	if (record_name_ptr)
		dsDataListDeallocate(node->_directoryRef, record_name_ptr);
	if (recordTypeDataList)
	    dsDataListDeallocate(node->_directoryRef, recordTypeDataList);
	if (returnAttributesDataList)
	    dsDataListDeallocate(node->_directoryRef, returnAttributesDataList);
    if (continuation_data_ptr)
		dsReleaseContinueData(node->_nodeRef, continuation_data_ptr);

	return results;
}

tDirStatus authenticate_user_to_node(KADirectoryNode *node, const char *username, const char *password)
{
    if (!username)
		return false;

    size_t ulNameLen = strlen(username);
    size_t ulPassLen = password ? strlen(password) : 0;

	dsBool authenticateOnly = true;
    tDataNodePtr _authType = dsDataNodeAllocateString (node->_directoryRef, kDSStdAuthNodeNativeNoClearText /*kDSStdAuthNodeNativeClearTextOK*/);
    tDataBufferPtr _authdataBufferPtr = dsDataBufferAllocate(node->_directoryRef, sizeof(ulNameLen) + ulNameLen + sizeof(ulPassLen) + ulPassLen);
    tDataBufferPtr _stepBufferPtr = dsDataBufferAllocate(node->_directoryRef, 2048);

    char *cpBuff = _authdataBufferPtr->fBufferData;
    memcpy(cpBuff, &ulNameLen, sizeof (ulNameLen));
    cpBuff += sizeof(ulNameLen);
    memcpy(cpBuff, username, ulNameLen);
    cpBuff += ulNameLen;
    memcpy(cpBuff, &ulPassLen, sizeof(ulPassLen));
    cpBuff += sizeof(ulPassLen);
    memcpy(cpBuff, password, ulPassLen);
    _authdataBufferPtr->fBufferLength = sizeof(ulNameLen) + ulNameLen + sizeof(ulPassLen) + ulPassLen;

    tDirStatus status = dsDoDirNodeAuth(node->_nodeRef, _authType, authenticateOnly, _authdataBufferPtr, _stepBufferPtr, 0);
    
    dsDataNodeDeAllocate(node->_directoryRef, _authType);
    dsDataBufferDeAllocate(node->_directoryRef, _authdataBufferPtr);
    dsDataBufferDeAllocate(node->_directoryRef, _stepBufferPtr);
    
	return status;
}

tDirStatus checkpw(const char *username, const char *password)
{
	tDirReference _directoryRef = 0;
	tDirStatus status = dsOpenDirService(&_directoryRef);
	
	if (status != eDSNoErr)
		return status;

	do {
		KADirectoryNode *node = NULL;
		
		status = KADirectoryClientGetNode(_directoryRef, eDSSearchNodeName, &node);
		
		if (status) break;
			
		CFArrayRef user_records = find_user_records_by_name(node, username);

		if (!user_records) { status = eDSRecordNotFound; break; }
		
		CFDictionaryRef user_record = (CFDictionaryRef)CFArrayGetValueAtIndex(user_records, 0);
		
		if (!user_record) { status = eDSRecordNotFound; break; }
			
		KADirectoryNode *authNode = KADirectoryNodeCreateFromUserRecord(_directoryRef, user_record);

		if (!authNode) { status = eDSNodeNotFound; break; }

		status = authenticate_user_to_node(authNode, username, password);
	
	} while (false);
	
	dsCloseDirService(_directoryRef);
	
	return status;
}

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
