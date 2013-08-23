#ifndef ONYX_SEARCH_CONTEXT_H_
#define ONYX_SEARCH_CONTEXT_H_

#include <QString>

/// Define search context. Application may inherit this class
/// to implement its own search context.
class BaseSearchContext
{
public:
    BaseSearchContext();
    virtual ~BaseSearchContext();

public:
    void reset();

    const QString & pattern() const { return pattern_; }
    void setPattern(const QString &pattern);

    bool forward() const { return forward_; }
    void setForward(bool forward = true);

    bool case_sensitive() const { return case_sensitive_; }
    void setCaseSensitive(bool sensitive = true);

    bool match_whole_word() const { return match_whole_word_; }
    void setMatchWholeWord(bool match = true);

    bool isStopped() { return stop_; }
    void stop(bool s = true) { stop_ = s; }

    int userData() const { return user_data_; }
    int & userData() { return user_data_; }

private:
    QString pattern_;
    bool forward_;
    bool case_sensitive_;
    bool match_whole_word_;
    bool stop_;
    int user_data_;
};


struct SearchContext
{
    QString pattern;
    bool    forward;
    bool    match_case;
    bool    match_whole_word;

    SearchContext() : forward(false),
                      match_case(false),
                      match_whole_word(false) {}
};


/// Define search context. Application may inherit this class
/// to implement its own search context.
class OnyxSearchContext
{
public:
    OnyxSearchContext();
    ~OnyxSearchContext();

public:
    void reset();

    const QString & pattern() const { return pattern_; }
    void setPattern(const QString &pattern);

    bool forward() const { return forward_; }
    void setForward(bool forward = true);

    bool case_sensitive() const { return case_sensitive_; }
    void setCaseSensitive(bool sensitive = true);

    bool match_whole_word() const { return match_whole_word_; }
    void setMatchWholeWord(bool match = true);

    bool &isStopped() { return stop_; }
    void stop(bool s = true) { stop_ = s; }

    int userData() const { return user_data_; }
    int & userData() { return user_data_; }

    bool searchAll() const { return search_all_; }
    void setSearchAll(bool all = true) { search_all_ = all; }

private:
    QString pattern_;
    bool forward_;
    bool case_sensitive_;
    bool match_whole_word_;
    bool stop_;
    int user_data_;
    bool search_all_;
};


enum SearchNavigateType {
    SEARCH_NAV_PREVIOUS = 11,
    SEARCH_NAV_NEXT = 12,
};

#endif  // ONYX_SEARCH_CONTEXT_H_
