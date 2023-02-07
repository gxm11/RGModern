g++ -c ./src/main.cpp -o main.o \
    -std=c++2a -s -fstack-protector-strong \
    -I/clang64/include -I/clang64/include/SDL2 \
    -I./src -I./vendors/include \
    -I./third_party/centurion/src \
	-I./third_party/concurrentqueue \
	-I./third_party/incbin \
	-I./third_party/readerwriterqueue \
	-I./third_party/xorstr/include \
    -I.