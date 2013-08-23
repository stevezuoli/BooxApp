
#include <QString>
#include "onyx/data/search_context.h"


BaseSearchContext::BaseSearchContext(void)
    : pattern_()
    , forward_(true)
    , case_sensitive_(false)
    , match_whole_word_(false)
    , stop_(false)
    , user_data_(0)
{
}

BaseSearchContext::~BaseSearchContext(void)
{
}

void BaseSearchContext::reset()
{
    pattern_.clear();
    forward_ = true;
    case_sensitive_ = false;
    match_whole_word_ = false;
    stop_ = false;
}

void BaseSearchContext::setPattern(const QString &pattern)
{
    pattern_ = pattern;
}

void BaseSearchContext::setForward(bool forward)
{
    forward_ = forward;
}

void BaseSearchContext::setCaseSensitive(bool sensitive)
{
    case_sensitive_ = sensitive;
}

void BaseSearchContext::setMatchWholeWord(bool whole)
{
    match_whole_word_ = whole;
}


OnyxSearchContext::OnyxSearchContext(void)
    : pattern_()
    , forward_(true)
    , case_sensitive_(false)
    , match_whole_word_(false)
    , stop_(false)
    , user_data_(0)
    , search_all_(false)
{
}

OnyxSearchContext::~OnyxSearchContext(void)
{
}

void OnyxSearchContext::reset()
{
    pattern_.clear();
    forward_ = true;
    case_sensitive_ = false;
    match_whole_word_ = false;
    stop_ = false;
    search_all_ = false;
}

void OnyxSearchContext::setPattern(const QString &pattern)
{
    pattern_ = pattern;
}

void OnyxSearchContext::setForward(bool forward)
{
    forward_ = forward;
}

void OnyxSearchContext::setCaseSensitive(bool sensitive)
{
    case_sensitive_ = sensitive;
}

void OnyxSearchContext::setMatchWholeWord(bool whole)
{
    match_whole_word_ = whole;
}


