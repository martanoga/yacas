
/** \file lispstring.cpp
 *  implementation of the more heavy functions that should not be inlined.
 */

#include "yacasprivate.h"
#include "lispstring.h"
#include "stubs.h"

void LispString::SetString(const LispChar * aString)
{
    LispInt length = PlatStrLen(aString);  // my own strlen
    GrowTo(length+1);
	value_type * aT = elements();
    for (LispInt i = 0; i <= length; i++)
    {
        aT[i] = aString[i];
    }
}

void LispString::SetStringCounted(const LispChar * aString,LispInt aLength)
{
    GrowTo(aLength+1);
	value_type * aT = elements();
    for (LispInt i = 0; i < aLength; i++)
    {
        aT[i] = aString[i];
    }
    aT[aLength] = '\0';
}

void LispString::SetStringStringified(const LispChar * aString)
{
    LispInt length = PlatStrLen(aString);  // my own strlen
    GrowTo(length+1 + 2);
	value_type * aT = elements();
    aT[0] = '\"';
    for (LispInt i = 0; i < length; i++)
    {
        aT[i+1] = aString[i];
    }
    aT[length+1] = '\"';
    aT[length+2] = '\0';
}

void LispString::SetStringUnStringified(const LispChar * aString)
{
    LispInt length = PlatStrLen(aString);  // my own strlen
    GrowTo(length-1);
	value_type * aT = elements();
    for (LispInt i = 1; i < length-1; i++)
    {
        aT[i-1] = aString[i];
    }
    aT[length-2] = '\0';
}

LispInt LispString::operator==(const LispString& aString)
{
    if (Size() != aString.Size())
        return 0;
    const LispChar * ptr1 = elements();
    const LispChar * ptr2 = aString.elements();
    return StrEqual(ptr1,ptr2);
}

LispString::~LispString()
{
}

void LispStringSmartPtr::operator=(LispString * aString)
{
	// Duh!  Increment first!
    if (aString)
        ++aString->iReferenceCount;
    if (iString && !--iString->iReferenceCount)
        delete iString;
    iString = aString;
}

LispStringSmartPtr::~LispStringSmartPtr()
{
    if (iString && !--iString->iReferenceCount)
        delete iString;
}