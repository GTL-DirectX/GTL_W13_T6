#pragma once
#include "String.h"
#include "Array.h"
namespace FStringUtils
{
    void ParseIntoArray(const FString& Source, TArray<FString>& OutArray, TCHAR Delim, bool bCullEmpty)
    {
        OutArray.Empty();
        int32 StartIndex = 0;
        int32 Length = Source.Len();

        for (int32 i = 0; i <= Length; ++i)
        {
            bool bIsDelimiter = (i == Length) || (Source[i] == Delim);
            if (bIsDelimiter)
            {
                int32 SubStrLen = i - StartIndex;
                if (SubStrLen > 0 || !bCullEmpty)
                {
                    OutArray.Emplace(Source.Mid(StartIndex, SubStrLen));
                }
                StartIndex = i + 1;
            }
        }
    }
}
