                              xX Chili Xx

                          testrunner for C in C

- What is Chili ?

  A program that finds and executes unit tests. Used to write nice and
  slim unit tests in C. Written in C with minimal dependencies to be
  portable to run on target for embedded development.
  
- How does it work ?

  Unit tests are compiled into ELF shared library exposing where each
  unit test is an exported function and following these simple rules:
  
  * Unit test functions starts with 'test_'
  * Unit test functions takes no arguments
  * Unit test functions returns int
  * Unit test functions returns > 0 on success
  * Unit test functions returns 0 on failure
  * Unit test functions returns < 0 on error
  
  Each shared library is considered a test suite that can be setup
  by Chili. To use common setup in a test suite export one or more
  of these functions for the desired behaviour:
  
  * once_before for once per suite setup. Executed before any
    test function.
  * once_after for once per suite cleanup. Executed after all
    test functions, regardless of success or failure.
  * each_before for test setup. Executed before every test
    function.
  * each_after for test cleanup. Executed after every test
    function
    
