#ifndef TEST_H
#define TEST_H

#include "ExceptionHandler.h"
#include "TestException.h"
#include "TestMacros.h"
#include "TestResult.h"

namespace testing {

class Test
{
public:
    Test (const char * testName,
          const char * filename, int linenumber);
    virtual ~Test();

    virtual void Run(TestResult& result );

protected:
    virtual void RunTest(TestResult& result) = 0;

    const char * m_name;
    const char * m_filename;
    const int m_linenumber;

private:
    Test(const Test &);
    Test& operator=(const Test &);
};



class TestRegistrar
{
public:
    TestRegistrar(Test * test);

};

}

#define TEST(test_name) \
    class test_name##Test : public testing::Test \
    { \
      public: \
        test_name##Test () : testing::Test (#test_name "Test", \
                                            __FILE__, \
                                            __LINE__){} \
        protected:                                      \
            virtual void RunTest (testing::TestResult& result_); \
    } test_name##Instance; \
    testing::TestRegistrar test_name##_registrar (&test_name##Instance); \
    void test_name##Test::RunTest (testing::TestResult& result_)

// Test with fixture
#define TEST_F(fixture, test_name) \
    struct fixture##test_name : public fixture {  \
        fixture##test_name(const char * name_) : m_name(name_) {}  \
        void test_name(testing::TestResult& result_); \
        const char * m_name; \
      private:               \
        fixture##test_name(const fixture##test_name &);                 \
        fixture##test_name& operator=(const fixture##test_name &);      \
    }; \
    class fixture##test_name##Test : public testing::Test  \
    { \
        public: \
            fixture##test_name##Test () \
                    : testing::Test (#test_name "Test", __FILE__, __LINE__) {} \
        protected: \
            virtual void RunTest (testing::TestResult& result_); \
    } fixture##test_name##Instance; \
    testing::TestRegistrar fixture##test_name##_registrar(\
            &fixture##test_name##Instance);  \
    void fixture##test_name##Test::RunTest (testing::TestResult& result_) { \
        fixture##test_name mt(m_name); \
        mt.test_name(result_); \
    } \
    void fixture ## test_name::test_name(testing::TestResult& result_)


// Test with fixture with construction parameters
#define TEST_FP(fixture, fixture_construction, test_name) \
    struct fixture##test_name : public fixture { \
        fixture##test_name(const char * name_) : fixture_construction, \
            m_name(name_) {} \
        void test_name(testing::TestResult& result_); \
        const char * m_name; \
    }; \
    class fixture##test_name##Test : public testing::Test               \
    { \
        public: \
            fixture##test_name##Test () \
                    : testing::Test (#test_name "Test", __FILE__, __LINE__) {} \
        protected:  \
            virtual void RunTest (testing::TestResult& result_); \
    } fixture##test_name##Instance; \
    testing::TestRegistrar fixture##test_name##_registrar(\
            &fixture##test_name##Instance); \
    void fixture##test_name##Test::RunTest (testing::TestResult& result_) { \
        fixture##test_name mt(m_name); \
        mt.test_name(result_); \
    } \
    void fixture ## test_name::test_name(testing::TestResult& result_)



#define EXPECT_TRUE(condition) \
    do { \
        try { \
            if (!(condition)) \
                result_.AddFailure( \
                        testing::Failure(#condition, m_name, __FILE__, \
                                         __LINE__)); \
        } catch( const testing::TestException& e ) { \
            ExceptionHandler::Handle(result_, e, m_name, __FILE__, __LINE__);\
        } catch(...) {                                                  \
            ExceptionHandler::Handle(result_, #condition, m_name, __FILE__,\
                                     __LINE__); \
        } \
    } while (0)

//#define EXPECT_FALSE(condition) EXPECT_TRUE((!(condition)))

#define EXPECT_FALSE(condition) \
    do { \
        try { \
            if (condition) \
                result_.AddFailure( \
                        testing::Failure(#condition " should not be true",\
                                         m_name, __FILE__,__LINE__)); \
        } catch( const testing::TestException& e ) { \
            ExceptionHandler::Handle(result_, e, m_name, __FILE__, __LINE__);\
        } catch(...) {                                                  \
            ExceptionHandler::Handle(result_, #condition, m_name, __FILE__,\
                                     __LINE__); \
        } \
    } while (0)


// TODO(hjiang): this should produce more informative error message.
#define EXPECT_SUBSTR(substr, orig) \
    EXPECT_TRUE(orig.find(substr) != std::string::npos)

#define EXPECT_EQ(expected,actual) \
    do \
    { \
        cppunitlite::CheckEqual( result_, expected, actual, __FILE__,\
                                 __LINE__, m_name ); \
    } while( 0 )

#define EXPECT_NE(expected,actual) \
    do \
    { \
        cppunitlite::CheckNotEqual( result_, expected, actual, __FILE__,\
                                 __LINE__, m_name ); \
    } while( 0 )

#define EXPECT_NULL(pointer) \
    do { \
        try { \
            if (static_cast<bool>(pointer))     \
                result_.AddFailure( \
                        testing::Failure(#pointer " should be NULL.",\
                                         m_name, __FILE__, __LINE__));\
        } catch( const testing::TestException& e ) { \
            ExceptionHandler::Handle(result_, e, m_name, __FILE__, __LINE__);\
        } catch(...) {                                                  \
            ExceptionHandler::Handle(result_, #pointer " should be NULL.",\
                                     m_name, __FILE__,__LINE__); \
        } \
    } while (0)

#define EXPECT_NOTNULL(pointer) \
    do { \
        try { \
            if (!static_cast<bool>(pointer))     \
                result_.AddFailure( \
                        testing::Failure(#pointer " should not be NULL.",\
                                         m_name, __FILE__, __LINE__));\
        } catch( const testing::TestException& e ) { \
            ExceptionHandler::Handle(result_, e, m_name, __FILE__, __LINE__);\
        } catch(...) {                                                  \
            ExceptionHandler::Handle(result_, #pointer " should not be NULL",\
                                     m_name, __FILE__,__LINE__); \
        } \
    } while (0)

#define EXPECT_CLOSE(expected,actual,kEpsilon) \
    do { \
        cppunitlite::CheckClose( result_, expected, actual, kEpsilon,\
                                 __FILE__, __LINE__, m_name ); \
    } while(0)

#define EXPECT_ARRAY_CLOSE(expected,actual,count,kEpsilon) \
    do { \
        cppunitlite::CheckArrayClose( result_, expected, actual, count,\
                                      kEpsilon, __FILE__, __LINE__, m_name );\
    } while(0)

#define EXPECT_ARRAY2D_CLOSE(expected,actual,rows,columns,kEpsilon) \
    do {                                                                \
            cppunitlite::CheckArrayClose2D( result_, expected, actual, rows,\
                                            columns, kEpsilon, __FILE__,\
                                            __LINE__, m_name ); \
    } while(0)

// CHECK_ASSERT is ignored, don't use it
#define CHECK_ASSERT(action) \
    do { \
        &result_; \
    } while(0);

#endif



