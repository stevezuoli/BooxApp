/*
 *  ADOBE CONFIDENTIAL
 *
 *  Copyright 2006-2009, Adobe Systems Incorporated
 *
 *  All Rights Reserved.
 *
 *  NOTICE:  All information contained herein is, and remains the property of
 *  Adobe Systems Incorporated and its suppliers, if any.  The intellectual and
 *  technical concepts contained herein are proprietary to Adobe Systems
 *  Incorporated and its suppliers and may be covered by U.S. and Foreign
 *  Patents, patents in process, and are protected by trade secret or copyright
 *  law.  Dissemination of this information or reproduction of this material is
 *  strictly forbidden unless prior written permission is obtained from Adobe
 *  Systems Incorporated.
 */
/**
 *  \file dp_types.h
 *
 *  Reader Mobile SDK public interface -- platform-dependent types
 */
#ifndef _DP_TYPES_H
#define _DP_TYPES_H

// these are platform-dependent types that the SDK uses and which
// can potentially be customized
//
// keep this list as small as possible
namespace dp
{

/**
 *  Type to represent UTF16 code point.
 */
typedef unsigned short utf16char;

#if defined(_MSC_VER)
/**
 *  Type to represent 64 bit unsigned integer.
 */
typedef unsigned __int64 uint64;
/**
 *  Type to represent time in milliseconds since start of 1970 GMT.
 */
typedef unsigned __int64 time_t;
#else
/**
 *  Type to represent 64 bit unsigned integer.
 */
typedef unsigned long long int uint64;
/**
 *  Type to represent time in milliseconds since start of 1970 GMT.
 */
typedef unsigned long long int time_t;
#endif

}

#endif // DP_TYPES_H
