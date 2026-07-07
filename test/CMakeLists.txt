add_executable(anisthesia-tests
	anisthesia_tests.cpp
)

target_link_libraries(anisthesia-tests PRIVATE
	anisthesia
)

target_compile_definitions(anisthesia-tests PRIVATE
	ANISTHESIA_PLAYERS_PATH="${CMAKE_CURRENT_LIST_DIR}/../data/players.anisthesia"
)

add_test(NAME anisthesia-tests COMMAND anisthesia-tests
	"/tmp/[SubsPlease] Fate Strange Fake - 01v2 (1080p) [7708674D].mkv"
)
