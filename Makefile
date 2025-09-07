install:
	cmake -S . -B build

build_game:
	cmake --build build --target QuestForTrueColor
	./build/QuestForTrueColor