add_executable(anisthesia-tests
	anisthesia_tests.cpp
)

target_link_libraries(anisthesia-tests PRIVATE
	anisthesia
)

add_test(NAME anisthesia-tests COMMAND anisthesia-tests)
