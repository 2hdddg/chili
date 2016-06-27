
                             xX Chili Xx
                          unit tests for C in C

    - What is Chili ?

      A program that finds and executes unit tests. Used to write nice
      and slim unit tests in C. Written in C with minimal dependencies
      for portability to be used on target for embedded development.

    - How does it work ?

      A suite of unit tests is compiled into an ELF shared library.
      Each unit test is an exported function following these rules:

      * Unit test functions starts with test_
      * Unit test functions takes no arguments
      * Unit test functions returns int
      * Unit test functions returns > 0 on success
      * Unit test functions returns 0 on failure
      * Unit test functions returns < 0 on error

      To use a common setup in a test suite, export these functions:

      * once_before for once per suite setup. Executed before any
        test function.
      * once_after for once per suite cleanup. Executed after all
        test functions, regardless of success or failure.
      * each_before for test setup. Executed before every test
        function.
      * each_after for test cleanup. Executed after every test
        function

      All of these functions takes no arguments and their return
      values follow the same rules as the test functions.

    - How do I run tests ?

      Compile and install Chili on the target system.
      Compile and copy shared libraries containing tests to the
      target system and:

      chili testsuite.so

